QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# DYNAMIC
INCLUDEPATH += "curlx64-windows/include"
LIBS += "curlx64-windows/lib/libcurl.lib"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ftp_controller.cpp \
    main.cpp \
    easyftp.cpp \
    misc/loghelper.cpp \
    ui/rclickmenu.cpp \
    ui/tree.cpp

HEADERS += \
    easyftp.h \
    ftp_controller.h \
    misc/loghelper.h \
    ui/rclickmenu.h \
    ui/tree.h

FORMS += \
    easyftp.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
