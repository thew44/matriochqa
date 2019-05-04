#include <QCoreApplication>
#include <iostream>

#include "matriochqa.h"
#include "utils/mqaexception.h"
#include "utils/logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        mqaLog("Starting Matriochqa");
        mqaLog(QString("Build date: %1 - %2").arg(__DATE__).arg(__TIME__));

        QString config_file = a.arguments()[0];
        config_file = "C:/Users/mallory/Documents/perso/emu.csv";
        Matriochqa mqa;
        mqa.loadconfig();
        mqa.prepare_all();
        mqa.setup_config_surveillance();

        mqaLog("Started - entering event loop");

        return a.exec();

    }
    catch (const MqaException& ex)
    {
        std::cout << "Exception: " << ex.msg().toStdString() << std::endl;
    }
}
