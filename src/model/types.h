#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

#define POS_ID 0
#define POS_ORDER POS_ID+1
#define POS_TITLE POS_ORDER+1
#define POS_CDROM_PATH POS_TITLE+1
#define POS_HDA_PATH POS_CDROM_PATH+1
#define POS_HDA_COPY POS_HDA_PATH+1
#define POS_HDB_PATH POS_HDA_COPY+1
#define POS_HDB_COPY POS_HDB_PATH+1
#define POS_MEM POS_HDB_COPY+1
#define POS_MAXSIZE POS_MEM

#define DEF_UNDEFINED -1
#define STR_TRUE "true"
#define STR_FALSE "false"

#define FILE_MD_INDEX "_index.md"
#define MD_SIGNATURE(iD, iT) "------" << endl << "*matriochqa, build " << iD << " - " << iT << endl << "Mathieu Allory, (c)2019 - All rights reserved*" << endl

enum e_emu_status
{
    ev_emu_new = 0,
    ev_emu_stopped,
    ev_emu_started,
    ev_emu_error
};

class MqaConfig;
typedef QSharedPointer<MqaConfig> ptr_MqaConfig;

#endif
