#pragma once
#include <stdint.h>

typedef void (*As_mix_func) (uint8_t *buf, uint32_t size);

typedef struct _As_config
{
	As_mix_func mix;	/*Call on each buffer*/
	uint32_t channels;	/*Number of audio channels*/
	uint32_t samples;	/*Samplers per buffer*/
	uint32_t freq;		/*Samples per second*/
	uint32_t bits;		/*Bits per sample*/
}As_config;

int device_init (As_config *cfg);
void device_shutdown (void);
