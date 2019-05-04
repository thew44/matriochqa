#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>

class MqaException
{
public:
    MqaException(const QString& i_msg);

    inline const QString& msg() const { return m_msg; }

private:
    QString m_msg;
};

#endif // EXCEPTION_H
