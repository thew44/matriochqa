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
    // Will destroy m_process and kill child emu if any running
}

void EmuInstance::start()
{
    //https://web.archive.org/web/20180104171638/http://nairobi-embedded.org/qemu_monitor_console.html

    emuLog(QString("Starting instance"));
    m_console.clear();
    m_console_currline.clear();
    // Get values needed to start - use friendliness
    QString hda = prepare_drive(m_current_config.m_hda, m_current_config.m_hda_copy);
    QString hdb = prepare_drive(m_current_config.m_hdb, m_current_config.m_hdb_copy);
    QString cdrom = prepare_drive(m_current_config.m_cdrom, false);
    QString mem = m_current_config.m_mem;

    // Prepare execution string
    QString exec_cmd = m_mqaconf->m_base_qemu_dir.canonicalPath() + QDir::separator() + m_current_config.m_machine.qemu_exec;
    QStringList exec_args;
    exec_args << "-L" << m_mqaconf->m_base_qemu_dir.canonicalPath();
    if (!m_current_config.m_machine.qemu_machine.isEmpty())
    {
        exec_args << "-M" << m_current_config.m_machine.qemu_machine;
    }
    if (!cdrom.isEmpty()) exec_args << "-cdrom" << cdrom;
    if (!hda.isEmpty()) exec_args << "-hda" << hda;
    if (!hdb.isEmpty()) exec_args << "-hdb" << hdb;

    // Video
    if (m_current_config.m_out_type == EmuConfig::vnc)
    {
        exec_args << "-vnc" << QString("%1:%2").arg(m_mqaconf->m_server_address).arg(get_id());
    }
    else
    {
        exec_args << "-nographic";
    }
    exec_args << "-m" << mem;

    // Add additional options
    exec_args.append(m_current_config.m_options);

    if(m_process.state() != QProcess::NotRunning)
    {
        throw MqaException(QString("Instance is already running"));
    }

    m_process.setWorkingDirectory(m_mqaconf->m_base_vm_dir.canonicalPath());
    m_process.start(exec_cmd, exec_args);
    emuLog(QString("Starting subprocess with arguments: %1").arg(exec_args.join(" ")));
    m_expected_status = st_running;
    m_process.waitForStarted();
    if (m_process.state() == QProcess::NotRunning)
    {
        throw MqaException(QString("Instance has failed to start"));
    }
    else
    {
        emuLog(QString("Instance is up and running"));
    }
    emit changed(get_id());
}

e_emu_status EmuInstance::status()
{
    if (m_expected_status == st_new) return ev_emu_new;

    QProcess::ProcessState process_state = m_process.state();

    if (m_expected_status == st_running && (process_state == QProcess::Running || process_state == QProcess::Starting)) return ev_emu_started;
    if (m_expected_status == st_stopped && (process_state == QProcess::Running || process_state == QProcess::Starting)) return ev_emu_stopping;
    if (m_expected_status == st_stopped && (process_state == QProcess::NotRunning)) return ev_emu_stopped;
    if (m_expected_status == st_running && (process_state == QProcess::NotRunning)) return ev_emu_error;

    // Unexpected combination
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
        emuLog(QString("Stopping instance"));
        m_process.terminate();
        m_expected_status = st_stopped;
        QThread::sleep(2);
        m_process.kill();
        emuLog(QString("Instance stopped"));
    }
}

void EmuInstance::prepare(const EmuConfig &i_config)
{
    // Is the proposed configuration different from the current one ?
    if (m_current_config != i_config)
    {
        m_next_config = i_config;
        emuLog(QString("Configuration has changed"));
    }
    else if (!m_next_config.is_empty() && i_config != m_next_config)
    {
        // The new config is equal to the one actually running, so
        // user just cancelled the pending conf by falling back to old conf.
        m_next_config.clear();
        emuLog(QString("Pending configuration changes cancelled"));
    }
}

void EmuInstance::apply()
{
    if (!m_next_config.is_empty())
    {
        emuLog(QString("Applying new configuration"));
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
            emuLog(QString("Removing old image copy '%1'").arg(dest_file));
            bool success_rem = QFile(dest_file).remove();
            if (!success_rem)
            {                
                throw MqaException(QString("Cannot remove file '%1'").arg(dest_file));
            }
        }

        // Now proceed with copy
        emuLog(QString("Copying '%1' to '%2'").arg(i_source).arg(dest_file));
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

void EmuInstance::emuLog(const QString &i_msg)
{
    // Add this line to the logbook
    m_logbook.push_back(QPair<QDateTime, QString>(QDateTime::currentDateTime(), i_msg));
    while (m_logbook.size() > m_mqaconf->m_logbook_depth)
    {
        m_logbook.pop_front();
    }
    emit changed(get_id());

    // Also log to general log file
    mqaLog(QString("%1 %2").arg(getCurrentConfig().toLogTitle()).arg(i_msg));
}
