// funktracker? used dos32

/*
   File format of a DOS32 executable (.EXE file produced from DLINK)

Offset	  Size			 Description
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
0		  n	   A stub loader or DOS32.EXE of n bytes  {no stub in DLL}
n+0		  4	   Signature "Adam"		{ ="DLL " for DLL files }
n+4		  2	   Minimum DOS32 version required (packed BCD)
n+6		  2	   DLINK version in packed BCD format
----if n+0Ch = 28h. Original DOS32-----
n+28h	  n	   Image  starts

----if n+4 < 350h. DOS32 version less than 3.50---
n+8		  4	   Number of bytes remaining after the stub file.
			   The entire .EXE file size (after linking) will be equal
			   to this value plus the stub file size.
n+0Ch	  4	   Start of 32bit executable image relative to the 'Adam'
			   signature.
n+10h	  4	   Length of executable image ( see below ).
n+14h	  4	   Initial memory required for application. This value
			   must be larger than the size of the 32bit executable
			   image.
n+18h	  4	   Initial EIP	{ public varible address for DLL file }
n+1Ch	  4	   Initial ESP	{ ignored in DLL header }
n+20h	  4	   Number of entries in fixup table. Each entry
			   in the table contains a 32bit offset relative to the
			   start of the executable image of a 16bit WORD that must
			   be set to the programs DATA selector at load time. The
			   table immediately follows the executable image.
n+24h	  4	   Flags  ( function active when bit is set )
				 bit 0	  = executable image is compressed
				 bit 1	  = display DOS32 logo when executed
				 bits 2..31 reserved.

----if n+4 = 350h. DOS32 version 3.50---
n+8		  4	   Length of executable image + fixups ( see below ).
n+0Ch	  4	   Number of bytes remaining after the stub file.
			   The entire .EXE file size (after linking) will be equal
			   to this value plus the stub file size.
n+10h	  4	   Start of 32bit executable image relative to the 'Adam'
			   signature.
n+14h	  4	   Initial EIP	{ public varible address for DLL file }
n+18h	  4	   Initial memory required for application. This value
			   must be larger than the size of the 32bit executable
			   image.
n+1Ch	  4	   Initial ESP	{ ignored in DLL header }
n+20h	  4	   Offset of DOS32 ver 3.50 fixup table. The table immediately
			   follows the executable image. Also can be treated as
			   a size of executable image.
n+24h	  1	   DOS32 copyright string screen attribute byte
n+25h	  1	   Something (?)
n+26h	  2	   Flags  ( function active when bit is set )
				 bit 0	  = executable image is compressed
				 bit 1	  = display DOS32 logo when executed
				 bits 2..31 reserved.
n+28h	  4	   Unknown (0)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "dos32.h"

void ConvertDOS32Header(FILE *fp, uint32_t stub_size, uDOS32Exec *h, uD32Exec *o)
{
	uint8_t xbuff[1024];
	int i;

	memset(o, 0x0, sizeof(uD32Exec));

	o->header_offset = stub_size;
	o->dos32_ver = h->dos32_min;
	o->dlink_ver = h->dlink_ver;

	if(h->dos32_min >= 0x350)
	{
		// 3.5
		o->eip = h->dos32.v35.eip;
		o->esp = h->dos32.v35.esp;
		if(o->esp == 0x6D616441)
		{
			o->esp = UNDEFINED_ESP;
		}

		o->original_flags = h->dos32.v35.flags;
		if((h->dos32.v35.flags & 0x01) != 0) o->flags |= FLAG_COMPRESSED_LZCRUNCH;
		if((h->dos32.v35.flags & 0x02) != 0) o->flags |= FLAG_PRINT_HEADER;
		if((h->dos32.v35.flags & 0x04) != 0) o->flags |= FLAG_OBSFUCATED_RELOCATIONS;

		// only valid in 3.5+
		o->flags |= FLAG_COMPRESSED_RELOCATIONS;

		o->header_size = h->dos32.v35.exec_offset;
		o->exec_length = h->dos32.v35.exec_length;
		o->length_after_stub = h->dos32.v35.length_after_stub;
		o->required_ram_size = (h->dos32.v35.required_ram_size + 1023) / 1024;

		if((o->flags & FLAG_COMPRESSED_LZCRUNCH) != FLAG_COMPRESSED_LZCRUNCH)
		{
			o->fixup_size = h->dos32.v35.exec_length - h->dos32.v35.fixup_table_offset;
			o->fixup_table_offset = stub_size + h->dos32.v35.exec_offset + h->dos32.v35.fixup_table_offset;
		}
	}
	else if(h->dos32_min >= 0x340 && h->dos32_min < 0x350)
	{
		// 3.4?
		o->dos32_ver = 0x340;
		o->eip = h->dos32.v33.eip;
		o->esp = h->dos32.v33.esp;
		if(o->esp == 0)
		{
			// set to undefined
			o->esp = UNDEFINED_ESP;
		}

		o->original_flags = h->dos32.v33.flags;
		if((h->dos32.v33.flags & 0x01) != 0) o->flags |= FLAG_COMPRESSED_LZCRUNCH;
		if((h->dos32.v33.flags & 0x02) != 0) o->flags |= FLAG_PRINT_HEADER;

		// 3.4r8? beta dlink
		if((h->dos32.v33.flags & 0x04) != 0) o->flags |= FLAG_COMPRESSED_LZCRUNCH; //FLAG_OBSFUCATED_RELOCATIONS;

		o->header_size = h->dos32.v33.exec_offset;
		o->exec_length = h->dos32.v33.exec_length;
		o->length_after_stub = h->dos32.v33.length_after_stub;
		o->required_ram_size = (h->dos32.v33.required_ram_size + 1023) / 1024;

		o->fixup_size = h->dos32.v33.fixup_count;
		o->fixup_table_offset = (stub_size + h->dos32.v33.exec_offset + h->dos32.v33.exec_length);
	}
	else if(h->dos32_min >= 0x303 && h->dos32_min < 0x340)
	{
		// 3.3 + 3.4?
		o->dos32_ver = 0x330;
		o->eip = h->dos32.v33.eip;
		o->esp = h->dos32.v33.esp;
		if(o->esp == 0)
		{
			// set to undefined
			o->esp = UNDEFINED_ESP;
		}

		o->original_flags = h->dos32.v33.flags;
		if((h->dos32.v33.flags & 0x01) != 0) o->flags |= FLAG_COMPRESSED_LZSS;
		if((h->dos32.v33.flags & 0x02) != 0) o->flags |= FLAG_PRINT_HEADER;

		o->header_size = h->dos32.v33.exec_offset;
		o->exec_length = h->dos32.v33.exec_length;
		o->length_after_stub = h->dos32.v33.length_after_stub;
		o->required_ram_size = (h->dos32.v33.required_ram_size + 1023) / 1024;

		o->fixup_size = h->dos32.v33.fixup_count;
		o->fixup_table_offset = (stub_size + h->dos32.v33.exec_offset + h->dos32.v33.exec_length);
	}
	else if(h->dlink_ver != 0 && h->dos32_min >= 0x300 && h->dos32_min < 0x303)
	{
		// these are swapped for 3.01!
		//o->dos32_ver = h->dlink_ver;
		//o->dlink_ver = h->dos32_min & 0x7FFF;
		// this is the dlink version

		o->dlink_ver = h->dlink_ver & 0x7FFF;

		if(h->dlink_ver >= 0x8000) o->flags |= FLAG_COMPRESSED_LZSS;

		if(h->dos32.v31.exec_offset >= 40)
		{
			o->original_flags = h->dos32.v31.flags;
			if((h->dos32.v31.flags & 0x01) != 0) o->flags |= FLAG_COMPRESSED_LZSS;
			if((h->dos32.v31.flags & 0x02) != 0) o->flags |= FLAG_PRINT_HEADER;
		}

		o->dos32_ver = 0x301;
		o->eip = h->dos32.v31.eip;
		o->esp = h->dos32.v31.esp;
		if(o->esp == 0)
		{
			// set to undefined
			o->esp = UNDEFINED_ESP;
		}

		o->header_size = h->dos32.v31.exec_offset;
		o->exec_length = h->dos32.v31.exec_length;
		o->length_after_stub = h->dos32.v31.length_after_stub;
		o->required_ram_size = (h->dos32.v31.required_ram_size + 1023) / 1024;

		o->fixup_size = h->dos32.v31.fixup_count;
		o->fixup_table_offset = (stub_size + h->dos32.v31.exec_offset + h->dos32.v31.exec_length);
	}
	else
	{
		// 3.0 had no compression
		//o->dlink_ver = h->dlink_ver & 0x7FFF;
		//if(h->dlink_ver >= 0x8000) o->flags |= FLAG_COMPRESSED_LZSS;

		// 3.00?
		o->dos32_ver = 0x300;
		o->eip = h->dos32.v30.eip;
		o->esp = h->dos32.v30.esp;
		if(o->esp == 0)
		{
			// set to undefined
			o->esp = UNDEFINED_ESP;
		}

		o->header_size = h->dos32.v30.exec_offset;
		o->exec_length = h->dos32.v30.exec_length;
		o->length_after_stub = h->dos32.v30.length_after_stub;
		o->required_ram_size = (h->dos32.v30.required_ram_size + 1023) / 1024;

		o->fixup_size = h->dos32.v30.fixup_count;
		o->fixup_table_offset = (stub_size + h->dos32.v30.exec_offset + h->dos32.v30.exec_length);
	}

	memset(xbuff, 0x0, 1024);
	fseek(fp, stub_size, SEEK_SET);
	fread(xbuff, 1, 1024, fp);

	// Test for VAC2 signature. Only 1 vac2 dos32 version exists.
	for(i=0; i < 1000; i++)
	{
		if( xbuff[i+0] == 0xFF
		 && xbuff[i+1] == 0xD3
		 && xbuff[i+2] == 0x73
		 && xbuff[i+4] == 0x31
		 && xbuff[i+5] == 0xC0
		 && xbuff[i+6] == 0xFF
		 && xbuff[i+7] == 0xD3
		 && xbuff[i+8] == 0x72
		 && xbuff[i+10] == 0xFF
		 && xbuff[i+11] == 0xD3
		 && xbuff[i+12] == 0x72)
		{
			o->flags |= FLAG_VAC2;

			// turn off rotated relocationss because should be none!
			// turn off compressed relocations because thats not a vac2 thing
			o->flags &= ~(FLAG_OBSFUCATED_RELOCATIONS | FLAG_COMPRESSED_RELOCATIONS);
		}
	}
}

static void header(void)
{
	printf("Info-DOS32 v0.3a - Dark Fiber [NuKE]\n");
}

static int bcd_unpack(uint8_t a)
{
	return (a >> 4) * 10 + (a&0xF);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	uD32Exec hdr;
	uDOS32Exec h;
	uMZExec mz;
	uint32_t stub_offset;
	uint32_t file_size;
	uint8_t xbuff[4];
	int i;
	uint32_t addr;

	int opt_relocs = 0;
	char *exec_file = NULL;

	header();

	for(i=1; i < argc; i++)
	{
		if(strcmp(argv[i], "-r") == 0)
		{
			opt_relocs = 1;
		}
		else if(exec_file == NULL)
		{
			exec_file = argv[i];
		}
		else
		{
			printf("Unknown argument '%s'\n", argv[i]);
			exit(0);
		}
	}

	if(exec_file == NULL)
	{
		printf("infod32 [-r] execfile\n");
		printf("-r	Print relocation information\n");
		exit(0);
	}

	memset(&h, 0x0, sizeof(uDOS32Exec));
	memset(&mz, 0x0, sizeof(uMZExec));

	fp = fopen(exec_file, "rb");
	if(fp != NULL)
	{


		fseek(fp, 0x0L, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);

		fread(&mz, 1, sizeof(uMZExec), fp);

		if(mz.signature[0] == 'M' && mz.signature[1] == 'Z')
		{
			printf("Found MS-DOS executable stub\n");

			stub_offset = mz.nblocks * 512;
			if(mz.lastsize > 0)
			{
				stub_offset -= 512;
				stub_offset += mz.lastsize;
			}

			printf("  Stub size : %li (0x%08lX)\n", stub_offset, stub_offset);
		}
		else
		{
			stub_offset = 0;
		}

		fseek(fp, stub_offset, SEEK_SET);

		fread(&h, 1, sizeof(uDOS32Exec), fp);

		if(memcmp(h.magic, "Adam", 4) == 0)
		{
			uint32_t eof_offs;

			ConvertDOS32Header(fp, stub_offset, &h, &hdr);

			printf("\n");
			printf("DOS32 Header found at 0x%08lX (%li bytes long)\n", hdr.header_offset, hdr.header_size);
			printf("Minimum DOS32 version reqiured v%i.%02i\n", bcd_unpack(hdr.dos32_ver>>8), bcd_unpack(hdr.dos32_ver&0xFF));
			printf("DLinked with version v%i.%02i\n", bcd_unpack(hdr.dlink_ver>>8), bcd_unpack(hdr.dlink_ver&0xFF));

			printf("DOS32 Compatability : 0x%04X\n", hdr.dos32_ver);
			printf("DLINK Compatability : 0x%04X\n", hdr.dlink_ver);
			printf("Length after stub 0x%08lX\n", hdr.length_after_stub);

			printf("Executable code length via header   %10li (0x%08lX)\n", hdr.exec_length, hdr.exec_length);


			fseek(fp, 0x0L, SEEK_END);

			if(hdr.dos32_ver < 0x350 || (hdr.flags & FLAG_COMPRESSED_LZCRUNCH) != FLAG_COMPRESSED_LZCRUNCH)
			{
				eof_offs = ftell(fp);
				eof_offs -= hdr.fixup_table_offset;
				if(hdr.dos32_ver < 0x350)
				{
					eof_offs -= 4 * hdr.fixup_size;
				}
				else
				{
					eof_offs -= hdr.fixup_size;
				}
				printf("Executable code length vs File Size %10li (0x%08lX)\n", eof_offs, eof_offs);
			}

			printf("Amount of memory required to run is %ikb\n", hdr.required_ram_size);
			printf("Initial EIP : 0x%08lX\n", hdr.eip);

			if(hdr.esp == UNDEFINED_ESP)
				printf("Initial ESP : Undefined\n");
			else
				printf("Initial ESP : 0x%08lX\n", hdr.esp);

			printf("Flags : %08lX (original = %08lX)\n", hdr.flags, hdr.original_flags);
			if((hdr.flags & FLAG_COMPRESSED_LZCRUNCH) != 0) printf("   Executable is load time compressed with LZ-Crunch\n");
			if((hdr.flags & FLAG_COMPRESSED_LZSS) != 0) printf("   Executable is load time compressed with LZSS\n");
			if((hdr.flags & FLAG_VAC2) != 0) printf("   Executable is runtime compressed with Vacuum/2\n");
			if((hdr.flags & FLAG_PRINT_HEADER) != 0) printf("   Display DOS32 Copyright Banner\n");
			if((hdr.flags & FLAG_OBSFUCATED_RELOCATIONS) != 0) printf("   Reloctation values are rotated\n");
			if((hdr.flags & FLAG_COMPRESSED_RELOCATIONS) != 0) printf("   Compressed reloctation values\n");

			if(hdr.dos32_ver >= 0x350)
			{
				if(((hdr.flags&FLAG_VAC2)!=FLAG_VAC2) && (hdr.flags & FLAG_COMPRESSED_LZCRUNCH) != FLAG_COMPRESSED_LZCRUNCH)
					printf("Fixup size %li bytes\n", hdr.fixup_size);
			}
			else
			{
				printf("Number of relocations %li\n", hdr.fixup_size);
			}

			if(((hdr.flags&FLAG_VAC2)!=FLAG_VAC2) && (hdr.dos32_ver < 0x350 || (hdr.flags & FLAG_COMPRESSED_LZCRUNCH) != FLAG_COMPRESSED_LZCRUNCH))
				printf("Fixup offset 0x%08lX\n", hdr.fixup_table_offset);

			if(opt_relocs == 1 && hdr.fixup_size > 0)
			{
				addr = 0;

				fseek(fp, hdr.fixup_table_offset, SEEK_SET);

				if(hdr.dos32_ver < 0x350)
				{
					for(i=0; i < hdr.fixup_size; i++)
					{
						fread(xbuff, 1, 4, fp);
						addr = (((uint32_t)xbuff[3]) << 24) | (((uint32_t)xbuff[2]) << 16) + (((uint32_t)xbuff[2]) << 8) + xbuff[0];
						printf("   fixup 0x%08lX\n", addr);
					}
				}
				else
				{
					if((hdr.flags & FLAG_COMPRESSED_RELOCATIONS) == FLAG_COMPRESSED_RELOCATIONS)
					{
						printf("For v3.5+, relocation table is compressed inside executable\n");
					}
					else
					{
						uint32_t reloc;
						uint32_t temp1;
						uint32_t count;
						uint8_t c;

						count = 0;
						reloc = 0;

						c = fgetc(fp);
						count += 1;

						while(count <= hdr.fixup_size)
						{
							c = ((c&0x7) << 5) | ((c&0xF8) >> 3);

							if((c & 1) == 1)
							{
								reloc = (reloc & 0xFFFF8000) | (c << 7);
								c = fgetc(fp);
								count += 1;
							}
							else
							{
								reloc = (reloc & 0xFFFFFFF0) + c;
								reloc >>= 1;
								addr += reloc;

								if(count >= hdr.fixup_size)
								{
									// base
									printf("  fixup 0x%08lX +base\n", addr);
									//break;
									c = fgetc(fp);
									count += 1;
								}
								else
								{
									c = fgetc(fp);
									count += 1;

									if(c == 0)
									{
										// word at
										printf("  fixup 0x%08lX\n", addr);
										c = fgetc(fp);
										count += 1;
									}
									else
									{
										// base
										printf("  fixup 0x%08lX +base\n", addr);
									}
								}
							}
						}
					}
				}
			}

		}
		else
			printf("Not a DOS32 executable\n");

		fclose(fp);
	}
	else
	{
		printf("Failed to open file %s\n", argv[1]);
	}

	return 0;
}
