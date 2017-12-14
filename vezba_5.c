#include "remote_controller.h"
#include "stream_controller.h"


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

int configFileRead(char fileName[]);

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
			/* TODO: Parse return value*/
			configFileRead(argv[1]);
		}
	}

    /* initialize remote controller module */
    ERRORCHECK(remoteControllerInit());
    
    /* register remote controller callback */
    ERRORCHECK(registerRemoteControllerCallback(remoteControllerCallback));
    
    /* initialize stream controller module */
    ERRORCHECK(streamControllerInit());

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

    /* deinitialize stream controller module */
    ERRORCHECK(streamControllerDeinit());
  
    return 0;
}

void remoteControllerCallback(uint16_t code, uint16_t type, uint32_t value)
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
            }
			break;
		case KEYCODE_P_PLUS:
			printf("\nCH+ pressed\n");
            channelUp();
			break;
		case KEYCODE_P_MINUS:
		    printf("\nCH- pressed\n");
            channelDown();
			break;
		case KEYCODE_EXIT:
			printf("\nExit pressed\n");
            pthread_mutex_lock(&deinitMutex);
		    pthread_cond_signal(&deinitCond);
		    pthread_mutex_unlock(&deinitMutex);
			break;
		default:
			printf("\nPress P+, P-, info or exit! \n\n");
	}
}

int configFileRead(char fileName[])
{
	FILE *filePtr;

	char *fileLineBuffer[128];
	char *lineBuffer;
    size_t bufferSize = 128;
    ssize_t read;

	char paramName[10];
	char paramValue[10];

	uint8_t paramValueCounter = 0;
	
	lineBuffer = &fileLineBuffer; 
	filePtr = fopen(fileName, "r");
	
	if(filePtr == NULL)
	{
		printf("\nERROR opening file %s\n", fileName);
		return 0;
	}
	
	//lineBuffer = (char *)malloc(bufferSize * sizeof(char));

	if(lineBuffer == NULL)
	{
		printf("\nERROR allocating memory!\n");
		return 0;
	}
	/* getline bugs occasionally, replace! */
	while((read = getline(&lineBuffer, &bufferSize, filePtr)) != -1) 
	{
 		/* Check if first char is '#' thats is comment or empty line */
		if(lineBuffer[0] != '#' && lineBuffer != '\0')
		{	
			if(strstr(lineBuffer, "Module") != NULL)
			{
				printf("\n\nizlaz:%s", lineBuffer);
				paramValueCounter = 0;
				while(lineBuffer[paramValueCounter] != '"')
				{	
					paramValueCounter++;
				}
				printf("\nparamCounter:%d", paramValueCounter);
			}
			else if(strstr(lineBuffer, "Bandwidth") != NULL)
			{
				printf("\n\nizlaz:%s", lineBuffer);

			}
			else if(strstr(lineBuffer, "Freq") != NULL)
			{
				printf("\n\nizlaz:%s", lineBuffer);
			}
			else if(strstr(lineBuffer, "ProgramNumber") != NULL)
			{
				printf("\n\nizlaz:%s", lineBuffer);
			}
			
			
			//printf("\nparamName:%s", paramName);
			//printf("\nparamValue:%s", paramValue);
			//printf("\n");
				
		}
    }

	//free(lineBuffer);
	fclose(filePtr);
}








