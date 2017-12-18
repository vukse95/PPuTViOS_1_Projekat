#ifndef __STREAM_CONTROLLER_H__
#define __STREAM_CONTROLLER_H__

#include <stdio.h>
#include "tables.h"
#include "tdp_api.h"
#include "tables.h"
#include "pthread.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

/* Change ERROR message to be by structure not define */
#define DESIRED_FREQUENCY 754000000	        /* Tune frequency in Hz */
#define BANDWIDTH 8    				        /* Bandwidth in Mhz */
#define MODULE_NAME_SIZE 8					/* example DVB-T2 */

typedef struct _Config
{
	uint32_t frequency;
	uint32_t bandwidth;	/* Can be smaller... */
	t_Module module;	/* enum */
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
}ChannelInfo;

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
 * @return stream controller error code
 */
StreamControllerError getChannelInfo(ChannelInfo* channelInfo);



/* TODO:Napisati Dokumentaciju */
void changeChannelExtern(int16_t channelNumber);

#endif /* __STREAM_CONTROLLER_H__ */
