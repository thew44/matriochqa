#include <QThread>
#include "utils/mqaexception.h"
#include "emuinstance.h"
#include "utils/logger.h"
#include <QFileInfo>
#include <QTextCodec>

EmuInstance::EmuInstance(ptr_MqaConfig i_mqaconf, const EmuConfig &i_config) : m_mqaconf(i_mqaconf), m_current_config(i_config)
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &EmuInstance::readStdIo);
    connect(&m_process, &QProcess::readyReadStandardError, this, &EmuInstance::readStdErr);
    connect(&m_process, &QProcess::stateChanged, this, &EmuInstance::processStateChanged);
}

EmuInstance::~EmuInstance()
{
    mqaLog("DELETE");
    //stop();
}

void EmuInstance::start()
{
    //https://web.archive.org/web/20180104171638/http://nairobi-embedded.org/qemu_monitor_console.html

    mqaLog(QString("%1 Starting instance").arg(m_current_config.toLogTitle()));
    m_console.clear();
    m_console_currline.clear();
    // Get values needed to start - use friendliness
    QString hda = prepare_drive(m_current_config.m_hda, m_current_config.m_hda_copy);
    QString hdb = prepare_drive(m_current_config.m_hdb, m_current_config.m_hdb_copy);
    QString cdrom = prepare_drive(m_current_config.m_cdrom, false);
    QString mem = m_current_config.m_mem;

    // Prepare execution string
    QString exec_cmd = m_mqaconf->m_base_qemu_dir.canonicalPath() + QDir::separator() + m_mqaconf->m_qemu_exec["x86_64"];
    QStringList exec_args;
    exec_args << "-L" << m_mqaconf->m_base_qemu_dir.canonicalPath();
    if (!cdrom.isEmpty()) exec_args << "-cdrom" << cdrom;
    if (!hda.isEmpty()) exec_args << "-hda" << hda;
    if (!hdb.isEmpty()) exec_args << "-hdb" << hdb;
    exec_args << "-nographic";
    exec_args << "-m" << mem;

    if(m_process.state() != QProcess::NotRunning)
    {
        throw MqaException(QString("Instance '%1' is already running").arg(m_current_config.toLogTitle()));
    }

    m_status_new = false;
    m_process.setWorkingDirectory(m_mqaconf->m_base_vm_dir.canonicalPath());
    m_process.start(exec_cmd, exec_args);
    m_process.waitForStarted();
    if (m_process.state() == QProcess::NotRunning)
    {
        throw MqaException(QString("Instance '%1' has failed to start").arg(m_current_config.toLogTitle()));
    }
    else
    {
        mqaLog(QString("%1 is running").arg(m_current_config.toLogTitle()));
    }        
}

e_emu_status EmuInstance::status()
{
    if (m_status_new) return ev_emu_new;

    if (m_process.state() == QProcess::Running || m_process.state() == QProcess::Starting) return ev_emu_started;

    if (m_process.exitStatus() == QProcess::NormalExit) return ev_emu_stopped;

    return ev_emu_error;
}

bool EmuInstance::has_configuration_pending()
{
    return (!m_next_config.is_empty());
}

void EmuInstance::stop()
{
    if (m_process.state() != QProcess::NotRunning)
    {
        mqaLog(QString("%1 Stopping instance").arg(m_current_config.toLogTitle()));
        m_process.terminate();
        QThread::sleep(2);
        m_process.kill();
        mqaLog(QString("%1 Instance stopped").arg(m_current_config.toLogTitle()));
    }
}

void EmuInstance::prepare(const EmuConfig &i_config)
{
    // Is the proposed configuration different from the current one ?
    if (m_current_config != i_config)
    {
        m_next_config = i_config;
        mqaLog(QString("%1 Configuration has changed").arg(m_current_config.toLogTitle()));
        emit changed(get_id());
    }
    else if (!m_next_config.is_empty() && i_config != m_next_config)
    {
        // The new config is equal to the one actually running, so
        // user just cancelled the pending conf by falling back to old conf.
        m_next_config.clear();
        mqaLog(QString("%1 Pending changes cancelled").arg(m_current_config.toLogTitle()));
        emit changed(get_id());
    }
}

void EmuInstance::apply()
{
    if (!m_next_config.is_empty())
    {
        mqaLog(QString("%1 Applying new configuration").arg(m_current_config.toLogTitle()));
        stop();
        m_current_config = m_next_config;
        m_next_config.clear();
        emit changed(get_id());
        start();
    }
}

int EmuInstance::get_id() const
{
    return m_current_config.get_id();
}

const QString EmuInstance::getEmuPID() const
{
    return QString("%1").arg(m_process.processId());
}

void EmuInstance::readStdIo()
{
    add_console_bytes(m_process.readAllStandardOutput());
}

void EmuInstance::readStdErr()
{
    add_console_bytes(m_process.readAllStandardError());
}

void EmuInstance::processStateChanged(QProcess::ProcessState newState)
{
    emit changed(get_id());
}

QString EmuInstance::prepare_drive(const QString &i_source, bool i_copy)
{
    if (i_source.isEmpty()) return "";

    // Check whether source exist
    QFileInfo src_nfo(i_source);
    if (!src_nfo.isReadable())
    {
        throw MqaException(QString("Image file '%1' cannot be read").arg(i_source));
    }

    QString dest_file = src_nfo.filePath();

    // Do we need to copy ?
    if (i_copy)
    {
        QFileInfo dest_nfo;
        dest_nfo.setFile(m_mqaconf->m_base_vm_dir, src_nfo.completeBaseName() + QString("_%1.").arg(get_id()) + src_nfo.suffix());
        dest_file = dest_nfo.absoluteFilePath();
        if (dest_nfo.exists())
        {
            // A file is already there, removing it
            mqaLog(QString("%1 Removing old image copy '%2'").arg(m_current_config.toLogTitle()).arg(dest_file));
            bool success_rem = QFile(dest_file).remove();
            if (!success_rem)
            {                
                throw MqaException(QString("Cannot remove file '%1'").arg(dest_file));
            }
        }

        // Now proceed with copy
        mqaLog(QString("%1 Copying '%2' to '%3'").arg(m_current_config.toLogTitle()).arg(i_source).arg(dest_file));
        bool success_cpy = QFile(i_source).copy(dest_file);
        if (!success_cpy)
        {
            throw MqaException(QString("Cannot copy file '%1'").arg(dest_file));
        }
    }

    return dest_file;
}

void EmuInstance::add_console_bytes(const QByteArray &i_data)
{
    for (int i = 0; i < i_data.size(); ++i)
    {
        char c = i_data[i];
        // Keep only printable characters
        // Also filter \r
        if ( c >= 32 )
        {
            m_console_currline += c;
        }
        // When new line (filter \r) append to the console
        if (c == '\n')
        {
            m_console.push_back(m_console_currline);
            m_console_currline.clear();
        }
    }
    // Flush old console logs if required
    while(m_console.size() > m_mqaconf->m_console_depth)
    {
        m_console.pop_front();
    }

    emit consoleChanged(get_id());
}