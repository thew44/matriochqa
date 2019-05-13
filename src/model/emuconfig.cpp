#include "emuconfig.h"

#include <QStringList>
#include <functional>   // std::equal_to
#include "types.h"
#include "model/csvparser.h"
#include "utils/mqaexception.h"
#include <QStringBuilder>

EmuConfig::EmuConfig()
{

}

bool EmuConfig::deserialize(csvreader_t &i_reader)
{
    std::string title, arch, cdrom, hda, hdacpy, hdb, hdbcpy, mem, nettype, netfwd, netbase, out, options;
    // Pull next row in configuration file
    bool not_eof = i_reader.read_row(m_id, title, arch, cdrom, hda, hdacpy, hdb, hdbcpy, mem, nettype, netfwd, netbase, out, options);
    // End of file reached ?
    if (not_eof == false) return !not_eof;

    // Perform some checks & copies to QString
    if (m_id <= 0)
        throw MqaException(QString("Syntax error at line %1: wrong id '%2'. Must be a positive integer.").arg(i_reader.get_file_line()).arg(m_id));

    m_title = QString::fromStdString(title);
    if (m_title.isEmpty())
        throw MqaException(QString("Syntax error at line %1: Title cannot be empty.").arg(i_reader.get_file_line()));

    m_cdrom = QString::fromStdString(cdrom);

    m_hda = QString::fromStdString(hda);
    if (m_hda.isEmpty() && m_cdrom.isEmpty())
        throw MqaException(QString("Syntax error at line %1: Path to hda and cdrom cannot be both empty.").arg(i_reader.get_file_line()));
    m_hda_copy = (hdacpy == STR_TRUE ? true : false);

    m_hdb = QString::fromStdString(hdb);
    m_hdb_copy = (hdbcpy == STR_TRUE ? true : false);

    m_mem = QString::fromStdString(mem);
    m_mem = (m_mem.isEmpty() ? "2G" : m_mem);
    if (m_mem.right(1) != "M" && m_mem.right(1) != "G" && m_mem.right(1) != "K")
        throw MqaException(QString("Syntax error at line %1: Memory quantity '%2' is invalid. Must be suffixed with K, M or G.").arg(i_reader.get_file_line()).arg(m_mem));

    return !not_eof;
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
