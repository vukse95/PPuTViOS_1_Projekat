#include "remote_controller.h"
#include "stream_controller.h"
#include "osd_graphics.h"
#include <signal.h>


static inline void textColor(int32_t attr, int32_t fg, int32_t bg)
{
   char command[13];

   /* command is the control command to the terminal */
   sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
   printf("%s", command);
}

/* macro function for error checking */
#define ERRORCHECK(x)                                                       \
{                                                                           \
if (x != 0)                                                                 \
 {                                                                          \
    textColor(1,1,0);                                                       \
    printf(" Error!\n File: %s \t Line: <%d>\n", __FILE__, __LINE__);       \
    textColor(0,7,0);                                                       \
    return -1;                                                              \
 }                                                                          \
}

static void remoteControllerCallback(uint16_t code, uint16_t type, uint32_t value);
static pthread_cond_t deinitCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t deinitMutex = PTHREAD_MUTEX_INITIALIZER;
static ChannelInfo channelInfo;
static InputConfig configInputConfig;

int configFileRead(char fileName[]);
void timeOutChannelTrigger();

static timer_t keyTimer;
static struct itimerspec timerSpec;
static struct itimerspec timerSpecOld;
static struct sigevent signalEvent;
static int32_t timerFlags = 0;

static int32_t pressedKeys[3];
static int8_t pressedKeysCounter = 0;
static int8_t anyKeyPressedFlag = 0;

static int8_t mutedVolume = 0;
static int8_t mutePressed = 0;


int main(int argc, char *argv[])
{
	char *ret;

	/* parse input arguments */
	if(argc < 2)
	{
		printf("\nInput ERROR, config file now found!\n\n");
		printf("Usage: TV_App config_file\n");
		return 0;
	}
	else
	{	
		ret = strchr(argv[1], '.');
		if(strcmp(ret, ".ini") != 0)
		{
			printf("\nWrong configuration file type!!\n");
			return 0;
		}
		else
		{
			configFileRead(argv[1]);
		}
	}

	signalEvent.sigev_notify = SIGEV_THREAD;
	signalEvent.sigev_notify_function = timeOutChannelTrigger;
	signalEvent.sigev_value.sival_ptr = NULL;
	signalEvent.sigev_notify_attributes = NULL;
	timer_create(CLOCK_REALTIME, &signalEvent, &keyTimer);

	printf("\nTimer init!\n");

	memset(&timerSpec, 0, sizeof(timerSpec));
	timerSpec.it_value.tv_sec = 2;
	timerSpec.it_value.tv_nsec = 0;

    /* initialize remote controller module */
    ERRORCHECK(remoteControllerInit());

	/* initialize graphic controller module */
	ERRORCHECK(OsdInit());
    
    /* register remote controller callback */
    ERRORCHECK(registerRemoteControllerCallback(remoteControllerCallback));
    
    /* initialize stream controller module */
    ERRORCHECK(streamControllerInit(configInputConfig));

    /* wait for a EXIT remote controller key press event */
    pthread_mutex_lock(&deinitMutex);
	if (ETIMEDOUT == pthread_cond_wait(&deinitCond, &deinitMutex))
	{
		printf("\n%s : ERROR Lock timeout exceeded!\n", __FUNCTION__);
	}
	pthread_mutex_unlock(&deinitMutex);
    
    /* unregister remote controller callback */
    ERRORCHECK(unregisterRemoteControllerCallback(remoteControllerCallback));

    /* deinitialize remote controller module */
    ERRORCHECK(remoteControllerDeinit());
	
	/* deinitialize graphic controller module */
	ERRORCHECK(OsdDeinit());

    /* deinitialize stream controller module */
    ERRORCHECK(streamControllerDeinit());
	
  
    return 0;
}

void remoteControllerCallback(uint16_t code, uint16_t type, uint32_t value)
{

	OsdGraphicsInfo* osd = getOsdInfo();
	eitBufferElement *eitTable = eitTableGet();
	
	if(code >= KEYCODE_1 && code <= KEYCODE_0)
	{
		if(pressedKeysCounter == 0)
		{
			timer_settime(keyTimer, timerFlags, &timerSpec, &timerSpecOld);
			pressedKeys[0] = 0;
			pressedKeys[1] = 0;
			pressedKeys[2] = 0;

			pressedKeys[0] = code;
			pressedKeysCounter++;			
		}
		else if (pressedKeysCounter == 1)
		{
			pressedKeys[1] = code;
			pressedKeysCounter++;
		}
		else if(pressedKeysCounter >= 2)
		{
			if(anyKeyPressedFlag == 0)
			{
				pressedKeys[2] = code;
				anyKeyPressedFlag = 1;
			}
		}
	
		fflush(stdout);
	}
	else
	{
		switch(code)
		{
			case KEYCODE_INFO:
            	printf("\nInfo pressed\n");          
	            if (getChannelInfo(&channelInfo) == SC_NO_ERROR)
    	        {
    	            printf("\n********************* Channel info *********************\n");
    	            printf("Program number: %d\n", channelInfo.programNumber);
    	            printf("Audio pid: %d\n", channelInfo.audioPid);
    	            printf("Video pid: %d\n", channelInfo.videoPid);
    	            printf("**********************************************************\n");
					//printf("Name:%s\n", eitTable[]->name);					
					eitTableGet();

					osd->audioPid = channelInfo.audioPid;
					osd->videoPid = channelInfo.videoPid;
					osd->channelNumber = channelInfo.programNumber;
					osd->draw = 1;
    	        }
				break;
			case KEYCODE_P_PLUS:
				printf("\nCH+ pressed\n");
    	        channelUp();
				/* TODO: Odraditi bolje ako ostane vremena */
				usleep(900000);
				if (getChannelInfo(&channelInfo) == SC_NO_ERROR)
				{
					osd->audioPid = channelInfo.audioPid;
					osd->videoPid = channelInfo.videoPid;
					osd->channelNumber = channelInfo.programNumber;
					osd->draw = 1;
				}
				break;
			case KEYCODE_P_MINUS:
			    printf("\nCH- pressed\n");
    	        channelDown();
				/* TODO: Odraditi bolje ako ostane vremena */
				usleep(900000);
				if (getChannelInfo(&channelInfo) == SC_NO_ERROR)
				{
					osd->audioPid = channelInfo.audioPid;
					osd->videoPid = channelInfo.videoPid;
					osd->channelNumber = channelInfo.programNumber;
					osd->draw = 1;
				}
				break;
			case KEYCODE_VOL_UP:
				printf("\nVOL+ pressed\n");
				osd->drawVolume = 1;
				if(osd->volume >=0 && osd->volume < 10)
				{
					osd->volume++;
					setVolume(osd->volume);
					/* TODO: Povecati volume na set top-boxu */
				}
				break;
			case KEYCODE_VOL_DOWN:
				printf("\nVOL- pressed\n");
				osd->drawVolume = 1;
				if(osd->volume > 0 && osd->volume <= 10)
				{
					osd->volume--;
					setVolume(osd->volume);
					/* TODO: Smanjiti volume na set top-boxu */
				}
				break;
			case KEYCODE_MUTE:
				printf("\nMUTE pressed\n");
				/* TODO: MUTE odraditi i na set top-boxu */
				if(mutePressed == 0)
				{
					mutedVolume = osd->volume;
					osd->volume = 0;
					mutePressed = 1;
					setVolume(osd->volume);
				}
				else if(mutePressed == 1)
				{
					osd->volume = mutedVolume;
					setVolume(mutedVolume);
					mutePressed = 0;
				}
				osd->drawVolume = 1;
				break;
			case KEYCODE_EXIT:
				printf("\nExit pressed\n");
    	        pthread_mutex_lock(&deinitMutex);
			    pthread_cond_signal(&deinitCond);
			    pthread_mutex_unlock(&deinitMutex);
				break;
			/* TODO: Odraditi za volume i za mute! */
			default:
				printf("\nPress P+, P-, info or exit! \n\n");
		}
	}
}

int configFileRead(char fileName[])
{
	FILE *filePtr;

	char *fileLineBuffer[128];
	char *lineBuffer;
	size_t bufferSize = 128;
	ssize_t read;

	char paramValue[10];
	int paramValueInt = 0;

	int paramValueCounter = 0;
	int paramValueCounterAux = 0;

	/* Pointing to static array to avoid memory leak in case of */
	/* premature closure of the program */
	lineBuffer = &fileLineBuffer;

	filePtr = fopen(fileName, "r");

	if(filePtr == NULL)
	{
		printf("\nERROR opening file %s\n", fileName);
		return 0;
	}

	/* getline bugs occasionally, replace! */
	while((read = getline(&lineBuffer, &bufferSize, filePtr)) != -1)
	{
 		/* Check if first char is '#' thats is comment or empty line */
		if(lineBuffer[0] != '#' && lineBuffer != '\0')
		{
			if(strstr(lineBuffer, "Freq") != NULL)
			{
				paramValueCounter = 0;
				memset(paramValue,'\0',sizeof(paramValue)); /* Cleaning paramValue buffer */
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValueCounter++;
				}
				paramValueCounter += 1;
				paramValueCounterAux = 0;
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValue[paramValueCounterAux] = lineBuffer[paramValueCounter];
					paramValueCounter++;
					paramValueCounterAux++;
				}
				paramValueInt = atoi(paramValue);
				configInputConfig.frequency = paramValueInt;
				printf("\nParam Value[Freq]:%d", paramValueInt);
			}
			else if(strstr(lineBuffer, "Bandwidth") != NULL)
			{
				paramValueCounter = 0;
				memset(paramValue,'\0',sizeof(paramValue)); /* Cleaning paramValue buffer */
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValueCounter++;
				}
				paramValueCounter += 1;
				paramValueCounterAux = 0;
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValue[paramValueCounterAux] = lineBuffer[paramValueCounter];
					paramValueCounter++;
					paramValueCounterAux++;
				}
				paramValueInt = atoi(paramValue);
				configInputConfig.bandwidth = paramValueInt;
				printf("\nParam Value[Bandwidth]:%d", paramValueInt);
			}
			else if(strstr(lineBuffer, "Module") != NULL)
			{
				paramValueCounter = 0;
				memset(paramValue,'\0',sizeof(paramValue)); /* Cleaning paramValue buffer */
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValueCounter++;
				}
				paramValueCounter += 1;
				paramValueCounterAux = 0;
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValue[paramValueCounterAux] = lineBuffer[paramValueCounter];
					paramValueCounter++;
					paramValueCounterAux++;
				}
				if(strcmp(paramValue, "DVB_T") == 0)
				{
					configInputConfig.module = DVB_T;
				}
				else if(strcmp(paramValue, "DVB_T2") == 0)
				{
					configInputConfig.module = DVB_T2;
				}
				printf("\nParam Value[Module]:%s", paramValue);
			}
			else if(strstr(lineBuffer, "ProgramNumber") != NULL)
			{
				paramValueCounter = 0;
				memset(paramValue,'\0',sizeof(paramValue));
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValueCounter++;
				}
				paramValueCounter += 1;
				paramValueCounterAux = 0;
				while(lineBuffer[paramValueCounter] != '"')
				{
					paramValue[paramValueCounterAux] = lineBuffer[paramValueCounter];
					paramValueCounter++;
					paramValueCounterAux++;
				}
				paramValueInt = atoi(paramValue);
				configInputConfig.programNumber = paramValueInt;
				printf("\nParam Value[ProgramNumber]:%d", paramValueInt);
			}
		}
	}

	fclose(filePtr);
}

void timeOutChannelTrigger()
{
	int convertedKey = 0;
	OsdGraphicsInfo* osd = getOsdInfo();

	anyKeyPressedFlag = 0;
	pressedKeysCounter = 0;

	if(pressedKeys[0] == 11)
	{
		pressedKeys[0] = 1;
	}
	if(pressedKeys[1] == 11)
	{
		pressedKeys[1] = 1;
	}
	if(pressedKeys[1] == 11)
	{
		pressedKeys[1] = 1;
	}

	if(pressedKeys[0] != 0 && pressedKeys[1] == 0 && pressedKeys[2] == 0)
	{		
		convertedKey = pressedKeys[0] - 1;
	}
	else if(pressedKeys[0] != 0 && pressedKeys[1] != 0 && pressedKeys[2] == 0)
	{
		convertedKey = (pressedKeys[0] - 1) * 10;
		convertedKey += pressedKeys[1] - 1;
	}
	else if(pressedKeys[0] != 0 && pressedKeys[1] != 0 && pressedKeys[2] != 0)
	{		
		convertedKey = (pressedKeys[0] - 1) * 100;
		convertedKey += (pressedKeys[1] - 1) * 10;
		convertedKey += pressedKeys[2] - 1;

	}
	printf("\nUnet broj%d\n", convertedKey);
	fflush(stdout);
	changeChannelExtern(convertedKey);
	/* TODO: Odraditi bolje ako ostane vremena */
	usleep(900000);
	if (getChannelInfo(&channelInfo) == SC_NO_ERROR)
	{
		osd->audioPid = channelInfo.audioPid;
		osd->videoPid = channelInfo.videoPid;
		osd->channelNumber = channelInfo.programNumber;
		osd->draw = 1;
	}
}






