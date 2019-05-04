#ifndef MQACONFIG_H
#define MQACONFIG_H
#include <QString>
#include <QDir>
#include <QHash>
#include "types.h"

struct MqaConfig
{
    bool m_new_emu_start_immediatly = true;
    unsigned int m_console_depth = 100;
    QDir m_base_vm_dir = QDir("C:/temp");
    QDir m_base_qemu_dir = QDir("C:/Program Files/qemu");    
    QHash<QString, QString> m_qemu_exec;
    QFileInfo m_mqa_vm_conf = QFileInfo("C:/mqa/vm.csv");

    QDir m_markdown_dir = QDir("C:/mqa/mqa/content");
    QString m_markdown_emupagedir = "/docs/";

    int m_server_port = 4455;
    QString m_server_address = "127.0.0.1";

    MqaConfig()
    {
        m_qemu_exec.insert("x86_64", "qemu-system-x86_64.exe");
    }
};

#endif // MQACONFIG_H
