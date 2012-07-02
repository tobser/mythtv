include ( ../../mythconfig.mak )
include ( ../../settings.pro )
include ( ../../programs-libs.pro )

INCLUDEPATH += $${SYSROOT}$${PREFIX}/include/mythtv/libmythtv

QT += network xml sql
TEMPLATE = lib
CONFIG += debug plugin 
TARGET = mythteatime
target.path = $${LIBDIR}/mythtv/plugins
INSTALLS += target

uifiles.path = $${PREFIX}/share/mythtv/themes/default
uifiles.files = teatime-ui.xml
installfiles.path = $${PREFIX}/share/mythtv
installfiles.files = teatime-ui.xml

INSTALLS += uifiles

# Input
HEADERS += teatimeui.h teatimeuisettings.h teatimedata.h 
SOURCES += main.cpp teatimeui.cpp teatimedata.cpp teatimeuisettings.cpp

macx {
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
}
