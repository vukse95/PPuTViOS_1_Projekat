#include "osd_graphics.h"

#include <directfb.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#define DFBCHECK(x...)                                      \
{                                                           \
DFBResult err = x;                                          \
if (err != DFB_OK)                                          \
  {                                                         \
    fprintf(stderr, "%s <%d>:\n\t", __FILE__, __LINE__);    \
    DirectFBErrorFatal( #x, err );                          \
    return (void*)OSD_ERROR;                                \
  }                                                         \
}

/* DirectFB variables */
static IDirectFBSurface* primary = NULL;
static IDirectFB* dfbInterface = NULL;
static DFBSurfaceDescription surfaceDesc;

/* Screen size */
static int screenWidth;
static int screenHeight;

/* Volume logo parameters */
static IDirectFBImageProvider* provider = NULL;
static IDirectFBSurface* logoSurface = NULL;
static int32_t logoWidth;
static int32_t logoHeight;

/* Program timer variables */
static struct itimerspec timerSpecProgram;
static struct itimerspec timerSpecOldProgram;
int32_t timerFlagsProgram;
timer_t timerIdProgram;

/* Volume timer variables */
static struct itimerspec timerSpecVolume;
static struct itimerspec timerSpecOldVolume;
int32_t timerFlagsVolume;
timer_t timerIdVolume;

/* OSD thread  variables */
static pthread_t osdThread;
static int32_t threadExit;

/* OsdGraphicsInfo structure local instance */
static OsdGraphicsInfo OsdInfo;

/**
 * @brief - OSD thread.
 *
 * @param params
 *
 * @return 
 */
static void* OSDTask(void* params);


/**
 * @brief - Initialize program timer.
 */
static void timerInitProgram(void);

/**
 * @brief - Program timer callback which removes everything related to program info from the screen.
 */
static void clearScreenProgram(void);

/**
 * @brief - Initialize volume timer.
 */
static void timerInitVolume(void);

/**
 * @brief - Volume timer callback which removes everything related to volume info from the screen.
 */
static void clearScreenVolume(void);
OsdGraphicsError OsdInit(void) 
{
	/* Create OSD thread */
    if (pthread_create(&osdThread, NULL, &OSDTask, NULL))
    {
        printf("Error creating OSD thread\n");
        return OSD_ERROR;
    }

    return OSD_NO_ERROR;
}

void timerInitProgram(void)
{
    struct sigevent signalEvent;

    /* set sigevent callback */
    signalEvent.sigev_notify = SIGEV_THREAD;
    signalEvent.sigev_notify_function = (void*)clearScreenProgram;
    signalEvent.sigev_value.sival_ptr = NULL;
    signalEvent.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &signalEvent, &timerIdProgram);
}

void timerInitVolume(void)
{
    struct sigevent signalEvent;

    /* set sigevent callback */
    signalEvent.sigev_notify = SIGEV_THREAD;
    signalEvent.sigev_notify_function = (void*)clearScreenVolume;
    signalEvent.sigev_value.sival_ptr = NULL;
    signalEvent.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &signalEvent, &timerIdVolume);
}

void* OSDTask(void* params)
{
    IDirectFBFont* fontInterface = NULL;
    DFBFontDescription fontDesc;
    char channelString[4];
    char audioPidString[50];
    char videoPidString[50];
	char teletext[] = "Teletext available";
	char noTeletext[] = "Teletext not available";

	//memset(&OsdInfo, 0, sizeof(OsdInfo));
	
    /* Initialize timer */
	timerInitProgram();
	timerInitVolume();

    /* Initialize directFB */
    DFBCHECK(DirectFBInit(NULL, NULL));

    /* Fetch the directFB interface */
    DFBCHECK(DirectFBCreate(&dfbInterface));

    /* Set full screen */
    DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));

    /* Create primary surface with double buffering */
    surfaceDesc.flags = DSDESC_CAPS;
    surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary);

    /* Fetch the screen size */
    DFBCHECK(primary->GetSize(primary, &screenWidth, &screenHeight));

    while (threadExit == 0)
    {
        /* check whether the screen should be black */
        if (OsdInfo.drawBlack == 1)
        {
            DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff));
            DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
        } 
        else
        {
            DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
            DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
        }

        /* if the draw flag has been set, draw the info banner */
        if (OsdInfo.draw == 1)
        {
            usleep(300000);

            /* set the timer to 3 seconds */
            if (OsdInfo.timerSetProgram == 0)
            {
                memset(&timerSpecProgram, 0, sizeof(timerSpecProgram));
                timerSpecProgram.it_value.tv_sec = 3;
                timerSpecProgram.it_value.tv_nsec = 0;
                timer_settime(timerIdProgram, timerFlagsProgram, &timerSpecProgram, &timerSpecOldProgram);

                OsdInfo.timerSetProgram = 1;
            }

            /* draw channel rectangle */
            DFBCHECK(primary->SetColor(primary, 0x00, 0x8c, 0x44, 0xff));
            DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth / 10, screenHeight / 8));

            DFBCHECK(primary->SetColor(primary, 0x00, 0xa6, 0x51, 0xff));
            DFBCHECK(primary->FillRectangle(primary, 10, 10, screenWidth / 10 - 20, screenHeight / 8 - 20));

            /* draw channel number */
            fontDesc.flags = DFDESC_HEIGHT;
            fontDesc.height = 48;
            sprintf(channelString, "%d", OsdInfo.channelNumber);
            DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
            DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff));
            DFBCHECK(primary->SetFont(primary, fontInterface));
            DFBCHECK(primary->DrawString(primary, channelString, -1, screenWidth / 10 - 110, screenHeight / 8 - 50, DSTF_LEFT));

            /* draw info rectangle */
            DFBCHECK(primary->SetColor(primary, 0x00, 0xa6, 0x51, 0xff));
            DFBCHECK(primary->FillRectangle(primary, screenWidth / 2 - 500, screenHeight * 6 / 8, 1000, screenHeight / 8));

            /* draw audio and video pid */
            sprintf(audioPidString, "Audio PID: %d", OsdInfo.audioPid);
            sprintf(videoPidString, "Video PID: %d", OsdInfo.videoPid);
            fontDesc.height = 28;
            DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
            DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff));
            DFBCHECK(primary->DrawString(primary, audioPidString, -1, screenWidth / 2 - 470, screenHeight * 6 / 8 + 50, DSTF_LEFT));
            DFBCHECK(primary->DrawString(primary, videoPidString, -1, screenWidth / 2 - 470, screenHeight * 6 / 8 + 100, DSTF_LEFT));            

            /* draw teletext if the channel has it */
            if (OsdInfo.hasTeletext == 1)
            {
                DFBCHECK(primary->DrawString(primary, teletext, -1, screenWidth / 2 - 50, screenHeight * 6 / 8 + 50, DSTF_LEFT));
            }
            else
            {
                DFBCHECK(primary->DrawString(primary, noTeletext, -1, screenWidth / 2 - 50, screenHeight * 6 / 8 + 50, DSTF_LEFT));
            }

            if (strlen(OsdInfo.eventName) > 1 && strlen(OsdInfo.eventGenre) > 1)
            {
                /* draw name and genre rectangle */
                DFBCHECK(primary->SetColor(primary, 0x00, 0xa6, 0x51, 0xff));
                DFBCHECK(primary->FillRectangle(primary, screenWidth / 2 - 500, screenHeight * 5 / 8, 1000, screenHeight / 8 - 20));

                /* draw name and genre */
                DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff));
                DFBCHECK(primary->DrawString(primary, OsdInfo.eventName+1, -1, screenWidth / 2 - 470, screenHeight * 5 / 8 + 50, DSTF_LEFT));
                DFBCHECK(primary->DrawString(primary, OsdInfo.eventGenre, -1, screenWidth / 2 - 470, screenHeight * 5 / 8 + 100, DSTF_LEFT));
            }
        }

        /* if the draw volume has been set, draw volume logo */
        if (OsdInfo.drawVolume == 1)
        {
            char volumePicture[50];

            /* set the picture file */
            sprintf(volumePicture, "volume_%d.png", OsdInfo.volume);

            /* create image provider */
            DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, volumePicture, &provider));
            DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
            DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &logoSurface));
            DFBCHECK(provider->RenderTo(provider, logoSurface, NULL));

            provider->Release(provider);

            /* fetch the logo size and blit it to the screen */
            DFBCHECK(logoSurface->GetSize(logoSurface, &logoWidth, &logoHeight));
            DFBCHECK(primary->Blit(primary, logoSurface, NULL, screenWidth - 200, 0));

            /* set the timer to 3 seconds */
            if (OsdInfo.timerSetVolume == 0)
            {
                memset(&timerSpecVolume, 0, sizeof(timerSpecVolume));
                timerSpecVolume.it_value.tv_sec = 3;
                timerSpecVolume.it_value.tv_nsec = 0;
                timer_settime(timerIdVolume, timerFlagsVolume, &timerSpecVolume, &timerSpecOldVolume);

                OsdInfo.timerSetVolume = 1;
            }
        }

        DFBCHECK(primary->Flip(primary, NULL, 0));
        usleep(100000);
    }
}

OsdGraphicsError OsdDeinit(void)
{	
	/* Thread join */
    threadExit = 1;
    if (pthread_join(osdThread, NULL))
    {
        printf("\n%s : Error! Pthread join failed!\n", __FUNCTION__);
        return OSD_ERROR;
    }
	
	/* Release allocated DirectFB memory */
    primary->Release(primary);
    dfbInterface->Release(dfbInterface);

    return OSD_NO_ERROR;
}

OsdGraphicsInfo* getOsdInfo(void)
{
    return &OsdInfo;
}

void clearScreenProgram(void)
{
    /* Reset the timer and clear the info banner from the screen */
    OsdInfo.draw = 0;
    OsdInfo.timerSetProgram = 0;
}

void clearScreenVolume(void)
{
    /* Reset the timer and clear the volume logo from the screen */
    OsdInfo.drawVolume = 0;
    OsdInfo.timerSetVolume = 0;
}
