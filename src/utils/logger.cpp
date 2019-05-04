#include "logger.h"
#include <iostream>

Logger Logger::gLoggerInstance = Logger();

Logger::Logger()
{
}

void Logger::internal_log(const QString &i_msg)
{
    std::cout << i_msg.toStdString() << std::endl;
}

void Logger::Log(const QString& i_msg)
{
    gLoggerInstance.internal_log(i_msg);
}
