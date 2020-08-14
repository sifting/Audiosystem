#include <string.h>
#include <stdio.h>
#include <as/as.h>

static uint32_t time = 0;

/*Paints a saw tooth wave to the sound buffer*/
static void
mixer (uint8_t *data, uint32_t size)
{
	int16_t *buf = (int16_t *)data;
	uint32_t samples = size/2;
	for (uint32_t i = 0; i < samples; i++)
	{
		*buf = (int16_t)8000.0*((time%1024)/1024.0);
		time++;
		buf++;
	}
}

int
main (int argc, char **argv)
{
	As_config cfg;
	
	memset (&cfg, 0, sizeof (cfg));
	cfg.mix = mixer;
	cfg.channels = 1;
	cfg.freq = 44100;
	cfg.samples = 4096;
	cfg.bits = 16;
	
	printf ("init device: %i\n", device_init (&cfg));
	fgetc (stdin);
	device_shutdown ();
	return 0;
}
