/*
 * mpg123 defines 
 * used source: musicout.h from mpegaudio package
 */
#ifndef MPG123_H
#define MPG123_H

struct APEG_STREAM;
struct APEG_LAYER;

typedef unsigned char byte;
extern int _apeg_ignore_audio;

int alvorbis_update(struct APEG_LAYER*);

int _apeg_audio_poll(struct APEG_LAYER*);
int _apeg_audio_close(struct APEG_LAYER*);
int _apeg_audio_flush(struct APEG_LAYER*);

int _apeg_audio_reset_parameters(struct APEG_LAYER*);
int _apeg_start_audio(struct APEG_LAYER*, int);


#include "mpeg1dec.h"

static INLINE int clamp_val(int low, int val, int high)
{
	val -= low;
	val &= (~val) >> 31;
	val += low;

	val -= high;
	val &= val >> 31;
	val += high;

	return val;
}

#endif	// MPG123_H
