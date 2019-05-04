#include "cmdserver.h"
#include "model/mqaconfig.h"
#include "utils/mqaexception.h"
#include "utils/logger.h"

#ifdef ENABLE_HTTP_CMDSERVER
# include <QTcpSocket>
# include <QtHttpServer/QHttpServer>
#endif

CmdServer::CmdServer(QObject *parent) : QObject(parent)
{
#ifdef ENABLE_HTTP_CMDSERVER
    m_server = new QHttpServer();
#endif
}

CmdServer::~CmdServer()
{
    if (m_server) delete m_server;
}

QString CmdServer::BuildCommand(ptr_MqaConfig& i_config, int id, CmdServer::SrvCommand_t cmd)
{
    QString cmd_api;
    switch(cmd)
    {
        case srv_command_start: cmd_api = "start"; break;
        case srv_command_stop: cmd_api = "stop"; break;
        case srv_command_restart: cmd_api = "restart"; break;
    }

    return QString("http://%1:%2/%3/%4")
            .arg(i_config->m_server_address)
            .arg(i_config->m_server_port)
            .arg(id)
            .arg(cmd_api);
}

void CmdServer::start_server()
{
#ifdef ENABLE_HTTP_CMDSERVER
    // Create commands by route
    //m_server->route("/<arg>/start", [this](int id, QTcpSocket *socket)
    /*
    m_server->route("/<arg>/start", []()
    {
        //mqaLog(QString("Received 'start' request for instance ID %1 from address '%2'").arg(id).arg(socket->peerAddress().toString()));
        //this->emit start_instance(id);
    });
    */
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
#else
    mqaWarn("Commands through web services are disabled - no support compiled");
#endif
}
