#ifndef MDGENERATOR_H
#define MDGENERATOR_H

#include <QObject>
#include "model/types.h"
#include "model/mqaconfig.h"

class EmuInstance;

class MdGenerator : public QObject
{
    Q_OBJECT
public:
    explicit MdGenerator(QObject *parent = nullptr);

    void setConfig(ptr_MqaConfig i_config) { m_config = i_config; }
    void setInstancesCatalog(QMap<int, EmuInstance*>* i_emu_instances) { m_current_emulators = i_emu_instances; }

signals:

private:
    void rebuild_index_page();
    void rebuild_instance_page(EmuInstance* i_instance);
    QString str_status(e_emu_status i_st);
    QString get_emu_page(int id);

public slots:
    void emu_instance_changed(int id);
    void emu_instance_newlog(int id);

private:
    ptr_MqaConfig m_config;
    QMap<int, EmuInstance*>* m_current_emulators = nullptr;
};

#endif // MDGENERATOR_H
