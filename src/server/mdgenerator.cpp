#include "mdgenerator.h"
#include "model/emuinstance.h"
#include "utils/logger.h"
#include "server/cmdserver.h"
#include <QMap>
#include <QTextStream>
#include <QDateTime>

MdGenerator::MdGenerator(QObject *parent) : QObject(parent)
{
}

void MdGenerator::rebuild_index_page()
{
    if (m_config.isNull()) return;

    // Rebuild _index.md
    QFile outputFile(m_config->m_markdown_dir.path() + "/" + FILE_MD_INDEX);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QTextStream out(&outputFile);

        out << "---" << endl;
        out << "title: Introduction" << endl;
        out << "type: docs" << endl;
        out << "---" << endl;

        out << "# Matriochqa Report: Overview" << endl;
        out << endl;
        out << QString("Currently %1 VM instances configured.").arg(m_current_emulators->size()) << endl;
        out << endl;

        out << "|VM ID|Title|Status|Configuration|Actions|" << endl;
        out << "|-----|-----|------|-------------|-------|" << endl;

        for (auto inst : m_current_emulators->values())
        {
            out << "|" << inst->get_id();
            out << "|[" << inst->get_title() << "]({{< relref \"" << m_config->m_markdown_emupagedir << get_emu_page(inst->get_id()) << "\" >}})";
            out << "|";
            out << str_status(inst->status());
            out << "|" << (inst->has_configuration_pending()? "UPDATE_PENDING" : "UP_TO_DATE");
            out << "|";
            out << "[Stop](" << CmdServer::BuildCommand(m_config, inst->get_id(), CmdServer::srv_command_stop) << ") - ";
            out << "[Start](" << CmdServer::BuildCommand(m_config, inst->get_id(), CmdServer::srv_command_start) << ") - ";
            out << "[Restart](" << CmdServer::BuildCommand(m_config, inst->get_id(), CmdServer::srv_command_restart) << ")";
            out << "|" << endl;

        }

        out << "Generated at: " << QDateTime::currentDateTime().toString() << endl << endl;

        out << MD_SIGNATURE(__DATE__, __TIME__) << endl;

        outputFile.close();
    }
    else
    {
        mqaWarn(QString("Cannot open file '%1' for writting, markdown report not updated").arg(outputFile.fileName()));
    }
}

void MdGenerator::rebuild_instance_page(EmuInstance* i_instance)
{
    if (m_config.isNull()) return;

    // Build destination dir
    QStringList dirs;
    dirs.append(m_config->m_markdown_dir.absolutePath().split("/"));
    dirs.append(m_config->m_markdown_emupagedir.split("/"));
    dirs.removeAll("");
    QString emuPageFile;
#ifndef WIN32
    // Windows already haz C:, D:, etc...
    emuPageFile += "/";
#endif
    emuPageFile += dirs.join("/") + "/" + get_emu_page(i_instance->get_id());

    // Rebuild emu instance page
    QFile outputFile(emuPageFile);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QTextStream out(&outputFile);

        out << "---" << endl;
        out << "title: " << i_instance->get_title() << endl;
        out << "type: docs" << endl;
        out << "---" << endl;

        out << "# Matriochqa Report: " << i_instance->get_title() << endl;
        out << "Instance with ID " << i_instance->get_id() << endl;
        out << endl;
        out << "**Status: " << str_status(i_instance->status()) << "**" << endl << endl;
        out << "QEMU backend PID: " << i_instance->getEmuPID() << endl;
        if (!i_instance->getNextConfig().is_empty())
        {
            out << endl << "<span style='color:red'>*Updated configuration not loaded - restart pending...*</span>" << endl;
        }
        out << endl;

        out << "**Commands:**" << endl << endl;
        out << "[Stop](" << CmdServer::BuildCommand(m_config, i_instance->get_id(), CmdServer::srv_command_stop) << ")"  << endl;
        out << "[Start](" << CmdServer::BuildCommand(m_config, i_instance->get_id(), CmdServer::srv_command_start) << ")"  << endl;
        out << "[Restart](" << CmdServer::BuildCommand(m_config, i_instance->get_id(), CmdServer::srv_command_restart) << endl;
        out << endl;

        out << "**Configuration:**"<< endl << endl;
        out << i_instance->getCurrentConfig().serializeToMdHeaders() << endl;
        out << i_instance->getCurrentConfig().serializeToMdValues("Current") << endl;
        if (!i_instance->getNextConfig().is_empty())
        {
            out << i_instance->getNextConfig().serializeToMdValues("Pending") << endl;
        }

        out << endl;
        out << "**Console:**"<< endl << endl;
        out << QString("*(Last %1 lines displayed)*").arg(m_config->m_console_depth) << endl;
        out << "```console" << endl;
        for (auto line : i_instance->getConsole())
        {
            out << line << endl;
        }
        out << "```" << endl;


        out << "Generated at: " << QDateTime::currentDateTime().toString() << endl << endl;

        out << MD_SIGNATURE(__DATE__, __TIME__) << endl;

        outputFile.close();
    }
    else
    {
        mqaWarn(QString("Cannot open file '%1' for writting, markdown report not updated").arg(outputFile.fileName()));
    }
}

QString MdGenerator::str_status(e_emu_status i_st)
{
    switch(i_st)
    {
    case ev_emu_new: return QString("NEW"); break;
    case ev_emu_stopped: return QString("STOPPED"); break;
    case ev_emu_started: return QString("STARTED"); break;
    case ev_emu_error: return QString("ERROR"); break;
    }
    return QString("UNKNOWN");
}

QString MdGenerator::get_emu_page(int id)
{
    return QString("emu_%1.md").arg(id);
}

void MdGenerator::emu_instance_changed(int id)
{
    rebuild_instance_page(m_current_emulators->find(id).value());
    rebuild_index_page();
}

void MdGenerator::emu_instance_newlog(int id)
{
    rebuild_instance_page(m_current_emulators->find(id).value());
}
