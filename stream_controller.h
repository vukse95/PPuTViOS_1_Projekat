#ifndef __STREAM_CONTROLLER_H__
#define __STREAM_CONTROLLER_H__

#include "tables.h"
#include "tdp_api.h"
#include "tables.h"
#include "pthread.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define VOLUME_SCALE 160400000

/**
 * @brief Structure that defines user config file parameters
 */
typedef struct _Config
{
	uint32_t frequency;
	uint32_t bandwidth;     /* Can be smaller... */
	t_Module module;        /* enum */
	uint16_t programNumber;
}InputConfig;

/**
 * @brief Structure that defines stream controller error
 */
typedef enum _StreamControllerError
{
	SC_NO_ERROR = 0,
	SC_ERROR,
	SC_THREAD_ERROR
}StreamControllerError;

/**
 * @brief Structure that defines channel info
 */
typedef struct _ChannelInfo
{
	int16_t programNumber;
	int16_t audioPid;
	int16_t videoPid;
	int16_t hasTeletext;
	char eventName[128];
	char eventGenre[128];
}ChannelInfo;

/**
 * @brief Structure that defines current name and genre
 */
typedef struct _eitBufferElement
{
	int16_t programNumber;
	char name[128];
	char genre[128];
}eitBufferElement;

/**
 * @brief Initializes stream controller module
 *
 * @return stream controller error code
 */
StreamControllerError streamControllerInit(InputConfig inputConfig);

/**
 * @brief Deinitializes stream controller module
 *
 * @return stream controller error code
 */
StreamControllerError streamControllerDeinit();

/**
 * @brief Channel up
 *
 * @return stream controller error
 */
StreamControllerError channelUp();

/**
 * @brief Channel down
 *
 * @return stream controller error
 */
StreamControllerError channelDown();

/**
 * @brief Returns current channel info
 *
 * @param [out] channelInfo - channel info structure with current channel info
 *
 * @return Stream controller error code
 */
StreamControllerError getChannelInfo(ChannelInfo* channelInfo);

typedef void (*ProgramTypeCallback)(int16_t type);

/**
 * @brief - Video PID callback
 *
 * @param [in] programTypeCallback - pointer to registerProgramTypeCallback function
 *
 * @return - Stream controller error
 */
StreamControllerError registerProgramTypeCallback(ProgramTypeCallback programTypeCallback);

/**
 * @brief - Returns total number of channels.
 *
 * @return - Total number of channels.
 */
uint8_t getNumberOfChannels();

/**
 * @brief - Sets the channel change flag and channel number
 *
 * @param channel - New channel number
 *
 * @return - void
 */
void changeChannelExtern(int16_t channelNumber);

/**
 * @brief - Gets EIT info by current channel
 *
 * @return - eitBufferElement
 */
eitBufferElement* eitTableGet();

/**
 * @brief - Fills or refreshes local buffer with current channel EIT info
 *
 * @param eitTableElement - EIT table element
 *
 * @return - void
 */
void eitBufferFilling(EitTable* eitTableElement);

/**
 * @brief - Sets audio stream volume from input
 *
 * @param volume - Volume level
 *
 * @return - void
 */
void setVolume(uint8_t volume);

#endif /* __STREAM_CONTROLLER_H__ */
