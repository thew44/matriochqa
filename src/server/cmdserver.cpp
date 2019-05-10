#include "cmdserver.h"
#include "model/mqaconfig.h"
#include "utils/mqaexception.h"
#include "utils/logger.h"
#include "server/mdgenerator.h"

#ifdef ENABLE_HTTP_CMDSERVER
# include <QTcpSocket>
# include <QtCore>
# include <QtHttpServer/QHttpServer>
#endif

ProtCommandQueue_t CmdServer::m_CommandQueue = ProtCommandQueue_t();

CmdServer::CmdServer(QObject *parent) : QObject(parent)
{
#ifdef ENABLE_HTTP_CMDSERVER
    m_server = new QHttpServer();    
#endif    
    m_dequeue_timer.setSingleShot(false);
    connect(&m_dequeue_timer, &QTimer::timeout, this, &CmdServer::dequeue_commands);
}

CmdServer::~CmdServer()
{
    if (m_server) delete m_server;
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
    // Create commands by route
    m_server->route("/emu/<arg>/<arg>/<arg>", [] (qint32 id, QString cmd, QString from, const QHttpServerRequest &request) {
        // Not reliable but due to lack of documentation of QHttpServer, I do not know
        // any other way for the moment
        QString sender_ip = request.headers()[QStringLiteral("Remote_Addr")].toString();
        mqaLog(QString("Command server: Received '%1' request for instance ID %2 from address '%3'").arg(cmd).arg(id).arg(sender_ip));
        SrvCommand_t parse_cmd = srv_command_unknown;
        // Parse command name
        if(cmd == "start")
        {
            parse_cmd = srv_command_start;
        }
        else if(cmd == "stop")
        {
            parse_cmd = srv_command_stop;
        }
        else if(cmd == "restart")
        {
            parse_cmd = srv_command_restart;
        }
        else
        {
            return "Unknown command '" + cmd + "'";
        }

        // This is not elegant, but we cannot just capture 'this'
        // and do "emit start_signal(id)", it crashes the application
        m_CommandQueue.acquire().enqueue(QPair<int, SrvCommand_t>(id, parse_cmd));
        m_CommandQueue.release();

        // Location is sent by the front
        // Horrible trick but I cannot find another way with this buggy server
        QString from_location = QByteArray::fromBase64(from.toLocal8Bit()).data();
        // Use meta refresh as QHttpServerResponder::StatusCode::MovedPermanently does not seems to work at the moment
        mqaLog("FROM: " + from_location);
        //return QString("<html><meta http-equiv=\"refresh\" content=\"0; url=%1\"></html>").arg(from_location);
        return QString("<html><head>"
                       "<script>window.addEvent('domready', function() { history.go(-1); });</script>"
                       "</head></html>");
    });

    // Start listining
    int retcode = 0;
    if (m_config->m_server_address.toLower() == "any")
    {
        retcode = m_server->listen(QHostAddress::Any, (quint16)m_config->m_server_port);
    }
    else
    {
        QHostAddress address;
        if (!address.setAddress(m_config->m_server_address))
        {
            throw MqaException(QString("Cannot parse IP address '%1'").arg(m_config->m_server_address));
        }
        retcode = m_server->listen(address, (quint16)m_config->m_server_port);
    }

    if(retcode < 0)
    {
        throw MqaException(QString("Cannot bind server on IP address '%1', port '%2'").arg(m_config->m_server_address).arg(m_config->m_server_port));
    }

    mqaLog(QString("Command server running on port %1").arg(m_config->m_server_port));
#else
    mqaWarn("Commands through web services are disabled - no support compiled");
#endif
    m_dequeue_timer.start(100);
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
            }
            // For unknown command, silent fail
        }
        m_CommandQueue.release();
    }

}

