#ifndef MQACONFIG_H
#define MQACONFIG_H
#include <QString>
#include <QDir>
#include <QHash>
#include <QSettings>
#include "types.h"

struct MqaConfig
{
    bool m_new_emu_start_immediatly = true;
    unsigned int m_console_depth = 100;
    unsigned int m_logbook_depth = 30;
    QDir m_base_vm_dir = QDir("C:/temp");
    QDir m_base_qemu_dir = QDir("C:/Program Files/qemu");    
    QHash<QString, machine_t> m_qemu_machines;
    QFileInfo m_mqa_vm_conf = QFileInfo("C:/mqa/vm.csv");

    QDir m_markdown_dir = QDir("C:/mqa/mqa/content");
    QString m_markdown_emupagedir = "/docs/";
    QString m_markdown_emuroot = "/";

    int m_server_port = 4455;
    int m_hugo_port = 1313;
    QString m_server_address = "127.0.0.1";

    MqaConfig(const QString &i_filename);

    void write();
    void read();
    void check();
    QString toMarkdown();

private:
    QSharedPointer<QSettings> settings;
};

#endif // MQACONFIG_H
