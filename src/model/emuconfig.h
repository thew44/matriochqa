#ifndef EMUCONFIG_H
#define EMUCONFIG_H

#include <QString>

class EmuInstance;

class EmuConfig
{
    // I do not like friends so much but EmuInstance
    // has to access all fields here to start emulation
    friend class EmuInstance;

public:
    EmuConfig();

    inline bool operator==(const EmuConfig& lhs) const
    {
        return  lhs.m_title == m_title &&
                lhs.m_order == m_order &&
                lhs.m_id == m_id &&
                lhs.m_cdrom == m_cdrom &&
                lhs.m_hda == m_hda &&
                lhs.m_hda_copy == m_hda_copy &&
                lhs.m_hdb == m_hdb &&
                lhs.m_hdb_copy == m_hdb_copy &&
                lhs.m_mem == m_mem;
    }

    inline bool operator!=(const EmuConfig& lhs) const
    {
        return !(this->operator==(lhs));
    }

    bool is_updated(const QString& i_serialized) const;

    void deserialize(const QString& i_serialized);

    QString toLogTitle() const;

    int get_id() const { return m_id; }

    bool is_empty() const { return m_id == 0; }

    void clear();


    static QString serializeToMdHeaders();
    QString serializeToMdValues(const QString &i_firstColumn) const;

private:
    QString m_title;
    int m_order = 0;
    int m_id = 0;
    QString m_cdrom;
    QString m_hda;
    bool m_hda_copy = false;
    QString m_hdb;
    bool m_hdb_copy = false;
    QString m_mem;
};

#endif // EMUINSTANCE_H
