/*	Purpose: DOS dir replacement
	Keywords: DOS, Watcom, dir, ls
	Changelog:
	20161215 (sgeorge) - Pulled old code
	- fixed drive free space bug when ran from other disk

	To Do;
	- Multlevel sorting eg: by extension then by name etc.

*/

#define APP_RELEASE "20161215"

#if __WATCOMC__ >= 2
#include <sys/_lfndos.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <i86.h>
#include <dos.h>
#include <env.h>
#include <bios.h>
#include <direct.h>
#include <stdint.h>

#include "dlist.h"


#define SORT_NAME_ASC	'N'
#define SORT_NAME_DESC	'n'
#define SORT_SIZE_ASC	'S'
#define SORT_SIZE_DESC	's'
#define SORT_DATE_ASC	'D'
#define SORT_DATE_DESC	'd'
#define SORT_EXT_ASC	'E'
#define SORT_EXT_DESC	'e'

#define MAX_SORT_COUNT	8

#define NAME_ENTRY_MAX	NAME_MAX

typedef struct udtEntry
{
	uint8_t name[NAME_ENTRY_MAX+1];
	uint32_t size;
	uint8_t attr;

	uint8_t clr;

	char *ext;

	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	uint8_t month;
	uint8_t day;
	uint16_t year;

	uint32_t adate;
	uint32_t atime;
} uEntry;

#define MAX_SIDX	32
typedef struct udtOpts
{
	uint8_t lower_case;
	uint8_t group_dirs;
	uint8_t time_24hour;
	uint8_t pause;

	char **clr_driver;

	uint8_t sort_idx;
	uint8_t sort[MAX_SORT_COUNT];
	uint8_t exclude;

	char *path;
	char *spec[MAX_SIDX];
	int sidx;
} uOpts;


enum filetypes
{
	TYPE_DIR = 0,
	TYPE_EXEC,
	TYPE_ARC,
	TYPE_TEXT,
	TYPE_IMAGE,
	TYPE_CODE,
	TYPE_BAD,
	TYPE_SOUND,
	TYPE_NORMAL
};

enum colour_codes
{
	CLR_BLACK	= 0,
	CLR_RED,
	CLR_GREEN,
	CLR_BROWN,
	CLR_BLUE,
	CLR_MAGENTA,
	CLR_CYAN,
	CLR_GREY,
	CLR_DK_GREY,
	CLR_BR_RED,
	CLR_BR_GRREEN,
	CLR_YELLOW,
	CLR_BR_BLUE,
	CLR_PURPLE,
	CLR_BR_CYAN,
	CLR_WHITE
};

#define CLR_DEFAULT		7

char *ansi_colours[16] =
{
	"\x1B[0;30m",
	"\x1B[0;31m",
	"\x1B[0;32m",
	"\x1B[0;33m",
	"\x1B[0;34m",
	"\x1B[0;35m",
	"\x1B[0;36m",
	"\x1B[0;37m",
	"\x1B[1;30m",
	"\x1B[1;31m",
	"\x1B[1;32m",
	"\x1B[1;33m",
	"\x1B[1;34m",
	"\x1B[1;35m",
	"\x1B[1;36m",
	"\x1B[1;37m"
};

typedef struct udtColourType
{
	uint8_t ext[4];
	uint8_t clr;
} uColourType;


uOpts opts;

#define MAX_CTYPES			256
uColourType ctypes[MAX_CTYPES];
int cindex = 0;
uint8_t clr_dir_type = CLR_DEFAULT;
uint8_t max_lines = 25;

#define SEARCH_SIZE	256
char search_pathspec[SEARCH_SIZE];

static void header(void)
{
#ifdef M_I386
	char *s = "i386";
#else
	char *s = "8086";
#endif
	printf("d - r" APP_RELEASE " (%s version) - Stu George\n", s);
}

static void syntax(void)
{
	header();
	printf("\n");
	printf("d [options] path\\filespec,filespec,filespec\n");
	printf("-l       - Display as lowercase\n");
	printf("-d       - Group directories first\n");
	printf("-24      - 24hour time display\n");
	printf("-p       - Pause printing per page\n");
	printf("-n       - No ANSI colour in output\n");
	printf("-o[nsde] - Sort by Name/Size/Extension/Date CAPS for ASC, lower for DESC\n");
	printf("           eg: -oN (Sort by Name ascending)\n");
	printf("-e[arhs] - Exclude Archive/Read-only/Hidden/System files\n");
	printf("           UPPERCASE Include, lowercase Exclude!\n");
	printf("\n");
	printf("-h       - This help\n");
	printf("-v       - Version\n");
	printf("\n");
	printf("Multiple filespecs are ran off the same path.\n");
	printf("eg: d c:\\utils\\*.exe,*.com,*.bat\n");
	printf("\n");
	printf("You can't do multiple paths, eg: d c:\\utils\\*.exe,c:\\dos\\*.exe\n");
}

static void parse_sort_string(char *s)
{
	int c = 2;

	if(s == NULL)
		return;

	while(s[c] != 0)
	{
		if(opts.sort_idx < MAX_SORT_COUNT)
		{
			switch(s[c])
			{
				case SORT_NAME_ASC:
				case SORT_NAME_DESC:
				case SORT_SIZE_ASC:
				case SORT_SIZE_DESC:
				case SORT_DATE_ASC:
				case SORT_DATE_DESC:
				case SORT_EXT_ASC:
				case SORT_EXT_DESC:
					opts.sort[opts.sort_idx++] = s[c++];
					break;
				default:
					printf("Unknown sort option\n");
					exit(1);
			}
		}
		else
		{
			printf("Too many sorting arguments\n");
			exit(1);
		}
	}
}

static void parse_colour(char *str, int type)
{
	int i;

	// brblue:exe,com,bat
	char *p;
	char *q;

	char *c[] = {
		"black",  "red",  "green",   "brown", "blue",   "magenta", "cyan",   "grey",
		"dkgrey", "brred", "brgreen", "yellow",  "brblue", "purple",  "brcyan", "white"
	};

	if(str == NULL)
		return;

	if(1 + cindex >= MAX_CTYPES)
		return;

	p = str;
	while(*p != 0x0 && *p != ':')
		p++;

	// handle dir type
	if(type == TYPE_DIR)
	{
		if(*p != 0x0)
			*p = 0;

		p = str;

		for(i=0; i < 16; i++)
		{
			if(strcmp(c[i], p) == 0)
			{
				clr_dir_type = i;
				return;
			}
		}
	}

	if(*p == 0x0)
		return;

	*p = 0;
	q = 1 + p;

	p = str;

	for(i=0; i < 16; i++)
	{
		if(strcmp(c[i], p) == 0)
		{
			while(*q != 0)
			{
				int j;

				for(j=0; j < 3 && q[j] != ','; j++)
				{
					ctypes[cindex].ext[j] = q[j];
				}

				ctypes[cindex].ext[j] = 0;
				ctypes[cindex].clr = i;

				q += j;
				if(*q == ',')
					q++;

				cindex++;
			}

			return;
		}
	}

	printf("Unknown colour '%s'\n", p);
	exit(1);
}

static int ext_to_ctype(char *fname)
{
	char *p;
	int i;

	p = strchr(fname, 0x0);
	while(p != fname && *p != '.')
		p--;

	if(p == fname)
		return CLR_DEFAULT;

	if(*p == '.')
		p++;

	for(i=0; i < cindex; i++)
	{
		if(stricmp(p, ctypes[i].ext) == 0)
			return ctypes[i].clr;
	}

	return CLR_DEFAULT;
}

static void spec_parse(char *p)
{
	char *q;

	q = p;
	while(*q != 0 && opts.sidx < MAX_SIDX)
	{
		while(*q != ',' && *q != 0x0)
			q++;

		if(*q == ',')
		{
			*q = 0;
			q++;
		}

		opts.spec[opts.sidx] = strdup(p);
		opts.sidx += 1;
		p = q;
	}
}

static void parse_args(char *arg_string, int reset_sort_index)
{
	char *p = arg_string;
	char conv;

	if(arg_string == NULL)
		return;

	while(*p != 0)
	{
		char *q;

		while(*p == 0x20)
			p++;

		q = p;
		while(*q != 0 && *q != ' ')
			q++;

		if(*q != 0)
		{
			*q = 0;
			q++;
		}

		conv = p[0];
		if(p[0] == '/')
			p[0] = '-';

		if(strcmp(p, "-?") == 0 || strcmp(p, "-h") == 0)
		{
			syntax();
			exit(1);
		}
		else if(strcmp(p, "-v") == 0)
		{
			header();
			exit(1);
		}
		else if(strcmp(p, "-l") == 0)
		{
			opts.lower_case = 1;
		}
		else if(strcmp(p, "-d") == 0)
		{
			opts.group_dirs = 1;
		}
		else if(strcmp(p, "-24") == 0)
		{
			opts.time_24hour = 1;
		}
		else if(strcmp(p, "-p") == 0)
		{
			opts.pause = 1;
		}
		else if(strcmp(p, "-n") == 0)
		{
			opts.clr_driver = NULL;
		}
		else if(p[0] == '-' && p[1] == 'e')
		{
			p += 2;
			while(*p != 0x0)
			{
				switch(*p)
				{
					case 'a':
						opts.exclude |= _A_ARCH;
						break;
					case 'A':
						opts.exclude &= ~_A_ARCH;
						break;
					case 'r':
						opts.exclude |= _A_RDONLY;
						break;
					case 'R':
						opts.exclude &= ~_A_RDONLY;
						break;
					case 'h':
						opts.exclude |= _A_HIDDEN;
						break;
					case 'H':
						opts.exclude &= ~_A_HIDDEN;
						break;
					case 's':
						opts.exclude |= _A_SYSTEM;
						break;
					case 'S':
						opts.exclude &= ~_A_SYSTEM;
						break;
					default:
						printf("Unknown exclude option '%c'\n", *p);
						exit(1);
				}

				p++;
			}
		}
		else if(p[0] == '-' && p[1] == 'o')
		{
			if(reset_sort_index != 0)
			{
				opts.sort_idx = 0;
				memset(opts.sort, 0x0, MAX_SORT_COUNT);
			}

			parse_sort_string(p);
		}
		else if(p[0] == '-')
		{
			printf("Unknown switch %s\n", p);
			exit(1);
		}
		else
		{
			p[0] = conv;

			if(opts.sidx < MAX_SIDX)
			{
				char *z;
				if(opts.path == NULL)
				{
					opts.path = strdup(p);

					z = strchr(opts.path, 0x0);

					while(z != opts.path && (*z != '\\') && (*z != ':'))
						z--;

					if(*z == '\\' || *z == ':')
					{
						z++;
						spec_parse(z);
						*z = 0;
					}
					else
					{
						free(opts.path);
						opts.path = strdup(".\\");

						spec_parse(p);
					}
				}
				else
				{
					spec_parse(p);
				}
			}
			else
			{
				printf("max search spec reached\n");
				exit(1);
			}
		}

		p = q;
	}

}

static void FreeEntry(void *q)
{
	free(q);
}

static DList* sort_list(DList *h)
{
	DList *lstX;
	DLElement *li, *lx;
	uEntry *e1, *e2;

	int i;
	int flag;

	lstX = NewDList(NULL);
	li = dlist_head(h);

	while(li != NULL)
	{
		e1 = dlist_data(li);
		li = dlist_next(li);

		lx = dlist_head(lstX);
		flag = 0;

		if(opts.sort[0] == SORT_NAME_ASC || opts.sort[0] == SORT_NAME_DESC)
		{
			while(flag == 0 && lx != NULL)
			{
				e2 = dlist_data(lx);
				if(opts.sort[0] == SORT_NAME_ASC)
				{
					if(strcmp(e1->name, e2->name) < 0)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				else
				{
					if(strcmp(e1->name, e2->name) > 0)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				lx = dlist_next(lx);
			}
		}
		else if(opts.sort[0] == SORT_EXT_ASC || opts.sort[0] == SORT_EXT_DESC)
		{
			while(flag == 0 && lx != NULL)
			{
				e2 = dlist_data(lx);
				if(opts.sort[0] == SORT_EXT_ASC)
				{
					if(strcmp(e1->ext, e2->ext) < 0)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				else
				{
					if(strcmp(e1->ext, e2->ext) > 0)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				lx = dlist_next(lx);
			}
		}
		else if(opts.sort[0] == SORT_DATE_ASC || opts.sort[0] == SORT_DATE_DESC)
		{
			while(flag == 0 && lx != NULL)
			{
				e2 = dlist_data(lx);
				if(opts.sort[0] == SORT_DATE_ASC)
				{
					if(e1->adate <= e2->adate && e1->atime < e2->atime)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				else
				{
					if(e1->adate >= e2->adate && e1->atime > e2->atime)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				lx = dlist_next(lx);
			}
		}
		else if(opts.sort[0] == SORT_SIZE_ASC || opts.sort[0] == SORT_SIZE_DESC)
		{
			while(flag == 0 && lx != NULL)
			{
				e2 = dlist_data(lx);
				if(opts.sort[0] == SORT_SIZE_ASC)
				{
					if(e1->size < e2->size)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				else
				{
					if(e1->size > e2->size)
					{
						dlist_ins_prev(lstX, lx, e1);
						flag = 1;
					}
				}
				lx = dlist_next(lx);
			}
		}
		else
		{
			// insert in load order. do nothing.
		}

		if(flag == 0)
		{
			dlist_ins(lstX, e1);
		}
	}

	FreeDList(h);
	return lstX;
}

void format_commas(uint32_t n, char *out)
{
	int c;
	char buf[20];
	char *p;

	sprintf(buf, "%lu", n);
	c = 2 - strlen(buf) % 3;

	for (p = buf; *p != 0; p++)
	{
		*out++ = *p;
		if (c == 1)
		{
			*out++ = ',';
		}
		c = (c + 1) % 3;
	}

	*--out = 0;
}

static void print_clr(uint8_t clr)
{
	if(opts.clr_driver == NULL)
		return;

	printf(opts.clr_driver[clr]);
	fflush(stdout);
}

static int dopause(int linecount)
{
	if(opts.pause == 0)
		return 0;

	if(2 + linecount == max_lines)
	{
		printf("Press any key to continue...");
		fflush(stdout);

		_bios_keybrd(_KEYBRD_READ);
		linecount = 0;

		printf("\r                               \r");
		fflush(stdout);
	}

	return linecount;
}

static int print_list(DList *h, int dirs_only, int linecount)
{
	DLElement *li;
	uEntry *e;
	uint8_t hh;
	char ssize[16];
	struct diskfree_t df;

	uint32_t bps;
	uint32_t spc;
	uint32_t a;
	uint32_t b;

	int flag;

	uint32_t fsize = 0;
	uint32_t fcount = 0;
	uint32_t dcount = 0;

	if(opts.time_24hour == 1)
		hh = 24;
	else
		hh = 12;

	li = dlist_head(h);
	while(li != NULL)
	{
		char attrs[8];
		char xdate[16];
		char xtime[16];

		e = dlist_data(li);
		li = dlist_next(li);

		flag = 0;

		if(dirs_only == 0)
		{
			flag = 0;
		}
		else if(dirs_only == 1 && ((e->attr & _A_SUBDIR) == _A_SUBDIR))
		{
			flag = 0;
		}
		else if(dirs_only == 2 && ((e->attr & _A_SUBDIR) != _A_SUBDIR))
		{
			flag = 0;
		}
		else
		{
			flag = 1;
		}

		if((e->attr & _A_SUBDIR) == _A_SUBDIR)
		{
			strcpy(ssize, "<DIR>");
			dcount += 1;
		}
		else
		{
			format_commas(e->size, ssize);
			fcount += 1;
			fsize += e->size;
		}

		if(flag == 0)
		{
			strcpy(attrs, "     ");
			if((e->attr & _A_SUBDIR) == _A_SUBDIR)
				attrs[0] = 'd';
			if((e->attr & _A_ARCH) == _A_ARCH)
				attrs[1] = 'a';
			if((e->attr & _A_RDONLY) == _A_RDONLY)
				attrs[2] = 'r';
			if((e->attr & _A_HIDDEN) == _A_HIDDEN)
				attrs[3] = 'h';
			if((e->attr & _A_SYSTEM) == _A_SYSTEM)
				attrs[4] = 's';

			sprintf(xtime, "%2i:%02i.%02i%c", e->hour > hh ? e->hour-hh : e->hour, e->minute, e->second, opts.time_24hour == 0 ? (e->hour > 12 ? 'p' : 'a') : ' ' );
			sprintf(xdate, "%02i-%02i-%04i", e->day, e->month, e->year);

			printf("%s %s %s %13s ", attrs, xdate, xtime, ssize);

			print_clr(e->clr);
			printf("%s", e->name);
			print_clr(CLR_GREY);
			printf("\n");

			linecount += 1;
			linecount = dopause(linecount);
		}
	}

	if(dirs_only != 1)
	{
		printf("\n");
		linecount += 1;
		linecount = dopause(linecount);

		format_commas(fcount, ssize);
		printf("          %5s files(s)", ssize);
		format_commas(fsize, ssize);
		printf(" %15s bytes\n", ssize);
		linecount += 1;
		linecount = dopause(linecount);

		format_commas(dcount, ssize);
		printf("          %5s dir(s)  ", ssize);

		//'A' + _getdrive() - 1
		if(opts.spec[0][1] == ':')
		{
			_getdiskfree( (toupper(opts.path[0]) - 'A') + 1  , &df);
		}
		else
		{
			_getdiskfree(_getdrive(), &df);
		}

		bps = df.bytes_per_sector;
		spc = df.sectors_per_cluster;
		a = bps * spc;
		b = df.avail_clusters * a;

		format_commas(b, ssize);

		printf(" %15s bytes free\n", ssize);
		linecount += 1;

		// last one, dont care about pause
		//linecount = dopause(linecount)
	}

	printf("\x1B[0m");
	fflush(stdout);

	return linecount;
}

void run_dir(void)
{
	int si;
	int lines;
	DList *h;
	struct find_t ff;

	uint8_t attrs = _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR | _A_ARCH;

	h = NewDList(NULL);

	for(si=0; si < opts.sidx; si++)
	{
		strcpy(search_pathspec, opts.path);
		strcat(search_pathspec, opts.spec[si]);
		if(_dos_findfirst(search_pathspec, attrs, &ff) == 0)
		{
			uEntry *e;

			do
			{

				if( (ff.attrib & opts.exclude) == 0)
				{
					e = calloc(1, sizeof(uEntry));

					memmove(e->name, ff.name, NAME_ENTRY_MAX);
					if(opts.lower_case == 1)
						strlwr(e->name);

					if((ff.attrib & _A_SUBDIR) != _A_SUBDIR)
					{
						char *ext = strchr(e->name, 0x0);
						while(ext != e->name && *ext != '.')
							ext--;

						if(*ext == '.')
							ext++;

						e->ext = ext;
					}

					e->attr = ff.attrib;
					e->size = ff.size;

					e->hour = (ff.wr_time >> 11) & 0x1F;
					e->minute = (ff.wr_time >> 5) & 0x3F;
					e->second = (ff.wr_time & 0x1F) * 2;

					e->month = (ff.wr_date >> 5) & 0xF;
					e->day = (ff.wr_date) & 0x1F;
					e->year = 1980 + ((ff.wr_date >> 9) & 0x7F);

					e->adate = (uint32_t)(e->year * 65536) + (e->month << 8) + e->day;
					e->atime = (uint32_t)(e->hour * 65536) + (e->minute << 8) + e->second;

					e->clr = ext_to_ctype(e->name);

					// dont colourise directory names
					if((e->attr & _A_SUBDIR) == _A_SUBDIR)
					{
						e->clr = clr_dir_type;
					}

					dlist_ins(h, e);
				}
			}while(_dos_findnext(&ff) == 0);
		}
		_dos_findclose(&ff);
	}

	h = sort_list(h);

	if(opts.group_dirs == 0)
	{
		lines = print_list(h, 0, 0);
	}
	else
	{
		lines = print_list(h, 1, 0);
		lines = print_list(h, 2, lines);
	}

	h->destroy = FreeEntry;
	FreeDList(h);
}

static void cleanup_spec(void)
{
	char *z;
	char *p;
	char *q;
	uint8_t attrs = _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR | _A_ARCH;

	struct find_t ff;

	///////////////////////////////////////////////////////////////////////
	// 1 - if null, set it to default
	if(opts.path == NULL)
	{
		opts.path = strdup(".\\");
		opts.spec[0] = strdup("*.*");
		opts.sidx = 1;
		return;
	}

	// back spaces out.
	p = strchr(opts.path, 0x0);
	p -= 1;
	while(*p == 0x20 && p > opts.path)
	{
		*p = 0x0;
		p--;
	}

	///////////////////////////////////////////////////////////////////////
	// 2 - if its a dir or drive eg: c:/dos/
	// append *.* to it
	p = strchr(opts.path, 0x0);
	p -= 1;

	// back spaces out.
	while(*p == 0x20 && p > opts.path)
	{
		p--;
	}

	if(*p == ':' && opts.sidx == 0)
	{
		opts.spec[0] = strdup("*.*");
		opts.sidx += 1;
		return;
	}

	if(*p == '\\' && opts.sidx == 0)
	{
		opts.spec[0] = strdup("*.*");
		opts.sidx += 1;
		return;
	}

	///////////////////////////////////////////////////////////////////////
	// 3 - test if its a dir, so we dir into it
	// eg: c:\dos
	// turn it into c:\dos\*.*

	strcpy(search_pathspec, opts.path);
	strcat(search_pathspec, opts.spec[0]);
	if(_dos_findfirst(search_pathspec, attrs, &ff) == 0)
	{
		if((ff.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if(_dos_findnext(&ff) != 0)
			{
				free(opts.path);

				opts.path = malloc(16 + strlen(search_pathspec));
				strcpy(opts.path, search_pathspec);
				strcat(opts.path, "\\");

				free(opts.spec[0]);
				opts.spec[0] = strdup("*.*");
			}
		}
	}
	_dos_findclose(&ff);
}


static void parse_cfg(char *s)
{
	char *buff;
	FILE *fp;
	char *p;
	uint32_t len;
	char cfg[1024];

	buff = malloc(strlen(s) + 8);
	strcpy(buff, s);

	p = strchr(buff, 0x0);

	while(p != buff && *p !='\\' && *p != '/')
	{
		p--;
	}

	if(*p == '\\' || *p == '/')
	{
		p += 1;
		strcpy(p, "d.cfg");
		fp = fopen(buff, "rt");
		if(fp != NULL)
		{
			fseek(fp, 0x0L, SEEK_END);
			len = ftell(fp);
			fseek(fp, 0x0L, SEEK_SET);

			while(ftell(fp) < len)
			{
				memset(cfg, 1024, 0x0);
				fgets(cfg, 1024, fp);

				while(isspace(*cfg) != 0)
				{
					memmove(cfg, cfg + 1, 1023);
				}

				p = strchr(cfg, 0x0a);
				if(p != NULL) *p = 0x0;

				p = strchr(cfg, 0x0D);
				if(p != NULL) *p = 0x0;

				p = strchr(cfg, '=');
				if(p != NULL)
				{
					*p = 0x0;
					p += 1;
				}

				if(strcmp(cfg, "DCLR_EXEC") == 0)
					parse_colour(p, TYPE_EXEC);
				else if(strcmp(cfg, "DCLR_ARC") == 0)
					parse_colour(p, TYPE_ARC);
				else if(strcmp(cfg, "DCLR_TEXT") == 0)
					parse_colour(p, TYPE_TEXT);
				else if(strcmp(cfg, "DCLR_IMAGE") == 0)
					parse_colour(p, TYPE_IMAGE);
				else if(strcmp(cfg, "DCLR_CODE") == 0)
					parse_colour(p, TYPE_CODE);
				else if(strcmp(cfg, "DCLR_BAD") == 0)
					parse_colour(p, TYPE_BAD);
				else if(strcmp(cfg, "DCLR_SOUND") == 0)
					parse_colour(p, TYPE_SOUND);
				else if(strcmp(cfg, "DCLR_DIR") == 0)
					parse_colour(p, TYPE_DIR);
				else if(strcmp(cfg, "D") == 0)
					parse_args(p, 0);
			}

			fclose(fp);
		}
	}

	free(buff);
}

int main(int argc, char *argv[])
{
	int i;
	int j;
	char *s;

#ifndef M_I386
	unsigned char far *dfp = MK_FP(0x40, 0x0);
	// 0x84 = lines-1 (eg: 24)
	// 0x4A = width (eg: 80)
	max_lines = dfp[0x84];
#else
	unsigned char *dfp = (unsigned char*)0x400;
	max_lines = dfp[0x84];
#endif

	memset(&opts, 0x0, sizeof(uOpts));
	memset(search_pathspec, 0x0, SEARCH_SIZE);

	opts.clr_driver = ansi_colours;
	clr_dir_type = CLR_DEFAULT;

	for(j=0, i=1; i < argc; i++)
		j += strlen(argv[i]);

	s = calloc(1, 4 + argc + j);
	for(i=1; i < argc; i++)
	{
		if(i > 1)
			strcat(s, " ");
		strcat(s, argv[i]);
	}

	parse_cfg(argv[0]);

	parse_args(getenv("D"), 0);
	parse_colour(getenv("DCLR_EXEC"), TYPE_EXEC);
	parse_colour(getenv("DCLR_ARC"), TYPE_ARC);
	parse_colour(getenv("DCLR_TEXT"), TYPE_TEXT);
	parse_colour(getenv("DCLR_IMAGE"), TYPE_IMAGE);
	parse_colour(getenv("DCLR_CODE"), TYPE_CODE);
	parse_colour(getenv("DCLR_BAD"), TYPE_BAD);
	parse_colour(getenv("DCLR_SOUND"), TYPE_SOUND);
	parse_colour(getenv("DCLR_DIR"), TYPE_DIR);

	parse_args(s, 1);
	cleanup_spec();
	free(s);

	/* if we get here, no errors, lets parse! */
	run_dir();

	free(opts.path);

	return 0;
}
