#include <windows.h>
#include <stdatomic.h>
#include "as.h"

#define MAX_BUFFERS 2 /*Must be power of 2*/
static struct _State
{
	HWAVEOUT device;
	HANDLE sent;
	WAVEHDR buf[MAX_BUFFERS];
	uint32_t buf_index;
	uint32_t mixing;
	uint8_t *mix_buf;
	HANDLE mix_thread;
	As_mix_func mix_func;
}_s;

/*Calls user code to fill buffer objects*/
static DWORD WINAPI
mix_main (LPVOID param)
{
	while (_s.mixing)
	{	/*Queue up the buffer*/
		WAVEHDR *buf = _s.buf + _s.buf_index;
		_s.mix_func ((uint8_t *)buf->lpData, (uint32_t)buf->dwBufferLength);
		waveOutWrite (_s.device, buf, sizeof (*buf));
		/*Advance to the next buffer*/
		_s.buf_index = (_s.buf_index + 1)&(MAX_BUFFERS - 1);
		/*Wait for the buffer to complete before filling another*/
		WaitForSingleObject (_s.sent, INFINITE);
	}
	return 0;
}
/*Notifies the mixer thread that a buffer has been sent to the device*/
static void CALLBACK
notify (HWAVEOUT device, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
	if (msg != WOM_DONE)
	{
		return;
	}
	ReleaseSemaphore (_s.sent, 1, NULL);
}

int
device_init (As_config *cfg)
{
	WAVEFORMATEX fmt;
	/*Clear out the global state*/
	memset (&_s, 0, sizeof (_s));
	/*Create synch event*/
	_s.sent = CreateSemaphore (NULL, MAX_BUFFERS - 1, MAX_BUFFERS, NULL);
	if (NULL == _s.sent)
	{
		return -1;
	}
	/*Install mix function*/
	_s.mix_func = cfg->mix;
	/*Describe the output format*/
	memset (&fmt, 0, sizeof (fmt));
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = cfg->channels;
	fmt.nSamplesPerSec = cfg->freq;
	fmt.wBitsPerSample = cfg->bits;
	fmt.nBlockAlign = fmt.nChannels*fmt.wBitsPerSample/8;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec*fmt.nBlockAlign;
	/*Create the device*/
	if(waveOutOpen (&_s.device, WAVE_MAPPER, &fmt,
	   (DWORD_PTR)notify, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		return -2;
	}
	/*Allocate mix buffer*/
	size_t size = cfg->samples*fmt.nChannels*fmt.wBitsPerSample/8;
	_s.mix_buf = malloc (MAX_BUFFERS*size);
	if (NULL == _s.mix_buf)
	{
		waveOutClose (_s.device);
		return -3;
	}
	/*Initialise the buffer objects*/
	for (uint32_t i = 0; i < MAX_BUFFERS; i++)
	{
		WAVEHDR *buf = _s.buf + i;
		memset (buf, 0, sizeof (*buf));
		buf->lpData = (LPSTR)(_s.mix_buf + i*size);
		buf->dwBufferLength = size;
		waveOutPrepareHeader (_s.device, buf, sizeof (*buf));
	}
	/*Spin up mix thread*/
	_s.mixing = 1;
	_s.buf_index = 0;
	_s.mix_thread = CreateThread (
		NULL, 0, (LPTHREAD_START_ROUTINE)mix_main, NULL, 0, NULL);
	if (NULL == _s.mix_thread)
	{
		waveOutClose (_s.device);
		free (_s.mix_buf);
		return -4;
	}
	SetThreadPriority (_s.mix_thread, THREAD_PRIORITY_TIME_CRITICAL);
	return 0;
}
void
device_shutdown (void)
{	/*Free buffers, and close the device*/
	waveOutReset (_s.device);
	for (uint32_t i = 0; i < MAX_BUFFERS; i++)
	{
		waveOutUnprepareHeader (_s.device, _s.buf + i, sizeof (_s.buf[0]));
	}
	waveOutClose (_s.device);
	/*Free mix buffer*/
	free (_s.mix_buf);
	/*Destroy event and thread*/
	CloseHandle (_s.mix_thread);
	CloseHandle (_s.sent);
}
