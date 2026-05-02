QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17
TARGET    = StateMachineMinimizer
TEMPLATE  = app

# Treat warnings as errors for clean builds
# CONFIG += warn_on

SOURCES += \
    main.cpp \
    engine/ImplicationTable.cpp \
    ui/SetupDialog.cpp \
    ui/StateInputTable.cpp \
    ui/ResultWidget.cpp \
    ui/DiagramView.cpp \
    ui/MainWindow.cpp

HEADERS += \
    engine/StateTypes.h \
    engine/ImplicationTable.h \
    ui/SetupDialog.h \
    ui/StateInputTable.h \
    ui/ResultWidget.h \
    ui/DiagramView.h \
    ui/MainWindow.h

# Default rules for deployment (macOS / Linux / Windows)
qnx:     target.path = /tmp/$${TARGET}/bin
else:    unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
