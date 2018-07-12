#include <SDL.h>

//#ifdef GP2X
#       include <sys/mman.h>
#       include <sys/stat.h>
#       include <fcntl.h>
#       include <sys/ioctl.h>
#       include <sys/soundcard.h>
#       include <sys/time.h>
#       include <sys/resource.h>
#       include <sys/types.h>
#       include <unistd.h>
#       include <string.h>
//#endif

#include <stdlib.h>

#include "beebconfig.h"
#include "beebconfig_data.h"
#include "video.h"

#ifdef USE_DMA
#include "dma.h"
#endif


#ifdef GP2X
/* GP2x MMSP2 registers:
 */
//volatile unsigned long  *MMSP2_16;
//volatile unsigned short *MMSP2_32;




volatile Uint8 *gp2x_sound_buffer_p=NULL;
static volatile Uint16 *gp2x_memregs=NULL;
static volatile Uint32 *gp2x_memregl=NULL;
static int gp2x_dev_mem=NULL;

static void GP2x_InitializeSoundBuffer(void);
#endif

/* Joystick for GP2X:
 */
#ifdef GP2X
      SDL_Joystick *joystick_p = NULL;
#endif

/* Frame buffer:
 */
SDL_Surface *frame_buffer_p=NULL;
SDL_Surface *rgb_surface=NULL;

/* Sound support:
 */
#define SOUND_BUFFER_SIZE (1024*64)
Uint8 SDLSoundBuffer[SOUND_BUFFER_SIZE];

unsigned long SDLSoundBufferOffset_IN;
unsigned long SDLSoundBufferOffset_OUT;

unsigned long SDLSoundBufferBytesHave;

/* This is a hack for now, the GP2x will say
 * that 512 samples is OK, then take 1024 via the
 * callback.
 *
 * I'll investigate this later.
 */

//#define REQUESTED_NUMBER_OF_SAMPLES 8192
//#define REQUESTED_NUMBER_OF_SAMPLES 4096
//#define REQUESTED_NUMBER_OF_SAMPLES 2048
//#define REQUESTED_NUMBER_OF_SAMPLES 256
//#define REQUESTED_NUMBER_OF_SAMPLES 128

#ifndef GP2x
#	define REQUESTED_NUMBER_OF_SAMPLES 1024
//#	define REQUESTED_NUMBER_OF_SAMPLES 512
//#	define REQUESTED_NUMBER_OF_SAMPLES 256
#else
#	define REQUESTED_NUMBER_OF_SAMPLES 512
#endif

int samples;

SDL_AudioSpec wanted;

void InitializeSoundBuffer(void)
{
        SDLSoundBufferOffset_IN =0;
        SDLSoundBufferOffset_OUT = 0;
        SDLSoundBufferBytesHave = 0;

        int i;
        for(i=0; i< SOUND_BUFFER_SIZE; i++)
                SDLSoundBuffer[i] = (Uint8) 0;
}

void FlushSoundBuffer(void)
{
        InitializeSoundBuffer();
}

/*
void AddBytesToSDLSoundBuffer(void *p, int len)
{
	//REQUESTED_NUMBER_OF_SAMPLES
	int i;
	Uint8 *pp = (Uint8*) p;

	for(i=0; i<len; i++)
		SDLSoundBuffer[i] = *(pp++);
}
*/

static int buffer_sound=0;

/*
void AddBytesToSDLSoundBuffer(void *p, int len)
{
        int i;
        Uint8 *pp;
        pp = (Uint8*) p;


//        printf("ADDED %d BYTES\n %d %d %d", len, (int) pp[0], (int) pp[1], (int) pp[2]);

        for(i=0; i<len; i++){
                SDLSoundBuffer[SDLSoundBufferOffset_IN] = *(pp++);
                SDLSoundBufferOffset_IN++;
                if (SDLSoundBufferOffset_IN >= SOUND_BUFFER_SIZE)
                        SDLSoundBufferOffset_IN = (unsigned long) 0;
        }
        SDLSoundBufferBytesHave+=len;

	if (SDLSoundBufferBytesHave > SOUND_BUFFER_SIZE) {
		printf("*** SOUND BUFFER OVERRUN ***\n");

		if (SDLSoundBufferOffset_IN >= REQUESTED_NUMBER_OF_SAMPLES*10)
			SDLSoundBufferOffset_OUT = SDLSoundBufferOffset_IN - REQUESTED_NUMBER_OF_SAMPLES*10;
		else
			SDLSoundBufferOffset_OUT = SOUND_BUFFER_SIZE-1 - REQUESTED_NUMBER_OF_SAMPLES*10 + SDLSoundBufferOffset_IN;

		SDLSoundBufferBytesHave = REQUESTED_NUMBER_OF_SAMPLES*10;		
	}

	// Only start sound once we have enough samples buffered.
	if (SDLSoundBufferBytesHave >= (REQUESTED_NUMBER_OF_SAMPLES*3) && buffer_sound == 0) {
		printf("Now have %d, starting sound\n", SDLSoundBufferBytesHave);
		 SDL_PauseAudio(0);
		buffer_sound=1;
	}
}
*/
void AddBytesToSDLSoundBuffer(void *p, int len)
{
        int i;
        Uint8 *pp;
        pp = (Uint8*) p;


//        printf("ADDED %d BYTES\n %d %d %d", len, (int) pp[0], (int) pp[1], (int) pp[2]);

        for(i=0; i<len; i++){
                SDLSoundBuffer[SDLSoundBufferOffset_IN] = *(pp++);
                SDLSoundBufferOffset_IN++;
                if (SDLSoundBufferOffset_IN >= SOUND_BUFFER_SIZE)
                        SDLSoundBufferOffset_IN = (unsigned long) 0;
        }
        SDLSoundBufferBytesHave+=len;

        if (SDLSoundBufferBytesHave > SOUND_BUFFER_SIZE) {
                //printf("*** SOUND BUFFER OVERRUN ***\n");

                if (SDLSoundBufferOffset_IN >= REQUESTED_NUMBER_OF_SAMPLES*10)
                        SDLSoundBufferOffset_OUT = SDLSoundBufferOffset_IN - REQUESTED_NUMBER_OF_SAMPLES*10;
                else
                        SDLSoundBufferOffset_OUT = SOUND_BUFFER_SIZE-1 - REQUESTED_NUMBER_OF_SAMPLES*10 + SDLSoundBufferOffset_IN;

                SDLSoundBufferBytesHave = REQUESTED_NUMBER_OF_SAMPLES*10;
        }

        // Only start sound once we have enough samples buffered.
//        if (SDLSoundBufferBytesHave >= (REQUESTED_NUMBER_OF_SAMPLES*3) && buffer_sound == 0) {
	if (SDLSoundBufferBytesHave >= (REQUESTED_NUMBER_OF_SAMPLES*2) && buffer_sound == 0) {
                //printf("Now have %d, starting sound\n", SDLSoundBufferBytesHave);
                 SDL_PauseAudio(0);
                buffer_sound=1;
        }
}


volatile Uint8 SequentialSDLSoundBuffer[10*1024]; // Shouldn't get this big!

volatile Uint8* GetSoundBufferPtr(void)
{
        return SequentialSDLSoundBuffer;
	//return gp2x_sound_buffer_p;
}

/*
static int CatchupSound(void)
{
	int i=0;

// was
//                        while ( SDLSoundBufferBytesHave > ( REQUESTED_NUMBER_OF_SAMPLES * 1)){
//			while ( SDLSoundBufferBytesHave > ( REQUESTED_NUMBER_OF_SAMPLES + (REQUESTED_NUMBER_OF_SAMPLES>>1) )) {
			while ( SDLSoundBufferBytesHave > ( REQUESTED_NUMBER_OF_SAMPLES * 2)){

				// Dump two out of the buffer per time.

				// 1

				SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;
				i++;

                                SDLSoundBufferOffset_OUT++;
                                if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
                                SDLSoundBufferBytesHave--;

				// 2

				SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;
				i++;

                                SDLSoundBufferOffset_OUT++;
                                if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
                                SDLSoundBufferBytesHave--;
                        }

	return i;
}
*/
void CatchupSound(void)
{
                        while (SDLSoundBufferBytesHave > ( REQUESTED_NUMBER_OF_SAMPLES * 2)){
                                SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;

                                SDLSoundBufferOffset_OUT++;
                                if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
                                SDLSoundBufferBytesHave--;
                        }
}


/*
int GetBytesFromSDLSoundBuffer(int len)
{
        static unsigned int uiCatchedUpTimes = 0;
        int i;
        volatile Uint8 *p;

//        if ( (unsigned long) len > SDLSoundBufferBytesHave)
//                len = SDLSoundBufferBytesHave;

        //p = SequentialSDLSoundBuffer;
	p = GetSoundBufferPtr();

	if (p == NULL) {printf("sound buffer is NULL\n"); return 0;}

        for(i=0; i<len; i++){
                *(p++) = SDLSoundBuffer[SDLSoundBufferOffset_OUT];
		SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;

                SDLSoundBufferOffset_OUT++;
                if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
        }

	SDLSoundBufferBytesHave-=len;

#ifdef GP2X
	if (SDLSoundBufferBytesHave > REQUESTED_NUMBER_OF_SAMPLES * 2){
#else
	if (SDLSoundBufferBytesHave > REQUESTED_NUMBER_OF_SAMPLES * 3){
#endif
		i=CatchupSound();
		fprintf(stderr, "Dumped %d sound samples, catched up %u times so far..\n"
		 , i // SDLSoundBufferBytesHave - ( REQUESTED_NUMBER_OF_SAMPLES * 2) 
		 , ++uiCatchedUpTimes);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////

//	i=0;
//	while ( SDLSoundBufferBytesHave > REQUESTED_NUMBER_OF_SAMPLES) { 
//		SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;
//		SDLSoundBufferOffset_OUT++;
//		if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
//			SDLSoundBufferOffset_OUT = (unsigned long) 0;
//
//		SDLSoundBufferBytesHave--;
//		i++;
//	}
//	if (i>0) printf("Dumped %d sound samples.\n", i);

///////////////////////////////////////////////////////////////////////////////////////////////////

        return len;
}




*/
int GetBytesFromSDLSoundBuffer(int len)
{
        static unsigned int uiCatchedUpTimes = 0;
        int i;
        volatile Uint8 *p;

//        if ( (unsigned long) len > SDLSoundBufferBytesHave)
//                len = SDLSoundBufferBytesHave;

        //p = SequentialSDLSoundBuffer;
        p = GetSoundBufferPtr();

        if (p == NULL) {printf("sound buffer is NULL\n"); return 0;}

        for(i=0; i<len; i++){
                *(p++) = SDLSoundBuffer[SDLSoundBufferOffset_OUT];
                //SDLSoundBuffer[SDLSoundBufferOffset_OUT] = 0;

                SDLSoundBufferOffset_OUT++;
                if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
        }

        SDLSoundBufferBytesHave-=len;


 //               if (SDLSoundBufferBytesHave > REQUESTED_NUMBER_OF_SAMPLES * 5){
		if (SDLSoundBufferBytesHave > REQUESTED_NUMBER_OF_SAMPLES * 4){
                        //fprintf(stderr, "Dumping %d sound samples, catched up %u times so far..\n"
                        // , SDLSoundBufferBytesHave - ( REQUESTED_NUMBER_OF_SAMPLES * 3), ++uiCatchedUpTimes);
                        CatchupSound();
                }

        return len;
}


unsigned long HowManyBytesLeftInSDLSoundBuffer(void)
{
        return SDLSoundBufferBytesHave;
}

/*
void fill_audio(void *udata, Uint8 *stream, int len)
{
	SDL_MixAudio(stream, SDLSoundBuffer , len, SDL_MIX_MAXVOLUME);
}
*/


/*
void fill_audio(void *udata, Uint8 *stream, int len)
{
        volatile Uint8 *p;

        void *tmp_udata;
        tmp_udata = udata;

        if (HowManyBytesLeftInSDLSoundBuffer() == 0)
                return;

	if ( HowManyBytesLeftInSDLSoundBuffer() < (unsigned int) len) { // (REQUESTED_NUMBER_OF_SAMPLES) ) {
		printf("not enough data have %d want %d.\n", SDLSoundBufferBytesHave, len);
		buffer_sound=0;
		return;
	}


//        if (len > (int) HowManyBytesLeftInSDLSoundBuffer() ) {
//                len = HowManyBytesLeftInSDLSoundBuffer();
//		printf("not enough data.\n");
//		return;
//	}

        p = GetSoundBufferPtr();
	if (p == NULL) {printf("sound buffer is NULL\n"); return;}

        len = GetBytesFromSDLSoundBuffer(len);
        SDL_MixAudio(stream, (const Uint8*) p, len, SDL_MIX_MAXVOLUME);
}
*/
void fill_audio(void *udata, Uint8 *stream, int len)
{
        volatile Uint8 *p;

        void *tmp_udata;
        tmp_udata = udata;

        if (HowManyBytesLeftInSDLSoundBuffer() == 0)
                return;

        if ( HowManyBytesLeftInSDLSoundBuffer() < (unsigned int) len) { // (REQUESTED_NUMBER_OF_SAMPLES) ) {
                //printf("not enough data have %d want %d.\n", SDLSoundBufferBytesHave, len);
                buffer_sound=0;
                return;
        }


//        if (len > (int) HowManyBytesLeftInSDLSoundBuffer() ) {
//                len = HowManyBytesLeftInSDLSoundBuffer();
//              printf("not enough data.\n");
//              return;
//      }

        p = GetSoundBufferPtr();
        if (p == NULL) {printf("sound buffer is NULL\n"); return;}

        len = GetBytesFromSDLSoundBuffer(len);
        SDL_MixAudio(stream, (const Uint8*) p, len, SDL_MIX_MAXVOLUME);
}


/* In main.cc
 */
void at_exit(void);

int InitializeSDL(int,char*[])
{
	int flags, ret;

	flags = SDL_INIT_VIDEO /* | SDL_INIT_AUDIO */ ;
#ifdef GP2X
	flags |= SDL_INIT_JOYSTICK;
#endif
	if (SDL_Init(flags) <0) {
                fprintf(stderr, "Unable to initialise SDL: %s\n"
                 , SDL_GetError());
                return 0;
        }


	/* atexit doesn't seem to work on the GP2X...
	 */
#ifndef GP2X
        atexit(at_exit);
#endif


        /* Setup frame buffer:
         */
        flags = SDL_HWSURFACE | SDL_HWPALETTE;
#ifndef GP2X
        //if ( (frame_buffer_p=SDL_SetVideoMode(640, 480, 8, flags)) == NULL ) {
        if ( (rgb_surface=SDL_SetVideoMode(320, 480, 16, flags)) == NULL ) {
#else
        if ( (frame_buffer_p=SDL_SetVideoMode(320, 240, 8, flags)) == NULL ) {
#endif
                fprintf(stderr, "Unable to set video mode: %s\n"
                 , SDL_GetError());
                return 0;
        }
	
	frame_buffer_p = SDL_CreateRGBSurface(SDL_SWSURFACE,
         320, 240, 8, 0, 0, 0, 0);
	if (frame_buffer_p == NULL)
	{
                fprintf(stderr, "Unable to create surface: %s\n"
                 , SDL_GetError());
                return 0;
	}		
	
#ifdef GP2X
        SDL_ShowCursor(SDL_DISABLE);
#endif

#ifdef GP2X
        if ( (joystick_p=SDL_JoystickOpen(0) ) == NULL) {
                fprintf(stderr, "Couldn't open the joystick: %s\n"
                 , SDL_GetError());
                return 0;
        }
#endif



#ifndef GP2X
	{
        /* Fill screen with some boxes to denote boundries.
         */
        SDL_FillRect(frame_buffer_p, NULL, SDL_MapRGB(frame_buffer_p->format
         , 0xff, 0x00, 0x00));
        SDL_Rect rect= {0, 0, 322, 258};
        Uint32 color = SDL_MapRGB(frame_buffer_p->format, 0xff, 0xff, 0x00);
        SDL_FillRect(frame_buffer_p, &rect, color);
        SDL_UpdateRect(frame_buffer_p, 0, 0, frame_buffer_p->w
         , frame_buffer_p->h);

        rect= (SDL_Rect) {0, 0, 322, 242};
        color = SDL_MapRGB(frame_buffer_p->format, 0x00, 0xff, 0xff);
        SDL_FillRect(frame_buffer_p, &rect, color);
        SDL_UpdateRect(frame_buffer_p, 0,0,frame_buffer_p->w,frame_buffer_p->h);
	}
#endif

#ifdef USE_DMA
	if(!init_dma())
		return 0;
#endif

	/* Successful:
	 */	
	return 1;
}


int InitializeSDLSound(int soundfrequency)
{
	SDL_AudioSpec *obtained_p;

        samples = REQUESTED_NUMBER_OF_SAMPLES;
	obtained_p = (SDL_AudioSpec*) malloc(sizeof(SDL_AudioSpec));

        SDLSoundBufferOffset_IN = 0;
        SDLSoundBufferOffset_OUT = 0;
        SDLSoundBufferBytesHave = 0;

        InitializeSoundBuffer();

#ifdef GP2X
	GP2x_InitializeSoundBuffer();
#endif

// This seems to be a big in Paeryn's SDL: AUDIO_U8 wants twice as much
// data as it should. So I've just set BeebEm to calc. sound at 44.1Khz
// and SDL can play it at 22.05Khz.
#ifdef GP2X
        wanted.freq = soundfrequency/2;
#else
	wanted.freq = soundfrequency;
#endif
  
        wanted.format = AUDIO_U8;
        wanted.channels = 1;
        wanted.samples = samples;                       //1024;
        wanted.callback = fill_audio;
        wanted.userdata = NULL;

        if ( SDL_OpenAudio(&wanted, obtained_p) < 0 ) {
                fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
//==                return(0);
        }

	printf("Sound:\n");
	printf("freq=%d\n", (int) obtained_p->freq);
	printf("format=%d\n", (int) obtained_p->format);
	printf("channels=%d\n", (int) obtained_p->channels);
	printf("silence=%d\n", (int) obtained_p->silence);
	printf("samples=%d\n", (int) obtained_p->samples);
	printf("size=%d\n------------\n\n", (int) obtained_p->samples);


//        SDL_PauseAudio(0);
//        SDL_Delay(250);

        return(1);
}

void FreeSDLSound(void)
{
        SDL_CloseAudio();
}




/* Sleeping:
 */

#define MIN_OS_WAIT_TIME	4

static void BusyWait(Uint32 u32TimeShouldWait, Uint32 u32StartTickCount)
{
        Uint32 u32AjustedTime;

        do{
                u32AjustedTime = SDL_GetTicks();

                /* Handle wrap around after ~47 days of continued execution
                 */
                if (u32AjustedTime < u32StartTickCount)
                        u32AjustedTime = u32AjustedTime
                         + ( ((Uint32) 0xffffffff) - u32StartTickCount);
                else
                        u32AjustedTime = u32AjustedTime - u32StartTickCount;

        }while(u32AjustedTime < u32TimeShouldWait);
}

void SDL_Sleep(unsigned int ticks)
{
        /* Do nothing if BeebEm asked to wait 0 ms.
         */

        if (ticks<1) return;

        // Only sleep if we are sure the OS can honnor it, busywait otherwise:
//        if(ticks >= MIN_OS_WAIT_TIME){
//#ifndef GP2X
//               SDL_Delay(ticks);
//        }else{
//#else
               BusyWait(ticks, SDL_GetTicks());
//#endif
//        }
}

#ifdef GP2X
/* GP2X:
 */


#define SYS_CLK_FREQ 7372800

//MMSP2_16


/* Volume, a value between 0 and 99:
 */
int volume = 0;

/* OSS Mixer file:
 */
int oss_mixer = -1;

void OSS_Mixer_Init(void) {
                oss_mixer=open("/dev/mixer", O_RDWR);
                if (oss_mixer == -1) {
                        printf("Can't open mixer.\n");
                }
}

void OSS_Mixer_Quit(void) {
	if (oss_mixer != -1)
		close(oss_mixer);
}

int OSS_Mixer_GetVolume(void) {
	if (oss_mixer == -1)
		return -1;
	else
		return volume;
}

void OSS_Mixer_SetVolume(int new_volume) {
        static int L;


	if (volume == new_volume) return;

	if (oss_mixer != -1) {
	        L=(((new_volume*0x50)/100)<<8)|((new_volume*0x50)/100);
        	ioctl(oss_mixer, SOUND_MIXER_WRITE_PCM, &L);
		volume = new_volume;
		printf("Volume changed to %d.\n", new_volume);
		config.settings.volume = new_volume;
	}
}

void GP2x_Initialize(void)
{
        if (gp2x_dev_mem == NULL) {
		gp2x_dev_mem = open("/dev/mem",   O_RDWR);
        	gp2x_memregl=(Uint32 *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev_mem, 0xc0000000);
        	gp2x_memregs=(Uint16*)gp2x_memregl;
	}
}

void GP2x_FreeResources(void)
{
	if (gp2x_dev_mem != NULL)
		close(gp2x_dev_mem);
}

static void GP2x_InitializeSoundBuffer(void)
{
	if (gp2x_sound_buffer_p == NULL) {
		if (gp2x_dev_mem == NULL) GP2x_Initialize();

		/* OK, now I'm writing over the secondary frame buffer.
		 */
		//gp2x_sound_buffer_p=(Uint8 *)mmap(0, 0x19000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev_mem, 0x02100000);

		// Secondary frame buffer
		gp2x_sound_buffer_p=(Uint8 *)mmap(0, 0x19000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev_mem, 0x03381000);

		// Primary frame buffer
		//gp2x_sound_buffer_p=(Uint8 *)mmap(0, 0x19000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev_mem, 0x03101000);
	}
}

// Value will change at 25HZ, same as the Beebem simulated CRT.
unsigned long GP2X_timer(void)
{
	if (gp2x_dev_mem == NULL) GP2x_Initialize();

	return gp2x_memregl[0x0A00>>2]; // /( (SYS_CLK_FREQ-1000)/25);
}


void GP2x_SetCPUSpeed(unsigned int MHZ)
{

	static unsigned int old_mhz = 0;

        unsigned v;
        unsigned mdiv,pdiv=3,scale=0;

	if (MHZ<100) MHZ=100;
	if (MHZ>260) MHZ=260;

	if (MHZ == old_mhz) return;

	if (gp2x_dev_mem == NULL) GP2x_Initialize();

	if (gp2x_memregs != NULL) {
		config.settings.speed = MHZ;
		old_mhz = MHZ;

        	MHZ*=1000000;
        	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
        	mdiv=((mdiv-8)<<8) & 0xff00;
        	pdiv=((pdiv-2)<<2) & 0xfc;
        	scale&=3;
        	v=mdiv | pdiv | scale;
        	gp2x_memregs[0x910>>1]=v;


	} else {
		printf("Unable to set GP2x 920T CPU speed - could not access MMSP2 registers.\n");
	}
}
#endif

