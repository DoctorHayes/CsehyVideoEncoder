#-------------------------------------------------
#
# Project created by QtCreator 2019-06-15T10:55:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CsehyVideoEncoder2
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

CONFIG += c++14

SOURCES += \
        encoderthread.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        encoderthread.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

VERSION = 2.0.0.0
QMAKE_TARGET_COMPANY = Csehy Summer School of Music
QMAKE_TARGET_PRODUCT = Csehy Video Encoder
QMAKE_TARGET_DESCRIPTION = A front-end for ffmpeg to trim, combine, and add lower thirds to videos.
QMAKE_TARGET_COPYRIGHT = Copyright 2019 by Sean T. Hayes

# Copy encode.bat to the build directory
copydata.commands = $(COPY_FILE) \"$$shell_path($$PWD/encode.bat)\" \"$$shell_path($$OUT_PWD)\"
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

RESOURCES += \
    resources.qrc

RC_ICONS = images/footage.ico

DISTFILES += \
    README.md \
    encode.bat
