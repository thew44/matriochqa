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

    T* try_acquire (void)
    {
        if (m_mtx.tryLock())
        {
            return &m_obj;
        }
        else
        {
            return nullptr;
        }
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
