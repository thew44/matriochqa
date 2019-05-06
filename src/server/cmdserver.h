#ifndef CMDSERVER_H
#define CMDSERVER_H

#include <QObject>
#include <QQueue>
#include <QPair>
#include <QMutex>
#include "model/types.h"
#include "utils/protected.h"

class QHttpServer;

enum SrvCommand_t {
    srv_command_stop,
    srv_command_start,
    srv_command_restart,
    srv_command_unknown
};


typedef Protected<QQueue<QPair<int, SrvCommand_t>>> CommandQueue_t;

class CmdServer : public QObject
{
    Q_OBJECT
public:

    explicit CmdServer(QObject *parent = nullptr);
    virtual ~CmdServer();

    void setConfig(ptr_MqaConfig i_config) { m_config = i_config; }

    static QString BuildCommand(ptr_MqaConfig &i_config, int id, SrvCommand_t cmd);

    void start_server();

private slots:
    void dequeue_commands();

signals:
    void start_instance(int id);
    void stop_instance(int id);
    void restart_instance(int id);

private:
    ptr_MqaConfig m_config;
    QHttpServer* m_server = nullptr;    
    static CommandQueue_t m_CommandQueue;
};

#endif // CMDSERVER_H
