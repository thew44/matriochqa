#ifndef CMDSERVER_H
#define CMDSERVER_H

#include <QObject>
#include "model/types.h"

class QHttpServer;

class CmdServer : public QObject
{
    Q_OBJECT
public:
    enum SrvCommand_t {
        srv_command_stop,
        srv_command_start,
        srv_command_restart
    };


    explicit CmdServer(QObject *parent = nullptr);
    virtual ~CmdServer();

    void setConfig(ptr_MqaConfig i_config) { m_config = i_config; }

    static QString BuildCommand(ptr_MqaConfig &i_config, int id, SrvCommand_t cmd);

    void start_server();

signals:
    void start_instance(int id);
    void stop_instance(int id);
    void restart_instance(int id);

private:
    ptr_MqaConfig m_config;
    QHttpServer* m_server = nullptr;
};

#endif // CMDSERVER_H
