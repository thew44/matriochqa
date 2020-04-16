#include "cmdserver.h"
#include "model/mqaconfig.h"
#include "utils/mqaexception.h"
#include "utils/logger.h"
#include "server/mdgenerator.h"
#include <QUrl>

ProtCommandQueue_t CmdServer::m_CommandQueue = ProtCommandQueue_t();

CmdServer::CmdServer(QObject *parent) : QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, QTcpServer::newConnection, this, CmdServer::newConnection);

    m_dequeue_timer.setSingleShot(false);
    connect(&m_dequeue_timer, &QTimer::timeout, this, &CmdServer::dequeue_commands);
}

CmdServer::~CmdServer()
{
    if (m_tcpServer) delete m_tcpServer;
}

QString CmdServer::BuildCommand(ptr_MqaConfig& i_config, int id, SrvCommand_t cmd, QString from)
{
    QString cmd_api;
    switch(cmd)
    {
        case srv_command_start: cmd_api = "start"; break;
        case srv_command_stop: cmd_api = "stop"; break;
        case srv_command_restart: cmd_api = "restart"; break;
    }

    return QString("http://%1:%2/emu/%3/%4%5")
            .arg(i_config->m_server_address)
            .arg(i_config->m_server_port)
            .arg(id)
            .arg(cmd_api)
            .arg(from.isEmpty() ? "": QString("/%1").arg(QString::fromLocal8Bit(from.toLocal8Bit().toBase64())));
}

void CmdServer::start_server()
{
#ifdef ENABLE_HTTP_CMDSERVER
    bool retcode = true;
    if (m_config->m_server_address.toLower() == "any")
    {
        retcode = m_tcpServer->listen(QHostAddress::Any, (quint16)m_config->m_server_port);
    }
    else
    {
        QHostAddress address;
        if (!address.setAddress(m_config->m_server_address))
        {
            throw MqaException(QString("Cannot parse IP address '%1'").arg(m_config->m_server_address));
        }
        retcode = m_tcpServer->listen(address, (quint16)m_config->m_server_port);
    }

    if(retcode == false)
    {
        throw MqaException(QString("Cannot bind server on IP address '%1', port '%2'").arg(m_config->m_server_address).arg(m_config->m_server_port));
    }

    mqaLog(QString("Command server running on port %1").arg(m_config->m_server_port));
    m_dequeue_timer.start(100);
#else
    mqaWarn("Commands through web services are disabled - no support compiled");
#endif
}

void CmdServer::dequeue_commands()
{
    // Do something only if we can lock the queue... otherwise
    // just wait for next timer timeout
    CommandQueue_t* queue = m_CommandQueue.try_acquire();
    if (queue)
    {
        while(!queue->empty())
        {
            QPair<int, SrvCommand_t> order = queue->dequeue();
            switch (order.second)
            {
            case srv_command_start: emit start_instance(order.first); break;
            case srv_command_stop: emit stop_instance(order.first); break;
            case srv_command_restart: emit restart_instance(order.first); break;
            case srv_command_unknown: break;
            default: break;
            }
            // For unknown command, silent fail
        }
        m_CommandQueue.release();
    }

}

// Nice design from https://stackoverflow.com/questions/20546750/qtcpsocket-reading-and-writing
void CmdServer::newConnection()
{
    while (m_tcpServer->hasPendingConnections())
    {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        m_sockets.insert(socket);
    }
}

void CmdServer::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    m_sockets.remove(socket);
    socket->deleteLater();
}

void CmdServer::readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray buffer;
    while (socket->bytesAvailable() > 0)
    {
        buffer.append(socket->readAll());
    }
    processQuery(socket, buffer);
}

void CmdServer::processQuery(QTcpSocket *ioSocket, const QByteArray &i_data)
{
    try
    {
        // Decode query
        QStringList httpQuery = QString::fromLocal8Bit(i_data).split("\r\n");
        if (httpQuery.size() < 1)
        {
            throw MqaException(QString("[%1] Query too small or not valid HTTP: %2").arg(ioSocket->peerAddress().toString()).arg(QString::fromLocal8Bit(i_data)));
        }
        /*
        for (auto s: httpQuery)
        {
            mqaLog(s);
        }*/
        QStringList httpQueryHeaderMethod = httpQuery[0].split(" ");
        if (httpQueryHeaderMethod.size() != 3)
        {
            throw MqaException(QString("[%1] First header not valid: %2").arg(ioSocket->peerAddress().toString()).arg(httpQuery[0]));
        }
        QString location = httpQueryHeaderMethod[1].trimmed();
        // At that stage we got the complete location, e.g. /favicon.ico or /emu/1/stop
        // We must filter only the valid commands and execute them
        QStringList commandElements = location.split("/");
        if (commandElements[0].isEmpty()) commandElements.pop_front();
        // Valid commands starts with "emu"
        // For the rest, silently reply with ERR404
        if (commandElements.size() < 3 || commandElements[0] != "emu")
        {
            sendHttpReply(ioSocket, QStringList() << "HTTP/1.1 403 Forbidden");
            return;
        }

        // Continue decoding
        bool convOK = true;
        int id = commandElements[1].toInt(&convOK);
        SrvCommand_t srvcmd = srv_command_unknown;
        if(commandElements[2] == CMD_START)
        {
            srvcmd = srv_command_start;
        }
        else if(commandElements[2] == CMD_RESTART)
        {
            srvcmd = srv_command_restart;
        }
        else if(commandElements[2] == CMD_STOP)
        {
            srvcmd = srv_command_stop;
        }
        // Did we decode it right ?
        if (convOK == false || srvcmd == srv_command_unknown)
        {
            mqaWarn(QString("[%1] Invalid command: %2").arg(ioSocket->peerAddress().toString()).arg(location));
            sendHttpReply(ioSocket, QStringList() << "HTTP/1.1 418 I'm a teapot"); // :-)
            return;
        }
        // Try to extract referer
        QUrl bktrackURL;
        for (auto hd : httpQuery)
        {
            if (hd.startsWith("Referer:"))
            {
                QString referer = hd.remove("Referer: ");
                bktrackURL = QUrl(referer);
                break;
            }
        }
        // We may have received a backtrack URL encoded in base64
        /* bktrackURL.isValid() is too strict to be useful
        if (!bktrackURL.isEmpty() || !bktrackURL.isValid())
        {
            mqaWarn(QString("[%1] Invalid backtrack URL: %2").arg(ioSocket->peerAddress().toString()).arg(qUtf8Printable(bktrackURL.toString())));
            bktrackURL.clear();
        }*/

        // Put some significant log
        mqaLog(QString("[%1] Received valid command: ID=%2, CMD=%3, URL=%4").arg(ioSocket->peerAddress().toString()).arg(id).arg(commandElements[2]).arg(bktrackURL.isEmpty() ? "none" : qUtf8Printable(bktrackURL.toString())));

        // Execute command
        QPair<int, SrvCommand_t> sigcmd(id, srvcmd);
        m_CommandQueue.acquire().push_back(sigcmd);
        m_CommandQueue.release();

        // Either redirect to the back track URL or just say OK
        if (!bktrackURL.isEmpty())
        {
            QStringList httpReplyHeaders;
            httpReplyHeaders.append("HTTP/1.1 307 Temporary Redirect");
            httpReplyHeaders.append("Location: " + bktrackURL.toString());
            sendHttpReply(ioSocket, httpReplyHeaders);
            return;
        }

        // Otherwise just say OK
        QString httpReplyBody = "<html><body>OK</body></html>";
        QStringList httpReplyHeaders;
        httpReplyHeaders.append("HTTP/1.1 200 OK");
        httpReplyHeaders.append("Content-Type: text/html");
        httpReplyHeaders.append(QString("Content-Length: %1").arg(httpReplyBody.size()));
        sendHttpReply(ioSocket, httpReplyHeaders, httpReplyBody );

    }
    catch(const MqaException& ex)
    {
        mqaErr("Cannot process http command: " + ex.msg());
    }
}

void CmdServer::sendHttpReply(QTcpSocket *ioSocket, const QStringList& i_headers, QString i_body)
{
    // Format complete reply string and send it on the wire
    QString completeReply = i_headers.join("\r\n")+ "\r\n\r\n" + i_body;
    ioSocket->write(completeReply.toStdString().c_str());
    ioSocket->flush();
    ioSocket->waitForBytesWritten(3000);
    ioSocket->close();
}


