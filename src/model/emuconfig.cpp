#include "emuconfig.h"

#include <QStringList>
#include <functional>   // std::equal_to
#include "types.h"
#include "utils/mqaexception.h"
#include <QStringBuilder>

EmuConfig::EmuConfig()
{

}

bool EmuConfig::is_updated(const QString &i_serialized) const
{
    EmuConfig other_inst;
    other_inst.deserialize(i_serialized);

    return this->operator==(other_inst);
}

void EmuConfig::deserialize(const QString &i_serialized)
{
    QStringList args = i_serialized.split(";");
    if (args.size() < POS_MAXSIZE) throw MqaException(QString("Syntax error in string '%1': not enough element.").arg(i_serialized));
    bool ok = true;

    m_id = args[POS_ID].toInt(&ok);
    if (!ok || m_id <= 0) throw MqaException(QString("Syntax error in string '%1': wrong id '%2'. Must be a positive integer.").arg(i_serialized).arg(args[POS_ID]));

    m_order = args[POS_ORDER].toInt(&ok);
    if (!ok) m_order = DEF_UNDEFINED;

    m_title = args[POS_TITLE];

    m_cdrom = args[POS_CDROM_PATH];

    m_hda = args[POS_HDA_PATH];
    if (m_hda.isEmpty() && m_cdrom.isEmpty()) throw MqaException(QString("Syntax error in string '%1': Path to hda and cdrom cannot be both empty."));
    m_hda_copy = (args[POS_HDA_COPY] == STR_TRUE ? true : false);

    m_hdb = args[POS_HDB_PATH];
    m_hdb_copy = (args[POS_HDB_COPY] == STR_TRUE ? true : false);

    m_mem = (args[POS_MEM].isEmpty() ? "2G" : args[POS_MEM]);
}

QString EmuConfig::toLogTitle() const
{
    return QString("[id=%1 title='%2']").arg(m_id).arg(m_title);
}

void EmuConfig::clear()
{
    this->operator=(EmuConfig());
}

QString EmuConfig::serializeToMdHeaders()
{
    QString md_str =
            QString("|Status|Memory|CDROM|hda|hdb|") % "\n" %
                    "|------|------|-----|---|---|";
    return md_str;
}

QString EmuConfig::serializeToMdValues(const QString& i_firstColumn) const
{
    QString md_str =
            "|" % i_firstColumn % "|" %
            m_mem % "|";

    if (m_cdrom.isEmpty())
    {
        md_str += "*(empty)*|";
    }
    else
    {
        md_str += m_cdrom % "|";
    }

    if (m_hda.isEmpty())
    {
        md_str += "*(empty)*|";
    }
    else
    {
        md_str += (m_hda_copy ? "[Copy] " : "[No Copy] ") % m_hda % "|";
    }

    if (m_hdb.isEmpty())
    {
        md_str += "*(empty)*|";
    }
    else
    {
        md_str += (m_hdb_copy ? "[Copy] " : "[No Copy] ") % m_hdb % "|";
    }

    return md_str;
}
