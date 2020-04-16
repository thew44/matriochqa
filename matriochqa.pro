QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle
# Comment to disable http commands server
# This will kill dependency to QHttpServer and QtNetwork
CONFIG += mqa_command_server

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/utils/mqaexception.cpp \
    src/matriochqa.cpp \
    src/utils/logger.cpp \
    src/model/emuconfig.cpp \
    src/model/emuinstance.cpp \
    src/server/mdgenerator.cpp \
    src/utils/hash.cpp \
    src/server/cmdserver.cpp \
    src/model/mqaconfig.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += src

HEADERS += \
    3rdparty/fast-cpp-csv-parser/csv.h \
    src/model/types.h \
    src/utils/mqaexception.h \
    src/matriochqa.h \
    src/utils/logger.h \
    src/model/emuconfig.h \
    src/model/emuinstance.h \
    src/model/mqaconfig.h \
    src/server/mdgenerator.h \
    src/utils/hash.h \
    src/server/cmdserver.h \
    src/utils/protected.h \
    src/model/csvparser.h

# For server, necessary to send order with the html panel
mqa_command_server {
QT += network
DEFINES += ENABLE_HTTP_CMDSERVER

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../qhttpserver_dist/lib/ -lQt5HttpServer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../qhttpserver_dist/lib/ -lQt5HttpServerd
else:unix:!macx: LIBS += -L$$PWD/../qhttpserver_dist/lib/ -lQt5HttpServer

INCLUDEPATH += $$PWD/../qhttpserver_dist/include
DEPENDPATH += $$PWD/../qhttpserver_dist/include
}

DISTFILES += \
    README.md
