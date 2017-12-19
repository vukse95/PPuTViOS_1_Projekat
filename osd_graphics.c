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

static IDirectFBSurface* primary = NULL;
static IDirectFB* dfbInterface = NULL;
static DFBSurfaceDescription surfaceDesc;
static int screenWidth;
static int screenHeight;

static IDirectFBImageProvider* provider = NULL;
static IDirectFBSurface* logoSurface = NULL;
static int32_t logoWidth;
static int32_t logoHeight;

static struct itimerspec timerSpec;
static struct itimerspec timerSpecOld;
int32_t timerFlags;
timer_t timerId;

static pthread_t gcThread;
static int32_t threadExit;

static OsdGraphicsInfo OsdInfo;

static void* graphicControllerTask(void* params);
static void timerInit(void);
static void clearScreen(void);

OsdGraphicsError OsdInit(void) 
{
    if (pthread_create(&gcThread, NULL, &graphicControllerTask, NULL))
    {
        printf("Error creating OSD thread\n");
        return OSD_ERROR;
    }

    return OSD_NO_ERROR;
}

void timerInit(void)
{
    struct sigevent signalEvent;

    /* set sigevent callback */
    signalEvent.sigev_notify = SIGEV_THREAD;
    signalEvent.sigev_notify_function = (void*)clearScreen;
    signalEvent.sigev_value.sival_ptr = NULL;
    signalEvent.sigev_notify_attributes = NULL;
    timer_create(CLOCK_REALTIME, &signalEvent, &timerId);
}

void* graphicControllerTask(void* params)
{
    IDirectFBFont* fontInterface = NULL;
    DFBFontDescription fontDesc;
    char channelString[4];
    char audioPidString[50];
    char videoPidString[50];

    /* initialize timer */
    timerInit();

    /* initialize directFB */
    DFBCHECK(DirectFBInit(NULL, NULL));

    /* fetch the directFB interface */
    DFBCHECK(DirectFBCreate(&dfbInterface));

    /* set full screen */
    DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));

    /* create primary surface with double buffering */
    surfaceDesc.flags = DSDESC_CAPS;
    surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary);

    /* fetch the screen size */
    DFBCHECK(primary->GetSize(primary, &screenWidth, &screenHeight));

    while (threadExit == 0)
    {
        if (OsdInfo.draw == 1)
        {
            OsdInfo.draw = 0;

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

            /* draw channel rectangle */
            DFBCHECK(primary->SetColor(primary, 0x03, 0x03, 0xff, 0xff));
            DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth / 10, screenHeight / 8));

            DFBCHECK(primary->SetColor(primary, 0x03, 0xff, 0x03, 0xff));
            DFBCHECK(primary->FillRectangle(primary, 10, 10, screenWidth / 10 - 20, screenHeight / 8 - 20));

            /* draw channel number */
            fontDesc.flags = DFDESC_HEIGHT;
            fontDesc.height = 48;
            sprintf(channelString, "%d", OsdInfo.channelNumber);
            DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
            DFBCHECK(primary->SetColor(primary, 0xff, 0x03, 0x03, 0xff));
            DFBCHECK(primary->SetFont(primary, fontInterface));
            DFBCHECK(primary->DrawString(primary, channelString, -1, screenWidth / 10 - 110, screenHeight / 8 - 50, DSTF_LEFT));

            /* draw info rectangle */
            DFBCHECK(primary->SetColor(primary, 0x03, 0x03, 0xff, 0xff));
            DFBCHECK(primary->FillRectangle(primary, screenWidth / 2 - 500, screenHeight * 6 / 8, 1000, screenHeight / 8));

            /* draw audio and video pid */
            sprintf(audioPidString, "Audio PID: %d", OsdInfo.audioPid);
            sprintf(videoPidString, "Video PID: %d", OsdInfo.videoPid);
            fontDesc.height = 28;
            DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
            DFBCHECK(primary->SetColor(primary, 0xff, 0x03, 0x03, 0xff));
            DFBCHECK(primary->DrawString(primary, audioPidString, -1, screenWidth / 2 - 470, screenHeight * 6 / 8 + 50, DSTF_LEFT));
            DFBCHECK(primary->DrawString(primary, videoPidString, -1, screenWidth / 2 - 470, screenHeight * 6 / 8 + 100, DSTF_LEFT));            

            /* set the timer to 5 seconds */
            memset(&timerSpec, 0, sizeof(timerSpec));
            timerSpec.it_value.tv_sec = 5;
            timerSpec.it_value.tv_nsec = 0;
            timer_settime(timerId, timerFlags, &timerSpec, &timerSpecOld);

            /* flip the buffers */
            DFBCHECK(primary->Flip(primary, NULL, 0));
        }

        if (OsdInfo.drawVolume == 1)
        {
            char volumePicture[50];

            OsdInfo.drawVolume = 0;

            /* blank screen */
            primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00);
            primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight);

            /* set the picture file */
            sprintf(volumePicture, "volume_%d.png", OsdInfo.volume);
            printf("%s\n", volumePicture);

            /* create image provider */
            DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, volumePicture, &provider));
            DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
            DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &logoSurface));
            DFBCHECK(provider->RenderTo(provider, logoSurface, NULL));

            provider->Release(provider);

            /* fetch the logo size and blit it to the screen */
            DFBCHECK(logoSurface->GetSize(logoSurface, &logoWidth, &logoHeight));
            DFBCHECK(primary->Blit(primary, logoSurface, NULL, screenWidth - 200, 0));

            /* set the timer to 5 seconds */
            memset(&timerSpec, 0, sizeof(timerSpec));
            timerSpec.it_value.tv_sec = 5;
            timerSpec.it_value.tv_nsec = 0;
            timer_settime(timerId, timerFlags, &timerSpec, &timerSpecOld);

            /* flip the buffers */
            DFBCHECK(primary->Flip(primary, NULL, 0));
        }
		usleep(100000);
    }
}

OsdGraphicsError OsdDeinit(void)
{
    threadExit = 1;
    if (pthread_join(gcThread, NULL))
    {
        printf("\n%s : Error! Pthread join failed!\n", __FUNCTION__);
        return OSD_ERROR;
    }
    primary->Release(primary);
    dfbInterface->Release(dfbInterface);

    return OSD_NO_ERROR;
}

OsdGraphicsInfo* getOsdInfo(void)
{
    return &OsdInfo;
}

void clearScreen(void)
{
    /* clear the screen callback */
    if (OsdInfo.drawBlack == 0)
    {
        primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00);
        primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight);
    }
    else
    {
        primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff);
        primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight);
    }
    primary->Flip(primary, NULL, 0);
}
