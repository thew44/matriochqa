#ifndef EMUINSTANCE_H
#define EMUINSTANCE_H

#include <QObject>
#include <QProcess>
#include <QLinkedList>
#include "emuconfig.h"
#include "types.h"
#include "mqaconfig.h"

class EmuInstance : public QObject
{
    Q_OBJECT
public:
    explicit EmuInstance(ptr_MqaConfig i_mqaconf, const EmuConfig& i_config);
    virtual ~EmuInstance();

public slots:
    void start();
    e_emu_status status();
    bool has_configuration_pending();
    void stop();
    void prepare(const EmuConfig& i_config);
    void apply();

    int get_id() const;
    const QString& get_title() const { return m_current_config.m_title; }
    const EmuConfig& getCurrentConfig() const { return m_current_config;}
    const EmuConfig& getNextConfig() const { return m_next_config;}
    const QLinkedList<QString>& getConsole() const { return m_console; }
    const QString getEmuPID() const;

private slots:
    void readStdIo();
    void readStdErr();
    void processStateChanged(QProcess::ProcessState newState);

signals:
    void changed(int id);
    void consoleChanged(int id);

private:
    QString prepare_drive(const QString& i_source, bool i_copy);
    void add_console_bytes(const QByteArray& i_data);
    EmuInstance() {}

    bool m_status_new = true;

    QLinkedList<QString> m_console;
    QString m_console_currline;

    ptr_MqaConfig m_mqaconf;
    EmuConfig m_current_config;
    EmuConfig m_next_config;
    QProcess m_process;
};

#endif // EMUINSTANCE_H
