#include "mqaconfig.h"
#include <QStringBuilder>

MqaConfig::MqaConfig(const QString &i_filename)
{
    //m_qemu_exec.insert("x86_64", "qemu-system-x86_64.exe");
    settings = QSharedPointer<QSettings>(new QSettings(i_filename, QSettings::Format::IniFormat));
}

void MqaConfig::write()
{
    settings->beginGroup("general");
    settings->setValue("base_vm_dir", m_base_vm_dir.path());
    settings->setValue("instance_config", m_mqa_vm_conf.absoluteFilePath());
    settings->setValue("new_instances_start_immediatly", m_new_emu_start_immediatly);
    settings->endGroup();

    settings->beginGroup("qemu");
    settings->setValue("base_qemu_dir", m_base_qemu_dir.path());
    for (auto k: m_qemu_machines.keys())
    {
        settings->setValue("exec/" + k, m_qemu_machines[k].qemu_exec);
        settings->setValue("machine/" + k, m_qemu_machines[k].qemu_machine);
    }
    settings->endGroup();

    settings->beginGroup("markdown");
    settings->setValue("hugo_content_dir", m_markdown_dir.path());
    settings->setValue("hugo_port", m_hugo_port);
    settings->setValue("mq_root", m_markdown_emuroot);
    settings->setValue("mq_instances_path", m_markdown_emupagedir);
    settings->setValue("content_console_depth", m_console_depth);
    settings->setValue("content_logbook_depth", m_logbook_depth);
    settings->endGroup();

    settings->beginGroup("command_server");
    settings->setValue("address", m_server_address);
    settings->setValue("port", m_server_port);
    settings->endGroup();
}

void MqaConfig::read()
{
    settings->beginGroup("general");
    m_base_vm_dir.setPath(settings->value("base_vm_dir").toString());
    m_mqa_vm_conf.setFile(settings->value("instance_config").toString());
    m_new_emu_start_immediatly = settings->value("new_instances_start_immediatly", false).toBool();
    settings->endGroup();

    settings->beginGroup("qemu");
    m_base_qemu_dir.setPath(settings->value("base_qemu_dir").toString());
    settings->beginGroup("exec");
    QStringList exec_keys = settings->childKeys();
    for (auto k: exec_keys)
    {
        machine_t m;
        m.qemu_exec = settings->value(k).toString();
        m_qemu_machines.insert(k, m);
    }
    settings->endGroup();
    settings->beginGroup("machine");
    QStringList machines_keys = settings->childKeys();
    for (auto mt: machines_keys)
    {
        if (m_qemu_machines.contains(mt))
        {
            m_qemu_machines[mt].qemu_machine = settings->value(mt).toString();
        }
    }
    settings->endGroup();
    settings->endGroup();

    settings->beginGroup("markdown");
    m_markdown_dir.setPath(settings->value("hugo_content_dir").toString());
    m_hugo_port = settings->value("hugo_port", 1313).toInt();
    m_markdown_emuroot = settings->value("mq_root", "/").toString();
    m_markdown_emupagedir = settings->value("mq_instances_path", "/docs/").toString();
    m_console_depth = settings->value("content_console_depth", 100).toUInt();
    m_logbook_depth = settings->value("content_logbook_depth", 30).toUInt();
    settings->endGroup();

    settings->beginGroup("command_server");
    m_server_address = settings->value("address", "127.0.0.1").toString();
    m_server_port = settings->value("port", 4450).toInt();
    settings->endGroup();
}

void MqaConfig::check()
{
    // TODO
}

QString MqaConfig::toMarkdown()
{
    QString md;
    md += QString("|Group|Variable|Value|\n") %
          "|-----|--------|-----|\n";

    QStringList keys = settings->allKeys();
    for (auto k : keys)
    {
        QStringList subk = k.split("/");
        QString grp = subk.first();
        subk.pop_front();
        md += QString("|%1|%2|%3|\n").arg(grp).arg(subk.join("/")).arg(settings->value(k).toString());
    }

    return md;
}
