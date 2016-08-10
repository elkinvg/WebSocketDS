TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

DEFINES += DBNAME DBUSER DBPASS DBHOST

INCLUDEPATH += /usr/local/include/tango \
            /usr/local/include  \
            /usr/local/include/omniORB4 \
            /usr/local/include/omnithread \
            /usr/include/mysql
            
SOURCES += main.cpp \
*.cpp \
*.h \
    WSThread_tls.cpp \
    WSThread_plain.cpp

HEADERS += \
