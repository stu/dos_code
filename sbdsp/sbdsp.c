/*	Purpose: Dumps the SoundBlaster DSP version
	Keywords: DOS, Watcom, SoundBlaster, DSP
	Changelog:
	20160522 (sgeorge) - Initial release
*/

#define APP_RELEASE "20160522"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <i86.h>
#include <stdint.h>

#define DSP_RESET		0x06
#define DSP_READ			0x0A
#define DSP_WRITE		0x0C

#define DSP_STATUS		0x0E

#define DSP_VERSION		0xE1
#define DSP_COPYRIGHT	0xE3

uint16_t g_port;

static void reset_dsp(void)
{
	outp(g_port + DSP_RESET, 1);
	delay(1);
	outp(g_port + DSP_RESET, 0);

	while(inp(g_port + DSP_STATUS) < 0x80)
		;

	while(inp(g_port + DSP_READ) != 0xAA)
		;
}

static void write_dsp(uint8_t command)
{
	 while(inp(g_port + DSP_WRITE) >= 0x80)
		;

	outp(g_port + DSP_WRITE, command);
}

static uint8_t read_dsp(void)
{
	uint16_t c;
	while(inp(g_port + DSP_STATUS) < 0x80)
		;

	c = inp(g_port + DSP_READ);

	return c & 0xFF;
}


static char* unspace(char *s)
{
	while(*s != 0 && isspace(*s) != 0)
		s++;

	return s;
}

static int parse_sb_env(void)
{
	int rc = 0;
	char *e;
	char *p;

	g_port = 0;

	e = getenv("BLASTER");
	p = e;
	if(e != NULL)
	{
		while(*p != 0x0)
		{
			p = unspace(p);
			if(p != 0x0)
			{
				// look for 'A' or 'a')
				if(*p == 'A' || *p == 'a')
				{
					p = unspace(p+1);

					while(*p != 0x0 && isspace(*p) == 0)
					{
						unsigned char c = toupper(*p);

						if(c >= 'A')
							c -= 'A' + 10;
						else
							c -= '0';

						g_port <<= 4;
						g_port += c;

						p += 1;
					}

					return g_port;
				}
				else
				{
					while(*p != 0x0 && isspace(*p) == 0)
						p++;
				}
			}
		}
	}

	return rc;
}

int main(int argc, char *argv[])
{
	int rc = 0;
	int rettype = 0;
	int quiet = 0;
	int i;

	for(i=1; i < argc; i++)
	{
		if(strcmp(argv[i], "-?") == 0)
		{
			printf("Stu's SoundBlaster DSP checker - r" APP_RELEASE "\n\n");
			printf("--major    Return DSP major version (DEFAULT)\n");
			printf("--minor    Return DSP minor version\n");
			printf("-q         Quiet (no printing anything)\n");
			printf("\n-?         This help\n");
			printf("\n\nA return value of 0 indicates an error\n");
			exit(0);
		}
		else if(strcmp(argv[i], "-q") == 0)
		{
			quiet = 1;
		}
		else if(strcmp(argv[i], "--major") == 0)
		{
			rettype = 0;
		}
		else if(strcmp(argv[i], "--minor") == 0)
		{
			rettype = 1;
		}
	}

	if(quiet == 0)
		printf("Stu's SoundBlaster DSP checker - r" APP_RELEASE "\n\n");

	if( parse_sb_env() > 0)
	{
		uint16_t dsp;

		reset_dsp();

		write_dsp(DSP_VERSION);
		dsp = (read_dsp() << 8) + read_dsp();

		if(quiet == 0)
			printf("DSP Version = %i.%i\n", dsp >> 8, dsp & 0xFF);

		if(rettype == 0)
			rc = dsp >> 8;
		else
			rc = dsp & 0xFF;

		/* do not show DSP copyright if not version 4! */
		if(quiet == 0 && dsp >= 0x400 && dsp <= 0x7FF )
		{
			uint8_t c;
			uint8_t buff[256];

			i = 0;
			memset(buff, 0x0, 256);

			write_dsp(DSP_COPYRIGHT);
			c = 1;
			while(i < 256 && c != 0)
			{
				c = read_dsp();
				buff[i++] = c;
			}

			printf("DSP Copyright = %s\n", buff);
		}
	}
	else
	{
		if(quiet == 0)
			printf("Could not find BLASTER environment variable.\n");

		rc = 0;
	}

	return rc;
}
