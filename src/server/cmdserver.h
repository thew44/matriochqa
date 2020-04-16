#ifndef CMDSERVER_H
#define CMDSERVER_H

#include <QObject>
#include <QQueue>
#include <QPair>
#include <QSet>
#include <QMutex>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include "model/types.h"
#include "utils/protected.h"

enum SrvCommand_t {
    srv_command_stop,
    srv_command_start,
    srv_command_restart,
    srv_command_unknown
};

#define CMD_START "start"
#define CMD_STOP "stop"
#define CMD_RESTART "restart"

typedef QQueue<QPair<int, SrvCommand_t>> CommandQueue_t;
typedef Protected<CommandQueue_t> ProtCommandQueue_t;

class CmdServer : public QObject
{
    Q_OBJECT
public:

    explicit CmdServer(QObject *parent = nullptr);
    virtual ~CmdServer();

    void setConfig(ptr_MqaConfig i_config) { m_config = i_config; }

    static QString BuildCommand(ptr_MqaConfig &i_config, int id, SrvCommand_t cmd, QString from);

    void start_server();

private slots:
    void dequeue_commands();
    void newConnection();
    void disconnected();
    void readyRead();

private:
    void processQuery(QTcpSocket* ioSocket, const QByteArray& i_data);
    void sendHttpReply(QTcpSocket* ioSocket, const QStringList& i_headers, QString i_body = "");

signals:
    void start_instance(int id);
    void stop_instance(int id);
    void restart_instance(int id);

private:
    QTcpServer* m_tcpServer = nullptr;
    ptr_MqaConfig m_config; 
    QTimer m_dequeue_timer;
    static ProtCommandQueue_t m_CommandQueue;
    QSet<QTcpSocket*> m_sockets;
};

#endif // CMDSERVER_H
