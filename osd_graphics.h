#ifndef __OSD_GRAPHICS_H__
#define __OSD_GRAPHICS_H__


#include <stdint.h>

typedef enum _GraphicControllerError
{
    GRAPHIC_NO_ERROR = 0,
    GRAPHIC_ERROR
} GraphicControllerError;

typedef struct _GraphicControllerFlags
{
    int32_t audioPid;
    int32_t videoPid;
    uint16_t channelNum;
    uint8_t volume;
    uint8_t isMuted;
    uint8_t hasTeletext;
    uint8_t drawBlack;
    uint8_t draw;
    uint8_t drawVolume;
} GraphicControllerFlags;

GraphicControllerError graphicInit(void);
GraphicControllerError graphicDeinit(void);
GraphicControllerFlags* getGraphicFlags(void);

#endif
