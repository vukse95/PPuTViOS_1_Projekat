CROSS_COMPILE=$(TOOLCHAIN_CROSS_COMPILE)

CC_PREFIX=$(CROSS_COMPILE)-
CC=$(CC_PREFIX)gcc
CXX=$(CC_PREFIX)g++
LD=$(CC_PREFIX)ld

SYSROOT=$(SDK_ROOTFS)
GALOIS_INCLUDE=$(SDK_GALOIS)
ROOTFS_PATH=$(SDK_ROOTFS)

INCS =	-I./../tdp_api
INCS += -I./include/ 							\
		-I$(SYSROOT)/usr/include/         		\
		-I$(GALOIS_INCLUDE)/Common/include/     \
		-I$(GALOIS_INCLUDE)/OSAL/include/		\
		-I$(GALOIS_INCLUDE)/OSAL/include/CPU1/	\
		-I$(GALOIS_INCLUDE)/PE/Common/include/	\
		-I$(ROOTFS_PATH)/usr/include/    		\
        -I$(ROOTFS_PATH)/usr/include/directfb/

LIBS_PATH = -L./../tdp_api

LIBS_PATH += -L$(SYSROOT)/home/galois/lib/

LIBS_PATH += -L$(ROOTFS_PATH)/home/galois/lib/directfb-1.4-6-libs

LIBS := $(LIBS_PATH) -ltdp

LIBS += $(LIBS_PATH) -lOSAL	-lshm -lPEAgent

LIBS += $(LIBS_PATH) -ldirectfb -ldirect -lfusion -lrt

CFLAGS += -D__LINUX__ -O0 -Wno-psabi --sysroot=$(SYSROOT)

CXXFLAGS = $(CFLAGS)

all: parser_playback_sample

SRCS =  ./vezba_5.c
SRCS += ./tables_parser.c ./remote_controller.c ./stream_controller.c
SRCS += ./osd_graphics.c

parser_playback_sample:
	$(CC) -o TV_App $(INCS) $(SRCS) $(CFLAGS) $(LIBS)
    
clean:
	rm -f TV_App
copy:
	cp TV_App ../../ploca/
