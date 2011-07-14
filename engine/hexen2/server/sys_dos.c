/*
	sys_dos.c
	DOS system interface code.
	from quake1 source with adaptations for uhexen2.

	$Id$

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include "q_stdinc.h"
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <dos.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dpmi.h>
#include <crt0.h>	/* for _crt0_startup_flags */
#include <sys/nearptr.h>
#include <conio.h>
#include <time.h>

#include "quakedef.h"
#include "dosisms.h"
#include "debuglog.h"

#define MIN_MEM_ALLOC	0x0800000	/* minimum 8 mb */
#define STD_MEM_ALLOC	0x1000000	/* standart 16 mb */

/* 2000-07-16, DOSQuake/DJGPP mem detection fix by
 * Norberto Alfredo Bensa
 */
int	_crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK;

#define	LEAVE_FOR_CACHE		(512*1024)	/* FIXME: tune */
#define	LOCKED_FOR_MALLOC	(128*1024)	/* FIXME: tune */

int		end_of_memory;
static qboolean	lockmem, lockunlockmem, unlockmem;
static int	win95;

static quakeparms_t	quakeparms;
static int		sys_checksum;

/* 2000-07-28, DOSQuake time running too fast fix
 * by Norberto Alfredo Bensa. set USE_UCLOCK_TIME
 * to 0 if you want to use the old original code.
 * FIXME: uclock() isn't an ANSI C function..
 */
#define	USE_UCLOCK_TIME		1

static void Sys_InitTime (void);
#if !USE_UCLOCK_TIME
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static double		oldtime = 0.0;
#endif	/* ! USE_UCLOCK_TIME */

static qboolean		nostdout = false;
cvar_t			sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean		isDedicated = true;	/* compatibility */

static int		minmem;

float			fptest_temp;

extern char	start_of_memory __asm__("start");

//=============================================================================

// this is totally dependent on cwsdpmi putting the stack right after tge
// global data

// This does evil things in a Win95 DOS box!!!
#if 0
extern byte	end;
#define	CHECKBYTE	0xed
static void Sys_InitStackCheck (void)
{
	int		i;

	for (i = 0; i < 128*1024; i++)
		(&end)[i] = CHECKBYTE;
}

void Sys_StackCheck (void)
{
	int		i;

	for (i = 0; i < 128*1024; i++)
	{
		if ( (&end)[i] != CHECKBYTE )
			break;
	}

	Con_Printf ("%i undisturbed stack bytes\n", i);
	if (end != CHECKBYTE)
		Sys_Error ("System stack overflow!");
}
#endif

//=============================================================================


static void Sys_DetectWin95 (void)
{
	__dpmi_regs	r;

	r.x.ax = 0x160a;		/* Get Windows Version */
	__dpmi_int(0x2f, &r);

	if (r.x.ax || r.h.bh < 4)	/* Not windows or earlier than Win95 */
	{
		win95 = 0;
		lockmem = true;
		lockunlockmem = false;
		unlockmem = true;
	}
	else
	{
		win95 = 1;
		lockunlockmem = COM_CheckParm ("-winlockunlock");

		if (lockunlockmem)
			lockmem = true;
		else
			lockmem = COM_CheckParm ("-winlock");

		unlockmem = lockmem && !lockunlockmem;
	}
}


static void *dos_getmaxlockedmem (int *size)
{
	__dpmi_free_mem_info	meminfo;
	__dpmi_meminfo		info;
	int		working_size;
	void		*working_memory;
	int		last_locked;
	int		extra, 	i, j, allocsize;
	static const char msg[] = "Locking data...";
	int		m, n;
	byte		*x;
 
// first lock all the current executing image so the locked count will
// be accurate.  It doesn't hurt to lock the memory multiple times
	last_locked = __djgpp_selector_limit + 1;
	info.size = last_locked - 4096;
	info.address = __djgpp_base_address + 4096;

	if (lockmem)
	{
		if (__dpmi_lock_linear_region(&info))
		{
			Sys_Error ("Lock of current memory at 0x%lx for %ldKb failed!\n",
						info.address, info.size / 1024);
		}
	}

	__dpmi_get_free_memory_information(&meminfo);

	if (!win95)	/* Not windows or earlier than Win95 */
	{
		working_size = meminfo.maximum_locked_page_allocation_in_pages * 4096;
	}
	else
	{
		working_size = meminfo.largest_available_free_block_in_bytes - LEAVE_FOR_CACHE;
	}

	working_size &= ~0xffff;	/* Round down to 64K */
	working_size += 0x10000;

	do
	{
		working_size -= 0x10000;	/* Decrease 64K and try again */
		working_memory = sbrk(working_size);
	} while (working_memory == (void *)-1);

	extra = 0xfffc - ((unsigned)sbrk(0) & 0xffff);

	if (extra > 0)
	{
		sbrk(extra);
		working_size += extra;
	}

// now grab the memory
	info.address = last_locked + __djgpp_base_address;

	if (!win95)
	{
		info.size = __djgpp_selector_limit + 1 - last_locked;

		while (info.size > 0 && __dpmi_lock_linear_region(&info))
		{
			info.size -= 0x1000;
			working_size -= 0x1000;
			sbrk(-0x1000);
		}
	}
	else
	{	/* Win95 section */
		j = COM_CheckParm("-winmem");
	//	minmem = MIN_MEM_ALLOC;
		minmem = STD_MEM_ALLOC;

		if (j && j < com_argc - 1)
		{
			allocsize = ((int)(atoi(com_argv[j + 1]))) * 0x100000 +
							LOCKED_FOR_MALLOC;

			if (allocsize < (minmem + LOCKED_FOR_MALLOC))
				allocsize = minmem + LOCKED_FOR_MALLOC;
		}
		else
		{
			allocsize = minmem + LOCKED_FOR_MALLOC;
		}

		if (!lockmem)
		{
		// we won't lock, just sbrk the memory
			info.size = allocsize;
			goto UpdateSbrk;
		}

		// lock the memory down
		write (STDOUT_FILENO, msg, strlen (msg));

		for (j = allocsize; j > (minmem + LOCKED_FOR_MALLOC); j -= 0x100000)
		{
			info.size = j;

			if (!__dpmi_lock_linear_region(&info))
				goto Locked;

			write (STDOUT_FILENO, ".", 1);
		}

	// finally, try with the absolute minimum amount
		for (i = 0; i < 10; i++)
		{
			info.size = minmem + LOCKED_FOR_MALLOC;

			if (!__dpmi_lock_linear_region(&info))
				goto Locked;
		}

		Sys_Error ("Can't lock memory; %lu Mb lockable RAM required. "
				"Try shrinking smartdrv.", info.size / 0x100000);

Locked:

UpdateSbrk:

		info.address += info.size;
		info.address -= __djgpp_base_address + 4;	// ending point, malloc align
		working_size = info.address - (int)working_memory;
		sbrk(info.address - (int)sbrk(0));		// negative adjustment
	}

	if (lockunlockmem)
	{
		__dpmi_unlock_linear_region (&info);
		printf ("Locked and unlocked %d Mb data\n", working_size / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb data\n", working_size / 0x100000);
	}
	else
	{
		printf ("Allocated %d Mb data\n", working_size / 0x100000);
	}

// touch all the memory to make sure it's there. The 16-page skip is to
// keep Win 95 from thinking we're trying to page ourselves in (we are
// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)working_memory;

	for (n = 0; n < 4; n++)
	{
		for (m = 0; m < (working_size - 16 * 0x1000); m += 4)
		{
			sys_checksum += *(int *)&x[m];
			sys_checksum += *(int *)&x[m + 16 * 0x1000];
		}
	}

// give some of what we locked back for malloc before returning.  Done
// by cheating and passing a negative value to sbrk
	working_size -= LOCKED_FOR_MALLOC;
	sbrk( -(LOCKED_FOR_MALLOC));
	*size = working_size;
	return working_memory;
}


int Sys_mkdir (const char *path, qboolean crash)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0 && crash)
		Sys_Error("Unable to create directory %s", path);
	return rc;
}

int Sys_rmdir (const char *path)
{
	return rmdir(path);
}

int Sys_unlink (const char *path)
{
	return unlink(path);
}

int Sys_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Sys_filesize (const char *path)
{
	struct ffblk	f;

	if (findfirst(path, &f, FA_ARCH | FA_RDONLY) != 0)
		return -1;

	return (long) f.ff_fsize;
}

int Sys_FileType (const char *path)
{
	int attr = _chmod(path, 0);
	/* Root directories on some non-local drives
	   (e.g. CD-ROM) as well as devices may fail
	   _chmod, but we are not interested in such
	   cases.  */
	if (attr == -1)
		return FS_ENT_NONE;
	if (attr & _A_SUBDIR)
		return FS_ENT_DIRECTORY;
	if (attr & _A_VOLID)	/* we shouldn't hit this! */
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Sys_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	int	in, out;
	int		err = 0;
	long	remaining, count;

	in = open (frompath, O_RDONLY | O_BINARY);
	if (in < 0)
	{
		Con_Printf ("%s: unable to open %s\n", frompath, __thisfunc__);
		return 1;
	}
	remaining = filelength (in);
	if (remaining < 0)
	{
		Con_Printf ("%s: %s failed filelength()\n", frompath, __thisfunc__);
		close (in);
		return 1;
	}
	out = open (topath, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0666);
	if (out < 0)
	{
		Con_Printf ("%s: unable to create %s\n", topath, __thisfunc__);
		close (in);
		return 1;
	}

	memset (buf, 0, sizeof(buf));
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);

		if (read(in, buf, count) < 0)
		{
			err = -1;
			break;
		}

		if (write(out, buf, count) < 0)
		{
			err = -1;
			break;
		}

		remaining -= count;
	}

	if (!err)
	{
	// restore the file's timestamp
		struct ftime	ft;
		if (getftime(in, &ft) == 0)
			setftime(out, &ft);
	}

	close (in);
	close (out);

	return err;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only.
=================================================
*/
static struct ffblk	finddata;
static int		findhandle = -1;

char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle == 0)
		Sys_Error ("Sys_FindFirst without FindClose");

	memset (&finddata, 0, sizeof(finddata));

	findhandle = findfirst(va("%s/%s", path, pattern), &finddata, FA_ARCH | FA_RDONLY);
	if (findhandle == 0)
		return finddata.ff_name;

	return NULL;
}

char *Sys_FindNextFile (void)
{
	if (findhandle != 0)
		return NULL;

	if (findnext(&finddata) == 0)
		return finddata.ff_name;

	return NULL;
}

void Sys_FindClose (void)
{
	findhandle = -1;
}

char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int	textlen = 0;
	char		ch;

	if (!isDedicated)
		return NULL;

	if (! kbhit())
		return NULL;

	ch = getche();

	switch (ch)
	{
	case '\r':
		putch('\n');
		if (textlen)
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		break;

	case '\b':
		putch(' ');
		if (textlen)
		{
			textlen--;
			putch('\b');
		}
		break;

	default:
		con_text[textlen] = ch;
		textlen = (textlen + 1) & 0xff;
		break;
	}

	return NULL;
}

void Sys_Sleep (unsigned long msecs)
{
	if (!msecs)
		return;
	if (msecs > 1000)
		msecs = 1000;

	usleep (msecs * 1000);
}


static void Sys_Init (void)
{
	MaskExceptions ();

	Sys_SetFPCW ();

#if !USE_UCLOCK_TIME
	dos_outportb(0x43, 0x34);	// set system timer to mode 2
	dos_outportb(0x40, 0);		// for the Sys_DoubleTime() function
	dos_outportb(0x40, 0);
#endif	/* ! USE_UCLOCK_TIME */

	Sys_InitTime ();

	_go32_interrupt_stack_size = 4 * 1024;
	_go32_rmcb_stack_size = 4 * 1024;
}

void Sys_Shutdown (void)
{
	if (unlockmem)
	{
		dos_unlockmem (&start_of_memory, end_of_memory - (int)&start_of_memory);
		dos_unlockmem (quakeparms.membase, quakeparms.memsize);
	}
}


// =======================================================================
// General routines
// =======================================================================

void Sys_PrintTerm (const char *msgtxt)
{
	unsigned char		*p;

//	if (sys_nostdout.integer)
	if (nostdout)
		return;

	for (p = (unsigned char *) msgtxt; *p; p++)
		putc (*p, stdout);
}

static void Sys_AtExit (void)
{
// shutdown only once (so Sys_Error can call this function to shutdown, then
// print the error message, then call exit without exit calling this function
// again)
	Sys_Shutdown();
}


void Sys_Quit (void)
{
	Host_Shutdown ();

	exit (0);
}

#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];

	va_start (argptr, error);
	q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (con_debuglog)
	{
		LOG_Print (ERROR_PREFIX);
		LOG_Print (text);
		LOG_Print ("\n\n");
	}

	Host_Shutdown ();

	fprintf(stderr, ERROR_PREFIX "%s\n\n", text);

// Sys_AtExit is called by exit to shutdown the system
	exit (1);
}

/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
#if USE_UCLOCK_TIME
	return (double) uclock() / (double) UCLOCKS_PER_SEC;

#else
	int				r;
	unsigned		t, tick;
	double			ft, time;
	static int		sametimecount;

	Sys_PushFPCW_SetHigh ();

//{static float t = 0; t=t+0.05; return t;}	// DEBUG

	t = *(unsigned short*)real2ptr(0x46c) * 65536;

	dos_outportb(0x43, 0);	// latch time
	r = dos_inportb(0x40);
	r |= dos_inportb(0x40) << 8;
	r = (r - 1) & 0xffff;

	tick = *(unsigned short*)real2ptr(0x46c) * 65536;
	if ((tick != t) && (r & 0x8000))
		t = tick;

	ft = (double) (t + (65536 - r)) / 1193200.0;
	time = ft - oldtime;
	oldtime = ft;

	if (time < 0)
	{
		if (time > -3000.0)
			time = 0.0;
		else
			time += 3600.0;
	}

	curtime += time;

	if (curtime == lastcurtime)
	{
		sametimecount++;

		if (sametimecount > 100000)
		{
			curtime += 1.0;
			sametimecount = 0;
		}
	}
	else
	{
		sametimecount = 0;
	}

	lastcurtime = curtime;

	Sys_PopFPCW ();

	return curtime;
#endif	/* ! USE_UCLOCK_TIME */
}


/*
================
Sys_InitTime
================
*/
static void Sys_InitTime (void)
{
#if !USE_UCLOCK_TIME
	int		j;

	Sys_DoubleTime ();

	oldtime = curtime;

	j = COM_CheckParm("-starttime");

	if (j && j < com_argc - 1)
	{
		curtime = (double) (atof(com_argv[j+1]));
	}
	else
	{
		curtime = 0.0;
	}
	lastcurtime = curtime;
#endif	/* ! USE_UCLOCK_TIME */
}


/*
================
Sys_GetMemory
================
*/
static void Sys_GetMemory (void)
{
	int		j, tsize;

	j = COM_CheckParm("-mem");
	if (j && j < com_argc - 1)
	{
		quakeparms.memsize = (int) (atof(com_argv[j + 1]) * 1024 * 1024);
		quakeparms.membase = malloc (quakeparms.memsize);
	}
	else
	{
		quakeparms.membase = dos_getmaxlockedmem (&quakeparms.memsize);
	}

	fprintf(stderr, "malloc'd: %d\n", quakeparms.memsize);

	j = COM_CheckParm ("-heapsize");
	if (j && j < com_argc - 1)
	{
		tsize = atoi (com_argv[j + 1]) * 1024;

		if (tsize < quakeparms.memsize)
			quakeparms.memsize = tsize;
	}
}


/*
================
Sys_PageInProgram

walks the text, data, and bss to make sure it's all paged in so that the
actual physical memory detected by Sys_GetMemory is correct.
================
*/
static void Sys_PageInProgram (void)
{
	int		i, j;

	end_of_memory = (int)sbrk(0);

	if (lockmem)
	{
		if (dos_lockmem ((void *)&start_of_memory, end_of_memory - (int)&start_of_memory))
			Sys_Error ("Couldn't lock text and data");
	}

	if (lockunlockmem)
	{
		dos_unlockmem((void *)&start_of_memory, end_of_memory - (int)&start_of_memory);
		printf ("Locked and unlocked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else
	{
		printf ("Loaded %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}

// touch the entire image, doing the 16-page skip so Win95 doesn't think we're
// trying to page ourselves in
	for (j = 0; j < 4; j++)
	{
		for (i = (int)&start_of_memory; i < (end_of_memory - 16 * 0x1000); i += 4)
		{
			sys_checksum += *(int *)i;
			sys_checksum += *(int *)(i + 16 * 0x1000);
		}
	}
}


/*
================
Sys_NoFPUExceptionHandler
================
*/
static void Sys_NoFPUExceptionHandler (int whatever)
{
	printf ("\nError: Quake requires a floating-point processor\n");
	exit (0);
}


/*
================
Sys_DefaultExceptionHandler
================
*/
static void Sys_DefaultExceptionHandler (int whatever)
{
}


static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	char	*tmp;

	if (getcwd(dst, dstsize - 1) == NULL)
		return -1;

	tmp = dst;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0 && tmp != dst)
	{
		--tmp;
		if (tmp != dst && (*tmp == '/' || *tmp == '\\'))
			*tmp = 0;
	}

	return 0;
}


static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	printf ("Hexen II dedicated server %4.2f (%s)\n", ENGINE_VERSION, PLATFORM_STRING);
	printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

/*
================
main
================
*/
static char	cwd[MAX_OSPATH];

int main (int argc, char **argv)
{
	double		time, oldtime, newtime;

	PrintVersion();

// make sure there's an FPU
	signal(SIGNOFP, Sys_NoFPUExceptionHandler);
	signal(SIGABRT, Sys_DefaultExceptionHandler);
	signal(SIGALRM, Sys_DefaultExceptionHandler);
	signal(SIGKILL, Sys_DefaultExceptionHandler);
	signal(SIGQUIT, Sys_DefaultExceptionHandler);
	signal(SIGINT, Sys_DefaultExceptionHandler);

	if (fptest_temp >= 0.0)
		fptest_temp += 0.1;

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(argv[0], cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

	/* initialize the host params */
	memset (&quakeparms, 0, sizeof(quakeparms));
	quakeparms.basedir = cwd;
	quakeparms.userdir = cwd;
	quakeparms.argc = argc;
	quakeparms.argv = argv;
	host_parms = &quakeparms;

	LOG_Init (&quakeparms);

	COM_ValidateByteorder ();

	Sys_DetectWin95 ();
	Sys_PageInProgram ();
	Sys_GetMemory ();

	atexit (Sys_AtExit);	// in case we crash

	isDedicated = (COM_CheckParm ("-dedicated") != 0);

	Sys_Init ();

//	Sys_InitStackCheck ();

	Host_Init();

//	Sys_StackCheck ();
//	Con_Printf ("Top of stack: 0x%x\n", &time);

	oldtime = Sys_DoubleTime ();
	while (1)
	{
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		if (time < sys_ticrate.value)
			continue;

		Host_Frame (time);

//		Sys_StackCheck ();

		oldtime = newtime;
	}
}
