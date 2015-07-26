TEMPLATE = app
TARGET = LiberaBrilliancePlus
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += C++0x
QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH += /usr/local/include/tango /usr/include/omniORB4 /usr/include/libera-2.8
LIBS += -L/opt/libera/lib -L/usr/local/lib -ltango -llog4tango -lomniORB4 -lomnithread -lomniDynamic4 -lliberamci2.8 -lliberaistd2.8
release:LIBS += -Wl,-rpath,.
QMAKE_CXXFLAGS += -Wextra -Wall


SOURCES += \
    main.cpp \
    LiberaSignalSAHistory.cpp \
    LiberaSignal.cpp \
    LiberaLogsAttr.cpp \
    LiberaClient.cpp \
    LiberaBrilliancePlusStateMachine.cpp \
    LiberaBrilliancePlusClass.cpp \
    LiberaBrilliancePlus.cpp \
    LiberaAttr.cpp \
    ClassFactory.cpp

OTHER_FILES += \
    tango-ds-sequence.txt \
    tango-ds-sequence.png \
    tango-ds-classes.txt \
    tango-ds-classes.png \
    Makefile \
    LiberaBrilliancePlus.xmi \
    debian_control/preinst \
    debian_control/postinst \
    debian_control/control \
    build.xml \
    LiberaBrilliancePlus.sh \
    configure.sh \
    debian_control/prerm

HEADERS += \
    LiberaSignalSAHistory.h \
    LiberaSignalAttr.h \
    LiberaSignal.h \
    LiberaScalarAttr.h \
    LiberaLogsAttr.h \
    LiberaClient.h \
    LiberaBrilliancePlusClass.h \
    LiberaBrilliancePlus.h \
    LiberaAttr.h \
    documentation.h

