#ifndef EMUCONFIG_H
#define EMUCONFIG_H

#include <QString>
#include <QList>
#include "model/csvparser.h"
#include "model/types.h"

class EmuInstance;

class EmuConfig
{
    // I do not like friends so much but EmuInstance
    // has to access all fields here to start emulation
    friend class EmuInstance;
    friend class MdGenerator;

public:
    enum NetType
    {
        forward,
        nat
    };

    enum OutType
    {
        console,
        vnc
    };

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
                lhs.m_mem == m_mem &&
                lhs.m_net_type == m_net_type &&
                lhs.m_port_base == m_port_base &&
                lhs.m_net_eth == m_net_eth &&
                lhs.m_port_fwd == m_port_fwd &&
                lhs.m_out_type == m_out_type &&
                lhs.m_options == m_options;
    }

    inline bool operator!=(const EmuConfig& lhs) const
    {
        return !(this->operator==(lhs));
    }

    /*
     * @return true when EOF is reached. Object is then not modified.
     */
    bool deserialize(csvreader_t& i_reader, ptr_MqaConfig i_mqaconf);

    QString toLogTitle() const;

    int get_id() const { return m_id; }

    bool is_empty() const { return m_id == 0; }

    void clear();


    static QString serializeToMdHeaders();
    QString serializeToMdValues(const QString &i_firstColumn) const;

private:
    QString m_title;
    machine_t m_machine;
    int m_order = 0;
    int m_id = 0;
    QString m_cdrom;
    QString m_hda;
    bool m_hda_copy = false;
    QString m_hdb;
    bool m_hdb_copy = false;
    QString m_mem;
    NetType m_net_type = forward;
    int m_port_base = 0;
    QString m_net_eth;
    QList<int> m_port_fwd;
    OutType m_out_type = console;
    QStringList m_options;
};

#endif // EMUINSTANCE_H
