#include "hash.h"
#include <QFile>
#include <QCryptographicHash>
#include "utils/mqaexception.h"

QByteArray Hash::SHA1(const QString &iFileName)
{
    // Many thanks to https://stackoverflow.com/questions/16383392/how-to-get-the-sha-1-md5-checksum-of-a-file-with-qt
    // Returns empty QByteArray() on failure.
    QFile f(iFileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }

    throw MqaException(QString("(SHA1) Cannot open and read file " + iFileName));
}

Hash::Hash()
{

}
