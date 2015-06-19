/*
 *  patest_sine_PanLR3.cpp
 *
 *  Created by Yuan-Yi Fan on 1/12/11.
 *
 */

#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#include <iostream>

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64) 

#define NUM_CH (2)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

int panValue;

using namespace std;

#define TABLE_SIZE   (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
	
	float ampL;
	float ampR;
}
paTestData;

static int patestCallback( const void *inputBuffer, void *outputBuffer,
						  unsigned long framesPerBuffer,
						  const PaStreamCallbackTimeInfo* timeInfo,
						  PaStreamCallbackFlags statusFlags,
						  void *userData ) 
{
    paTestData *data = (paTestData*)userData; 
    float *out = (float*)outputBuffer;
    unsigned long i; //
	
    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags; // don't get un use warning from compiler
    (void) inputBuffer;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
		switch(panValue){
			case -1: //left ch full volume
				data->ampL=1;
				data->ampR=0;
				*out++ = data->ampL * data->sine[data->left_phase];  /* left */ 
				*out++ = data->ampR * data->sine[data->right_phase];  /* right */ 
				break;
			case 0: //centered
				data->ampL=0.5;
				data->ampR=0.5;
				*out++ = data->ampL * data->sine[data->left_phase];  /* left */ 
				*out++ = data->ampR * data->sine[data->right_phase];  /* right */
				break;
			case 1: //right ch full volume
				data->ampL=0;
				data->ampR=1;
				*out++ = data->ampL * data->sine[data->left_phase];  /* left */ 
				*out++ = data->ampR * data->sine[data->right_phase];  /* right */
				break;	
		}
		
//        *out++ = data->amp * data->sine[data->left_phase];  /* left */ 
//        *out++ = data->amp * data->sine[data->right_phase];  /* right */
        data->left_phase += 3;
        if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
        data->right_phase += 5; /* higher pitch so we can distinguish left and right. */
        if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
    }
	
    return paContinue;
}

static void StreamFinished( void* userData )
{
	paTestData *data = (paTestData *) userData;
	printf( "Stream Completed: %s\n", data->message );
}

int main(void);
int main(void)
{
    cout<< "input -1 for left ch full volume, 0 for centered, and 1 for right ch full volume!" << endl;
	cin >> panValue;
	if (panValue > 1) {
		cout << "value out of range";
		goto error;
	}
	if (panValue < -1) {
		cout << "value out of range";
		goto error;	
	}
    PaStreamParameters outputParameters;
    PaStream *stream; //audio stream
    PaError err; //error code
    paTestData data; // holds the paTestData structure whose sine table we are initializing
    int i;
	
    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);
    
    /* initialise sinusoidal wavetable */
    for( i=0; i<TABLE_SIZE; i++ )
    {
        data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. ); // cast to double for precision
    }
	
    data.left_phase = data.right_phase = 0;//initialize phase to zero
	data.ampL = 0.5;
	data.ampR = 0.5;	
	
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
	
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL; //portaudio talks to native api for cross-platform, that's why host api is there, ex JACK or CoreAudio
	
    err = Pa_OpenStream(
						&stream, // address to output stream
						NULL, /* no input (stream) */
						&outputParameters, 
						SAMPLE_RATE,
						FRAMES_PER_BUFFER,
						paClipOff,      /* we won't output out of range samples so don't bother clipping them (paClipOff:automatically clipping) */
						patestCallback,
						&data ); // pass the address of sine wave table
    if( err != paNoError ) goto error;
	
    sprintf( data.message, "No Message" );
    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    if( err != paNoError ) goto error;
	
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
	
    printf("Play for %d seconds.\n", NUM_SECONDS );
    Pa_Sleep( NUM_SECONDS * 1000 );
	
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
	
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
	
    Pa_Terminate();
    printf("Test finished.\n");
    
    return err;
error: //label
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}


