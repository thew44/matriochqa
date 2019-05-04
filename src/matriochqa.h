#ifndef MATRIOCHQA_H
#define MATRIOCHQA_H

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QFileSystemWatcher>
#include "model/emuconfig.h"
#include "model/emuinstance.h"
#include "model/mqaconfig.h"
#include "model/types.h"
#include "server/mdgenerator.h"
#include "server/cmdserver.h"

class Matriochqa : public QObject
{
    Q_OBJECT
public:
    Matriochqa();

    void loadconfig();

    void prepare_all();

    void setup_config_surveillance();

public slots:
    void config_updated();

private slots:
    //void suspect_config_updated();

private:
    ptr_MqaConfig m_mqaconf;
    MdGenerator m_mdgen;
    CmdServer m_cmdserver;

    QMap<int, EmuConfig> m_next_config;
    QMap<int, EmuInstance*> m_current_emulators;

    QFileSystemWatcher* m_fs_watcher = nullptr;
    QByteArray m_vm_conf_hash;
};

#endif // MATRIOCHQA_H
