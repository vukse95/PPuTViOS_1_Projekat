#ifndef __OSD_GRAPHICS_H__
#define __OSD_GRAPHICS_H__


#include <stdint.h>

typedef enum _OsdGraphicsError
{
    OSD_NO_ERROR = 0,
    OSD_ERROR
} OsdGraphicsError;

typedef struct _OsdGraphicsInfo
{
    int32_t audioPid;
    int32_t videoPid;
    uint16_t channelNumber;
    uint8_t volume;
    uint8_t isMuted;
    uint8_t hasTeletext;
    uint8_t drawBlack;
    uint8_t draw;
    uint8_t drawVolume;
}OsdGraphicsInfo;

OsdGraphicsError OsdInit(void);
OsdGraphicsError OsdDeinit(void);
OsdGraphicsInfo* getOsdInfo(void);

#endif
