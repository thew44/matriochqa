#ifndef PROTECTED_H
#define PROTECTED_H

#include <QMutex>

template<typename T>
class Protected
{
public:
    Protected()
    {
    }

    Protected(const Protected& i_src)
    {
        m_obj = i_src.m_obj;
    }

    T& acquire (void)
    {
        m_mtx.lock();
        return m_obj;
    }

    void release()
    {
        m_mtx.unlock();
    }

private:
    T m_obj;
    QMutex m_mtx;
};

#endif // PROTECTED_H
