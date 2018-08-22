QT       += widgets

TARGET = QtTiffTagViewer

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tifffile.cpp \
    optionsdialog.cpp

HEADERS += \
        mainwindow.h \
    tifffile.h \
    optionsdialog.h

FORMS += \
        mainwindow.ui \
    optionsdialog.ui
