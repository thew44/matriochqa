#include "matriochqa.h"

#include "model/emuconfig.h"
#include "utils/logger.h"
#include "utils/mqaexception.h"

#include <QThread>
#include <QFile>
#include <QTextStream>
#include "utils/hash.h"

Matriochqa::Matriochqa()
{
    m_mqaconf = ptr_MqaConfig(new MqaConfig());
    m_mdgen.setConfig(m_mqaconf);
    m_mdgen.setInstancesCatalog(&m_current_emulators);       
    m_cmdserver.setConfig(m_mqaconf);
}

void Matriochqa::loadconfig()
{
    QString aCfgFile = m_mqaconf->m_mqa_vm_conf.canonicalFilePath();
    mqaLog("Load configuration file " + aCfgFile);
    m_next_config.clear();

    mqaLog(QString("Filesize= %1").arg(m_mqaconf->m_mqa_vm_conf.size()));

    // Parse configuration
    QFile inputFile(aCfgFile);
    bool title_passed = false;
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          if (!title_passed)
          {
              in.readLine();
              title_passed = true;
          }
          else
          {
                EmuConfig conf;
                conf.deserialize(in.readLine());
                // Verify that there isn't already another line with the same ID
                if (m_next_config.contains(conf.get_id()))
                {
                    throw MqaException(QString("Image with ID %1 defined twice").arg(conf.get_id()));
                }

                m_next_config.insert(conf.get_id(), conf);
                mqaLog("Read virtual machine configuration: " + conf.toLogTitle());
          }
       }
       inputFile.close();
       // Do not forget to update hash - not completely bullet proof in case someone
       // lock the file in between... but should work in most++ cases
       m_vm_conf_hash = Hash::SHA1(aCfgFile);
    }
    else
    {
        throw MqaException(QString("Cannot open and read file " + aCfgFile));
    }
}

void Matriochqa::prepare_all()
{
    mqaLog("ENTERING prepare_all");
    mqaLog(QString("m_next_config size=%1    m_current_emulators size=%2").arg(m_next_config.size()).arg(m_current_emulators.size()));

    QList<int> emu_to_remove;
    // First, loop on all existing emulator instances to check whether some
    // were removed from the new configuration
    for (EmuInstance* eInst : m_current_emulators)
    {
        if (!m_next_config.contains(eInst->get_id()))
        {
            // This one was removed, stop it and kill it
            eInst->stop();
            emu_to_remove.push_back(eInst->get_id());
        }
    }
    // Apply removal from list
    for (int id : emu_to_remove)
    {
        delete m_current_emulators[id];
        m_current_emulators.remove(id);
    }

    mqaLog(QString("OUT OF prepare_all: %1").arg(m_current_emulators.size()));

    // Now loop on the ID in the new configuration
    // and update configuration in instances
    for(EmuConfig& new_vm_conf : m_next_config)
    {
        int new_id = new_vm_conf.get_id();        
        // If not yet created, this is a new emu instance
        if (!m_current_emulators.contains(new_id))
        {
            EmuInstance* cur_inst = new EmuInstance(m_mqaconf, new_vm_conf);
            connect(cur_inst, &EmuInstance::changed, &m_mdgen, &MdGenerator::emu_instance_changed);
            connect(cur_inst, &EmuInstance::consoleChanged, &m_mdgen, &MdGenerator::emu_instance_newlog);
            m_current_emulators.insert(new_id, cur_inst);
            if(m_mqaconf->m_new_emu_start_immediatly)
            {
                cur_inst->start();
            }
        }
        else
        {
            EmuInstance* cur_inst = m_current_emulators[new_id];
            cur_inst->prepare(new_vm_conf);
        }
    }
}

void Matriochqa::setup_config_surveillance()
{
    if (m_fs_watcher != nullptr)
    {
        delete m_fs_watcher;
    }

    // Watch any modification in the configuration file
    m_fs_watcher = new QFileSystemWatcher(this);
    // We have to watch for changes in the full dir
    // mostly because excel has a strange behavior when
    // saving CSV: it is first deleted !
    m_fs_watcher->addPath(m_mqaconf->m_mqa_vm_conf.canonicalFilePath());
    connect(m_fs_watcher, &QFileSystemWatcher::fileChanged, this, &Matriochqa::config_updated);
}

void Matriochqa::config_updated()
{
    mqaLog(QString("Configuration file '%1' seems to have changed, reprocessing...").arg(m_mqaconf->m_mqa_vm_conf.canonicalFilePath()));
    try
    {
        // Wait, because when file is edited with excel, it takes a while for it
        // really appear on the disk (see below)
        QThread::msleep(500);
        // Make sure file has changed since last call
        // This is necessary because FileSystemWatcher sometimes
        // triggers more than one signal
        if (Hash::SHA1(m_mqaconf->m_mqa_vm_conf.canonicalFilePath()) != m_vm_conf_hash)
        {
            // Load data from file
            loadconfig();

            mqaLog("AFTER loadconfig");
            mqaLog(QString("m_next_config size=%1    m_current_emulators size=%2").arg(m_next_config.size()).arg(m_current_emulators.size()));

            // Prepare all VM instances for new conf
            prepare_all();

            // Reconfigure the FileSystemWatcher - necessary when file is modified with excel
            // (CSV files are removed and created anew)
            // If already monitored, nothing will happen
            m_fs_watcher->addPath(m_mqaconf->m_mqa_vm_conf.canonicalFilePath());
        }
        else
        {
            mqaLog("Configuration file has not changed");
        }
    }
    catch(const MqaException& ex)
    {
        mqaLog("ERROR: Failed to reload configuration: " + ex.msg());
    }
}


