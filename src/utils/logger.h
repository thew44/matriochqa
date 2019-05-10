#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
private:
    Logger();

public:
    static void Log(const QString& i_msg);

protected:
    void internal_log(const QString& i_msg);

private:
    static Logger gLoggerInstance;
};

static void mqaLog(const QString& i_msg) { Logger::Log(i_msg); }
static void mqaErr(const QString& i_msg) { Logger::Log("ERROR: " + i_msg); }
static void mqaWarn(const QString& i_msg) { Logger::Log("WARN: " + i_msg); }

#endif // LOGGER_H
