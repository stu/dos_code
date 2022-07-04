#ifndef CPU_H
#define CPU_H
#ifdef __cplusplus
extern "C"{
#endif

#ifndef M_I86
#error Please compile with 16bit compiler
#endif

#pragma pack(1)
typedef struct udtXREGS
{
	union
	{
		struct
		{
			uint32_t eax;
			uint32_t ecx;
			uint32_t edx;
			uint32_t ebx;
			uint32_t esi;
			uint32_t edi;
			uint32_t ebp;
			uint16_t ds;
			uint16_t es;
			uint16_t fs;
			uint16_t gs;
			uint32_t flags;
		} x;

		struct
		{
			uint16_t ax;	uint16_t __na0;
			uint16_t cx;	uint16_t __na1;
			uint16_t dx;	uint16_t __na2;
			uint16_t bx;	uint16_t __na3;
			uint16_t si;	uint16_t __na4;
			uint16_t di;	uint16_t __na5;
			uint16_t bp;	uint16_t __na6;
			uint16_t ds;
			uint16_t es;
			uint16_t fs;
			uint16_t gs;
			uint16_t flags;	uint16_t __na7;
		} w;
	};
} uXREGS;
#pragma pack();

#define X86_CARRY_FLAG		0x0001

extern uint16_t DetectCPU(void);
extern void dosint(uint8_t intcc, uXREGS *regs);


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef CPU_H
