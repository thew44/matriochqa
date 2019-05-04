#ifndef SHA1_H
#define SHA1_H

#include <QByteArray>
#include <QString>

class Hash
{
public:
    static QByteArray SHA1(const QString& iFileName);

protected:
    Hash();
};

#endif // SHA1_H
