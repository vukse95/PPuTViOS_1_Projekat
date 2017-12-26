#ifndef __OSD_GRAPHICS_H__
#define __OSD_GRAPHICS_H__

#include <stdint.h>

/**
 * @brief - OSD error codes.
 */
typedef enum _OsdGraphicsError
{
    OSD_NO_ERROR = 0,
    OSD_ERROR
} OsdGraphicsError;

/**
 * @brief - OSD information.
 */
typedef struct _OsdGraphicsInfo
{
    int32_t audioPid;
    int32_t videoPid;
    uint16_t channelNumber;
    uint8_t volume;
    uint8_t isMuted;
    uint8_t hasTeletext;
	char eventGenre[50];
	char eventName[50];
    uint8_t drawBlack;
    uint8_t draw;
	uint8_t drawRadio;
    uint8_t drawVolume;
	uint8_t timerSetVolume;
	uint8_t timerSetProgram; 
}OsdGraphicsInfo;

/**
 * @brief - Initializes OSD module.
 *
 * @return - OSD error code.
 */
OsdGraphicsError OsdInit(void);

/**
 * @brief - Deinitializes OSD module.
 *
 * @return - OSD error code.
 */
OsdGraphicsError OsdDeinit(void);

/**
 * @brief - Returns the pointer to the OSD info.
 *
 * @return - Pointer to the OSD info.
 */
OsdGraphicsInfo* getOsdInfo(void);

#endif
