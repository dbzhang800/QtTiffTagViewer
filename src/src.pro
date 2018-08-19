QT       += widgets

TARGET = QtTiffTagViewer

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tifffile.cpp

HEADERS += \
        mainwindow.h \
    tifffile.h

FORMS += \
        mainwindow.ui
