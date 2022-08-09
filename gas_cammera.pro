QT -= gui

TEMPLATE = lib
DEFINES += GAS_CAMMERA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS+= -L/usr/lib/x86_64-linux-gnu/libjpeg.so -ljpeg
LIBS += -L/usr/lib/x86_64-linux-gnu  -lavcodec -lavformat -lavutil -lavcodec -ljpeg
SOURCES += \
    Queue.c \
    capture.c \
    encode.c \
    functions_tools.c \
    gas_cammera.c \
    rgb_converter.c \
    save_snapshot.c \
    write_record.c \
    yuv_converter.c

HEADERS += \
    Queue.h \
    capture.h \
    encode.h \
    functions_tools.h \
    gas_cammera.h \
    rgb_converter.h \
    save_snapshot.h \
    stage_lib.h \
    write_record.h \
    yuv_converter.h\
    jpeglib.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    gas_cammera.pro.user
