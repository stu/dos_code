#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <i86.h>

#include "cpu.h"

#pragma pack(push, 1)
typedef struct udtE820_Entry
{
	uint32_t base[2];
	uint32_t length[2];
	uint32_t type;
} uE820_Entry;
#pragma pack(pop)

enum e_E820_type
{
	e820_Memory_Available = 1,
	e820_Reserved,
	e820_ACPI_Reclaim,
	e820_ACPI_NVS
};


#define MAX_E820_ENTRIES		64
static void bios_e820_get_memory_map(void)
{
	uXREGS r;
	uE820_Entry buff[MAX_E820_ENTRIES];
	uint32_t cont;
	int e;


	memset((uint8_t*)buff, 0x0, (MAX_E820_ENTRIES * sizeof(uE820_Entry)));

	cont = 0;
	e = 0;

	do
	{
		memset((uint8_t*)&r, 0x0, sizeof(uXREGS));
		r.x.eax = 0xE820;
		r.x.edx = 0x534D4150;
		r.x.ebx = cont;
		r.x.ecx = sizeof(uE820_Entry);
		r.x.es = FP_SEG(&buff[e]);
		r.x.edi = FP_OFF(&buff[e]);
		dosint(0x15, &r);

		cont = r.x.ebx;

		if(r.x.ecx < sizeof(uE820_Entry))
			break;
		if(r.x.ebx == 0)
			break;

		e += 1;
	}while(e < MAX_E820_ENTRIES && r.x.eax == 0x534D4150 && cont != 0 && (r.w.flags&X86_CARRY_FLAG) == 0);

	printf("\n");

	if(e == 0)
		printf("You BIOS does not appear to support E820 functionality.\n");
	else
	{
		printf("Start Address       Length              Type\n");
		printf("==================================================\n");
	}
	// calc out memory
	for(cont=0; cont < e; cont++)
	{
		printf("0x%08lX%08lX  0x%08lX%08lX  0x%08lX\n",
			buff[cont].base[1],buff[cont].base[0],
			buff[cont].length[1],buff[cont].length[0],
			buff[cont].type);
	}
}

int main(int argc, char *argv[])
{
	printf("E820 bios memory map list\n");
	if(DetectCPU() < 3)
		printf("You need an 80386 or better CPU to run this\n");
	else
		bios_e820_get_memory_map();

	return 0;
}

