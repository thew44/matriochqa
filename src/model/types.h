#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

#define CFG_ID "id"
#define CFG_TITLE "title"
#define CFG_ARCH "architecture"
#define CFG_CDROM "cdrom"
#define CFG_HDA "hda"
#define CFG_HDACPY "hda copy"
#define CFG_HDB "hdb"
#define CFG_HDBCPY "hdb copy"
#define CFG_MEM "memory"
#define CFG_NETTYPE "network type"
#define CFG_NETFWD "forward"
#define CFG_NETBASE "base port"
#define CFG_OUT "output"
#define CFG_OPTIONS "options"

#define DEF_UNDEFINED -1
#define STR_TRUE "true"
#define STR_FALSE "false"

#define FILE_MD_INDEX "_index.md"
#define MD_SIGNATURE(iD, iT) "------" << endl << "*matriochqa, build " << iD << " - " << iT << endl << "Mathieu Allory, (c)2020 - All rights reserved*" << endl

enum e_emu_status
{
    ev_emu_new = 0,
    ev_emu_stopped,
    ev_emu_stopping,
    ev_emu_started,
    ev_emu_error
};

struct machine_t
{
    QString qemu_machine;
    QString qemu_exec;
};

struct MqaConfig;
typedef QSharedPointer<MqaConfig> ptr_MqaConfig;

#endif
