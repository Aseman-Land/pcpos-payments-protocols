TARGET = qt-example
CONFIG += c++17 
QT += network

INCLUDEPATH += ..

SOURCES += \
    ../behpardakhtpacket.cpp \
    ../sadadpacket.cpp \
    behpardakhtpos.cpp \
    main.cpp \
    pcposmanager.cpp \
    sadadpos.cpp

HEADERS += \
    ../behpardakhtpacket.h \
    ../sadadpacket.h \
    behpardakhtpos.h \
    pcposmanager.h \
    sadadpos.h
