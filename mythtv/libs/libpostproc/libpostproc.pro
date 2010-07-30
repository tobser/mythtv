include ( ../../settings.pro )

TEMPLATE = lib
TARGET = mythpostproc-$$LIBVERSION
CONFIG += thread dll warn_off
CONFIG -= qt
target.path = $${LIBDIR}
INSTALLS = target

QMAKE_LFLAGS += $$LDFLAGS

INCLUDEPATH = .. ../..

LIBS += -L../libavutil -lmythavutil-$$LIBVERSION

DEFINES += HAVE_AV_CONFIG_H

# Debug mode on x86 must compile without -fPIC
# otherwise gcc runs out of registers.
debug:contains(ARCH_X86_32, yes) {
        QMAKE_CFLAGS_SHLIB = -O1
}

QMAKE_CLEAN += $(TARGET) $(TARGETA) $(TARGETD) $(TARGET0) $(TARGET1) $(TARGET2)

# Input
SOURCES += postprocess.c

inc.path = $${PREFIX}/include/mythtv/libpostproc/
inc.files  = postprocess.h

INSTALLS += inc

contains( ARCH_ALPHA, yes ) {
    QMAKE_CFLAGS_RELEASE += -fforce-addr -freduce-all-givs
}

include ( ../libs-targetfix.pro )