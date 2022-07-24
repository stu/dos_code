/**/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <ctype.h>

bool have_gold;
bool have_gus;
bool have_sb;
bool have_wss_ultra16;

bool have_card;


char* unspace(char *s)
{
	while(*s != 0 && isspace(*s) != 0)
		s++;

	return s;
}

char* dos_env_skip_uint16(char *p)
{
	p = unspace(p);

	while(*p != 0 && isxdigit(*p) != 0)
		p++;

	p = unspace(p);

	return p;
}

char* dos_env_skip_int(char *p)
{
	p = unspace(p);

	while(*p != 0 && isdigit(*p) != 0)
		p++;

	p = unspace(p);

	return p;
}

uint16_t dos_env_parse_uint16(char *p)
{
	uint16_t x = 0;

	p = unspace(p);

	while(*p != 0x0 && isxdigit(*p) != 0)
	{
		x <<= 4;
		if(isdigit(*p) != 0)
			x += *p - 0x30;
		else if(*p >= 'A' && *p <= 'F')
			x += (*p - 'A') + 0x0A;
		else if(*p >= 'a' && *p <= 'f')
			x += (*p - 'a') + 0x0A;

		p++;
	}

	return x;
}

uint16_t dos_env_parse_int(char *p)
{
	uint16_t x = 0;

	p = unspace(p);

	while(*p != 0x0 && isdigit(*p) != 0)
	{
		if(isdigit(*p) != 0)
		{
			x *= 10;
			x += *p - 0x30;
		}

		p++;
	}

	return x;
}

char* lcase_string(char *f)
{
	char *p;

	if(f == NULL)
		return NULL;

	p = f;
	while(*p != 0)
	{
		if(*p >= 'A' && *p <= 'Z')
			*p |= 0x20;

		p++;
	}

	return f;
}

static void DecodeWSS_Ultra16(void)
{
	char *p;

	uint16_t port;
	int irq;
	int dma;
	int type;

	port = 0;
	irq = 0;
	dma = 0;
	type = 0;

	p = getenv("ULTRA16");
	if(p == NULL) return;

	have_wss_ultra16 = true;
	have_card = true;

	p = lcase_string(p);
	port = dos_env_parse_uint16(p);
	p = dos_env_skip_uint16(p);
	if(*p == ',') p++;

	if(*p != 0)
	{
		dma = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	if(*p != 0)
	{
		irq = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	if(*p != 0)
	{
		type = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	printf("\nWindows Sound System\n");
	printf("\tULTRA16=%s\n", getenv("ULTRA16"));
	printf("\tPort %x\n", port);
	printf("\tDMA %i (%s)\n", dma, dma<5?"Low 8bit":"High 16bit");
	printf("\tIRQ %i\n", irq);
}

static void DecodeGold(void)
{
	char *p;

	uint16_t port;

	port = 0;

	p = getenv("GOLD");
	if(p == NULL) return;

	have_gold = true;
	have_card = true;

	port = dos_env_parse_uint16(p);

	printf("\nAdlib Gold 1000\n");
	printf("\tGOLD=%s\n", getenv("GOLD"));
	printf("\tPort %x\n", port);
	//printf("\tPlayback DMA %i (%s)\n", playback_dma, playback_dma<5?"Low 8bit":"High 16bit");
	//printf("\tPlayback IRQ %i\n", playback_irq);
	//printf("\tRecord DMA %i (%s)\n", record_dma, record_dma<5?"Low 8bit":"High 16bit");
	//printf("\tSoundblaster MIDI IRQ %i\n", sb_midi_irq);
}


static void DecodeGUS(void)
{
	char *p;

	uint16_t port;
	int playback_dma;
	int record_dma;
	int playback_irq;
	int sb_midi_irq;

	port = 0;
	playback_dma = 0;
	playback_irq = 0;
	record_dma = 0;
	sb_midi_irq = 0;

	p = getenv("ULTRASND");
	if(p == NULL) return;

	have_gus = true;
	have_card = true;

	port = dos_env_parse_uint16(p);
	p = dos_env_skip_uint16(p);
	if(*p == ',') p++;

	if(*p != 0)
	{
		playback_dma = dos_env_parse_uint16(p);
		p = dos_env_skip_uint16(p);
		if(*p == ',') p++;
	}

	if(*p != 0)
	{
		record_dma = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	if(*p != 0)
	{
		playback_irq = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	if(*p != 0)
	{
		sb_midi_irq = dos_env_parse_int(p);
		p = dos_env_skip_int(p);
		if(*p == ',') p++;
	}

	printf("\nGravis Ultrasound\n");
	printf("\tULTRASND=%s\n", getenv("ULTRASND"));
	printf("\tPort %x\n", port);
	printf("\tPlayback DMA %i (%s)\n", playback_dma, playback_dma<5?"Low 8bit":"High 16bit");
	printf("\tPlayback IRQ %i\n", playback_irq);
	printf("\tRecord DMA %i (%s)\n", record_dma, record_dma<5?"Low 8bit":"High 16bit");
	printf("\tSoundblaster MIDI IRQ %i\n", sb_midi_irq);
}

static void DecodeSB(void)
{
	char *p;

	uint16_t base_address;
	int irq;
	int low_8bit_dma;
	int high_16bit_dma;
	int mpu_address;
	int emu_address;
	int type;

	irq = 0;
	low_8bit_dma = 0;
	high_16bit_dma = 0;
	mpu_address = 0;
	emu_address = 0;
	type = 0;
	base_address = 0;

	p = getenv("BLASTER");
	if(p == NULL) return;

	have_sb = true;
	have_card = true;

	p = lcase_string(p);

	while(*p != 0)
	{
		p = unspace(p);

		switch(*p)
		{
			case 'a':
				p = unspace(p+1);
				base_address = dos_env_parse_uint16(p);
				p = dos_env_skip_uint16(p);
				break;

			case 'i':
				p = unspace(p + 1);
				irq = dos_env_parse_int(p);
				p = dos_env_skip_int(p);
				break;

			case 'd':
				p = unspace(p + 1);
				low_8bit_dma = dos_env_parse_int(p);
				p = dos_env_skip_int(p);
				break;

			case 'h':
				p = unspace(p + 1);
				high_16bit_dma = dos_env_parse_int(p);
				p = dos_env_skip_int(p);
				break;

			case 'p':
				p = unspace(p + 1);
				mpu_address = dos_env_parse_uint16(p);
				p = dos_env_skip_uint16(p);
				break;

			case 'e':
				p = unspace(p + 1);
				emu_address = dos_env_parse_uint16(p);
				p = dos_env_skip_uint16(p);
				break;

			case 't':
				p = unspace(p + 1);
				type = dos_env_parse_int(p);
				p = dos_env_skip_int(p);
				break;

				// unknown junk in the blaster environment
			default:
				break;
		}
	}

	switch(type)
	{
		default:
		case 0:
		case 1:
			printf("\nSound Blaster\n");
			break;
		case 2:
			printf("\nSound Blaster Pro v1 / Sound Blaster 1.5\n");
			break;
		case 3:
			printf("\nSound Blaster 2.0\n");
			break;
		case 4:
			printf("\nSound Blaster Pro\n");
			break;
		case 5:
			printf("\nSound Blaster Pro 2.0\n");
			break;
		case 6:
			if(emu_address != 0)
				printf("\nSound Blaster AWE32/AWE64\n");
			else
				printf("\nSound Blaster 16\n");
			break;
	}

	printf("\tBLASTER=%s\n", getenv("BLASTER"));
	printf("\tPort %x\n", base_address);
	printf("\tLow 8bit DMA %i\n", low_8bit_dma);
	if(high_16bit_dma != 0)
		printf("\tHigh 16bit DMA %i\n", high_16bit_dma);
	printf("\tIRQ %i\n", irq);
	if(emu_address != 0)
		printf("\tWavetable Port %x\n", emu_address);
	if(mpu_address != 0)
		printf("\tMIDI Port %x\n", mpu_address);
}


int main(int argc, char *argv[])
{
	have_gold = false;
	have_gus = false;
	have_sb = false;
	have_wss_ultra16 = false;
	have_card = false;

	printf("Sound Card Info from Environment - Dark Fiber [NuKE]\n");

	DecodeGold();
	DecodeGUS();
	DecodeSB();
	DecodeWSS_Ultra16();

	if(have_card == false)
	{
		printf("Did not find Gravis Ultrasound or Sound Blaster environment string\n");
	}

	return 0;
}
