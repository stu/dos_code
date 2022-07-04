#ifndef DOS32_H
#define DOS32_H
#ifdef __cplusplus
extern "C"{
#endif


#pragma pack(1)

struct D32v35
{
	// for dlink >= 3.50
	// ds:1400? for relocation packing??
	uint32_t		exec_length;
	uint32_t		length_after_stub;
	uint32_t		exec_offset;
	uint32_t		eip;
	uint32_t		required_ram_size;
	uint32_t		esp;
	uint32_t		fixup_table_offset;
	uint8_t		ignore[2];
	uint16_t		flags;
	uint32_t		reserved;
};


struct D32v33
{
	// for dlink >=3.30 < 3.50
	uint32_t		length_after_stub;
	uint32_t		exec_offset;
	uint32_t		exec_length;
	uint32_t		required_ram_size;
	uint32_t		eip;
	uint32_t		esp;
	uint32_t		fixup_count;
	uint32_t		flags;
};

// we have seen a 3.01 with a header without flags entry... (sug_.exe)
struct D32v31
{
	// for dlink >= 1.00
	uint32_t		length_after_stub;			// 8
	uint32_t		exec_offset;					// 12
	uint32_t		exec_length;					// 16
	uint32_t		required_ram_size;			// 20
	uint32_t		eip;							// 24
	uint32_t		esp;							// 28
	uint32_t		fixup_count;					// 32
	uint32_t		flags;						// 36
};

struct D32v30
{
	// for dlink < 1.00
	uint32_t		length_after_stub;			// 8
	uint32_t		exec_offset;					// 12
	uint32_t		exec_length;					// 16
	uint32_t		required_ram_size;			// 20
	uint32_t		eip;							// 24
	uint32_t		esp;							// 28
	uint32_t		fixup_count;					// 32
};

// we will only be dealing with dlink v3.5+
typedef struct udtDOS32Exec
{
	uint8_t		magic[4];				// Adam

	// BCD
	uint16_t		dos32_min;					// 4

	// BCD
	uint16_t		dlink_ver;					// 6

	union
	{
		struct D32v35 v35;
		struct D32v33 v33;
		struct D32v31 v31;
		struct D32v30 v30;
	} dos32;

} uDOS32Exec;

typedef struct udtMZExec
{
	uint8_t 		signature[2];
	uint16_t		lastsize;
	uint16_t		nblocks;
	uint16_t		nreloc;
	uint16_t		hdrsize;
	uint16_t		minalloc;
	uint16_t		maxalloc;
	uint16_t		ss;
	uint16_t		sp;
	uint16_t		checksum;
	uint16_t		ip;
	uint16_t		cs;
	uint16_t		relocpos;
	uint16_t		noverlay;
} uMZExec;

#pragma pack()


#define FLAG_COMPRESSED_LZSS			0x01
#define FLAG_COMPRESSED_LZCRUNCH		0x02

#define FLAG_PRINT_HEADER			0x10
#define FLAG_OBSFUCATED_RELOCATIONS	0x20
#define FLAG_COMPRESSED_RELOCATIONS	0x40

#define FLAG_VAC2					0x80

#define UNDEFINED_ESP				0xAA55AA55

typedef struct udtD32Exec
{
	uint16_t		dos32_ver;
	uint16_t		dlink_ver;

	uint32_t		header_offset;
	uint32_t		header_size;

	uint32_t		length_after_stub;

	uint32_t		exec_length;

	uint32_t		required_ram_size;

	uint32_t		eip;
	uint32_t		esp;

	uint32_t		fixup_table_offset;
	uint32_t		fixup_size;

	uint32_t		flags;
	uint32_t		original_flags;
} uD32Exec;


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef DOS32_H
