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

        // Parse command line arguments
        QString cfg_file = "matriochqa.ini";
        QStringList args = a.arguments();
        if (args.size() >= 3 && args[1] == "-c")
        {
            cfg_file = args[2];
        }
        mqaLog(QString("General configuration file is '%1'").arg(cfg_file));

        // Init general config & command server
        Matriochqa mqa(cfg_file);

        // Load and prepare instance configuration
        mqa.loadconfig();
        mqa.prepare_all();
        mqa.setup_config_surveillance();

        mqaLog("Started - entering event loop");

        return a.exec();
    }
    catch (const MqaException& ex)
    {
        mqaErr(ex.msg());
    }
}
