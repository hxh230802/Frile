#-------------------------------------------------
#
# Project created by QtCreator 2023-09-10T00:09:49
#
#-------------------------------------------------

QT       += core gui ftp
QT       += gui-private

#CONFIG += objective_c++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FrilesM
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(C:/myQtLib/V1_0_0_0/myQtLib.pri)

RC_FILE += detailed.rc

SOURCES += \
        main.cpp \
        frilesm.cpp \
    filereceiver.cpp \
    baseinfo.cpp \
    classselectiondialog.cpp \
    lessonmanager.cpp \
    settingwnd.cpp

HEADERS += \
        frilesm.h \
    filereceiver.h \
    baseinfo.h \
    classselectiondialog.h \
    UAC.h \
    lessonmanager.h \
    settingwnd.h

FORMS += \
        frilesm.ui \
    filereceiver.ui \
    settingwnd.ui

RESOURCES += \
    pic.qrc
