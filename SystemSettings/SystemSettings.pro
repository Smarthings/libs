#-------------------------------------------------
#
# Project created by QtCreator 2017-12-27T12:23:00
#
#-------------------------------------------------

QT       += network core sql
CONFIG += c++11
QT       -= gui

TARGET = SystemSettings
TEMPLATE = lib

DEFINES += SYSTEMSETTINGS_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        systemsettings.cpp \
        wireless.cpp \
    databasesettings.cpp \
    screen.cpp \
    logs.cpp

HEADERS += \
        systemsettings.h \
        systemsettings_global.h \ 
        wireless.h \
    databasesettings.h \
    screen.h \
    logs.h

DISTFILES += \
    sql/Wireless.sql

unix {
    headers.path = /usr/lib/HomeThings
    headers.files = $$HEADERS
    target.path = /usr/lib

    sql_files.path = /etc/HomeThings/sql
    sql_files.files = sql/Wireless.sql

    INSTALLS += target
    INSTALLS += headers
    INSTALLS += sql_files
}
