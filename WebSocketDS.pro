TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

DEFINES += USELOG

INCLUDEPATH += /usr/local/include/tango \
            /usr/local/include  \
            /usr/local/include/omniORB4 \
            /usr/local/include/omnithread \
            /usr/include/mysql \
            wstangoproc
            
SOURCES += main.cpp \
wstangoproc/*.h \
wstangoproc/*.cpp \
*.cpp \
*.h

HEADERS += \
    common.h
