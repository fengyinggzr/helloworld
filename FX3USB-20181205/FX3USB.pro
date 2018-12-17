#-------------------------------------------------
#
# Project created by QtCreator 2018-11-15T13:21:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FX3USB
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        FX3USBWindow.cpp \
    FX3USBDevice.cpp \
    FX3USBThread.cpp \
    DataAnalyse.cpp

HEADERS += \
        FX3USBWindow.h \
    inc/CyAPI.h \
    inc/cyioctl.h \
    inc/CyUSB30_def.h \
    inc/VersionNo.h \
    inc/CyAPI.h \
    inc/cyioctl.h \
    inc/CyUSB30_def.h \
    inc/usb100.h \
    inc/usb200.h \
    inc/VersionNo.h \
    FX3USBDevice.h \
    FX3USBThread.h \
    FX3USBInclude.h \
    DataAnalyse.h

FORMS += \
        FX3USBWindow.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/lib/ -lCyAPI


INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/CyAPI.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/lib/libCyAPI.a

win32: LIBS += -lSetupAPI

win32: LIBS += -lUser32

win32: LIBS += -llegacy_stdio_definitions
