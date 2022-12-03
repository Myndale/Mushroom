// "Magic Mushroom" Demo Recreation - by Mark Feldman
// Written 5 June 2019...about 30 years after I first heard it!

#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <alloc.h>
#include <stdlib.h>
#include <mem.h>

const unsigned SPEAKER_PORT = 0x61;
const unsigned PIT_CONTROL = 0x43;
const unsigned PIT_TIMER_0 = 0x40;
const unsigned PIT_TIMER_2 = 0x42;


const unsigned long PIT_FREQ = 0x1234dd;
const unsigned PLAYBACK_FREQ = 9000;
const unsigned PIT_COUNT = PIT_FREQ / PLAYBACK_FREQ;

typedef void interrupt (far * intr_handler)(...);

volatile int finished = 0;
intr_handler old_handler;
volatile static unsigned long count = 0;
unsigned char speaker;
unsigned char next_sample;

unsigned char pit_lut[256];

unsigned long num_samples;
unsigned char huge * samples;

static void far interrupt timer_handler(...)
{
	// trigger one-shot timer
	outp(PIT_TIMER_2, next_sample);

	// get the next sample ready
	if (!finished)
	{
		next_sample = pit_lut[samples[count++]];
		finished = count >= num_samples;
	}

	// acknowledge the interrupt
	outp(0x20, 0x20);
}

void fread_huge(void huge * dest, long size, FILE * fp)
{
	unsigned char buffer[256];

	while (size >= sizeof(buffer))
	{
		fread(buffer, sizeof(buffer), 1, fp);
		memcpy(dest, buffer, sizeof(buffer));
		(unsigned char huge *)dest += sizeof(buffer);
		size -= sizeof(buffer);
	}
	if (size > 0)
	{
		fread(buffer, size, 1, fp);
		memcpy(dest, buffer, size);
	}
}

void main()
{
	// load the audio file (not the fastest way to do this, but it works)
	FILE * fp = fopen("MUSHROOM.OVL", "rb");
	if (!fp)
	{
		printf("Couldn't open MUSHROOM.OVL.");
		exit(0);
	}
	fseek(fp, 0, SEEK_END);
	num_samples = ftell(fp);
	samples = (unsigned char huge *)farmalloc(num_samples);
	if (!samples)
	{
		printf("Not enough memory.");
		getch();
		exit(0);
	}
	fseek(fp, 0, SEEK_SET);
	fread_huge(samples, num_samples, fp);
	fclose(fp);

	// set up pit one-shot lut
	for (int i=0; i<256; i++)
		pit_lut[i] = i * 54 / 256;

	// disable interrupts while we set up the timers and intr handler
	disable();

	// set timer 0 interrupt frequency
	outp(PIT_CONTROL, 0x34);
	outp(PIT_TIMER_0, (PIT_COUNT & 255));
	outp(PIT_TIMER_0, (PIT_COUNT >> 8));

	// set timer 2 for one-shot mode
	outp(PIT_CONTROL, 0x90);

	// connect speaker to pit timer 2
	outp(SPEAKER_PORT, (inp(SPEAKER_PORT) | 3));

	// install new intr handler
	old_handler = _dos_getvect(8);
	_dos_setvect(8, timer_handler);

	speaker = inp(SPEAKER_PORT) & 0xfc;

	// enable interrupts to start playing
	enable();

	// wait for user to press a key or for the playback to finish
	while (!kbhit() && !finished);

	// clean up
	disable();
	outp(SPEAKER_PORT, (inp(SPEAKER_PORT) & 0xfc));
	outp(PIT_CONTROL, 0xb6);
	outp(PIT_CONTROL, 0x34);
	outp(PIT_TIMER_0, 0);
	outp(PIT_TIMER_0, 0);
	_dos_setvect(8, old_handler);
	enable();
	farfree(samples);

	// clear the input buffer
	while (kbhit())
		getch();
}
