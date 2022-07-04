/*	Purpose: Read the DRAM config registers on VT82C496G chipset
	Keywords: DOS, Watcom, VT82C496G, DRAM, RealMode
	Changelog:
	20160130 (sgeorge) - Initial release
*/

#define APP_RELEASE "20160130"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <i86.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#define INDEX_PORT		0xA8
#define DATA_PORT		0xA9

#define DRAM_PAIR_01	0x43
#define DRAM_PAIR_23	0x44

/* globals */
char *gDRAMSize[] = { "512kb", "1mb", "2mb", "4mb", "8mb", "16mb", "32mb", "64mb" };
uint32_t gRAMSize[] = { 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

uint32_t g_RamCount;

static void PrintRamBank(int pair, uint8_t dram_info, uint8_t dram_bank_count)
{
	if(!(dram_bank_count == 0 && dram_info == 0))
	{
		printf("Bank %i:0 - %s\n", pair, gDRAMSize[dram_info]);
		g_RamCount += gRAMSize[dram_info];
	}
	else
	{
		printf("Bank %i:0 - Empty\n", pair);
	}

	if(dram_bank_count == 0)
	{
		printf("Bank %i:1 - Empty\n", pair);
	}
	else
	{
		printf("Bank %i:1 - %s\n", pair, gDRAMSize[dram_info]);
		g_RamCount += gRAMSize[dram_info];
	}
}

static int TestBIOS(void)
{
	int rc = 0;
	unsigned char far *b;
	char *s = "VT49";
	int len;
	int count;

	len = strlen(s);

	for(count=0; count < 0xfff0; b++, count++)
	{
		b = (unsigned char far*)MK_FP(0xF000, count);

		if(_fmemcmp(b, s, len) == 0)
		{
			int i;

			printf("Found: 0x%04X:%04X :: ", FP_SEG(b), FP_OFF(b));

			for(i=0; b[i] >=0x20 && b[i]<=0x7F && i < 30; i++)
			{
				printf("%c", b[i]);
			}
			printf("\n");
			rc=1;
		}
	}


	return rc;
}

int main(int argc, char *argv[])
{
	uint8_t dram_result;
	uint8_t dram_info;
	uint8_t dram_bank_count;

	printf("Stu's VT82C496G RAM checker - r" APP_RELEASE "\n");

	if(TestBIOS() == 0)
	{
		printf("No VT496G chipset found via BIOS string\n");
		exit(1);
	}

	g_RamCount = 0;

	/*
	Bitfields for Via VT82C496G pair 0/1 DRAM size and configuration:
	Bit(s)	Description (Table P0432)
	 7-5	(VT82C496G) bank-pair 0 DRAM size (x2 if double bank)
		000 = 512 KB
		001 = 1 MB
		010 = 2 MB
		011 = 4 MB
		100 = 8 MB
		101 = 16 MB
		110 = 32 MB
		111 = 64 MB
	 4	number of banks in pair 0
		(0 bank if register 20h bit 7-5 = 0)
		0 = 1 bank
		1 = 2 banks
	 3-1	(VT82C496G) bank-pair 1 DRAM size (x2 if double bank)
	 0	number of banks in pair 1
		(0 bank if register 20h bit 3-1 = 0)
		0 = 1 bank
		1 = 2 banks

	*/

	outp(INDEX_PORT, DRAM_PAIR_01);
	dram_result = inp(DATA_PORT);

	// bank pair 0
	dram_info = (dram_result >> 5) & 0x7;
	dram_bank_count = (dram_result & (1<<4));
	PrintRamBank(0, dram_info, dram_bank_count);

	// bank pair 1
	dram_info = (dram_result >> 1) & 0x7;
	dram_bank_count = (dram_result & (1<<0));
	PrintRamBank(1, dram_info, dram_bank_count);


	outp(INDEX_PORT, DRAM_PAIR_23);
	dram_result = inp(DATA_PORT);

	// bank pair 2
	dram_info = (dram_result >> 5) & 0x7;
	dram_bank_count = (dram_result & (1<<4));
	PrintRamBank(2, dram_info, dram_bank_count);

	// bank pair 3
	dram_info = (dram_result >> 1) & 0x7;
	dram_bank_count = (dram_result & (1<<0));
	PrintRamBank(3, dram_info, dram_bank_count);

	if(g_RamCount < 1024)
		printf("\nTotal ram count is %ikb\n", g_RamCount);
	else
		printf("\nTotal ram count is %imb\n", g_RamCount / 1024);

	return 0;
}
