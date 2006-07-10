/*
  test-fb.c -- Measure fullscreen framebuffer write speed. 
  Version 0.5

  Copyright (C) 2003 Matthew Allum, Openedhand Ltd. 

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Matthew Allum mallum@openedhand.com

  ===
 
  Compile with

  gcc -Wall -O2 test-fb.c -o test-fb

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#define VERSION "0.5"

static int con_fd = -1, fb_fd, last_vt = -1;
static struct fb_fix_screeninfo fix;
static struct fb_var_screeninfo vinfo;
static struct fb_cmap cmap;
static char  *fbuffer;
static int    fb_fd=0;
static int    xres, yres;
static int    Verbose = 0;
static int    TotalCycles = 100;

static char *defaultfbdevice = "/dev/fb0";
static char *defaultconsoledevice = "none";

static int   WantMultiBlit = 0;

static int 
framebuffer_open (char *fbdevice,
		  char *consoledevice);
static void
framebuffer_blit(void);

static void 
framebuffer_close(void);

static unsigned long long
GetTimeInMillis(void)
{
  struct timeval  tp;

  gettimeofday(&tp, 0);
  return (unsigned long long)(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

static int 
framebuffer_open (char *fbdevice,
		  char *consoledevice)
{
  struct vt_stat vts;
  char vtname[128];
  int fd, nr;
  unsigned short col[2];
  
  if (fbdevice == NULL) 
    fbdevice = defaultfbdevice;
  
  if (consoledevice == NULL) 
    consoledevice = defaultconsoledevice;
  
  if (strcmp(consoledevice,"none") != 0) 
    {
      sprintf(vtname,"%s%d", consoledevice, 1);
      fd = open(vtname, O_WRONLY);

      if (fd < 0) 
	{
	  perror("open consoledevice");
	  return -1;
	}
 
      /* Now get next available terminal */
      if (ioctl(fd, VT_OPENQRY, &nr) < 0) 
	{
	  perror("ioctl VT_OPENQRY");
	  return -1;
	}
      close(fd);  /* Close initial */
    
      /* Now open next available */
      sprintf(vtname, "%s%d", consoledevice, nr);
    
      con_fd = open(vtname, O_RDWR | O_NDELAY);
      if (con_fd < 0) 
	{
	  perror("open tty");
	  return -1;
	}
    
      /* Get info about it */
      if (ioctl(con_fd, VT_GETSTATE, &vts) == 0)
	last_vt = vts.v_active;
    
      /* Activate it */
      if (ioctl(con_fd, VT_ACTIVATE, nr) < 0) 
	{
	  perror("VT_ACTIVATE");
	  close(con_fd);
	  return -1;
	}
    
      /* Wait for it to activate */
      if (ioctl(con_fd, VT_WAITACTIVE, nr) < 0) 
	{
	  perror("VT_WAITACTIVE");
	  close(con_fd);
	  return -1;
	}
    
      /* Set the graphics mode */
      if (ioctl(con_fd, KDSETMODE, KD_GRAPHICS) < 0) 
	{
	  perror("KDSETMODE");
	  close(con_fd);
	  return -1;
	}
    }
  
  /* Now open the framebuffer */
  fb_fd = open(fbdevice, O_RDWR);
  if (fb_fd == -1) 
    {
      perror("open fbdevice");
      return -1;
    }

  /* Get the framebuffer info */
  if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix) < 0) 
    {
      perror("ioctl FBIOGET_FSCREENINFO");
      close(fb_fd);
      return -1;
    }
  
  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) 
    {
      perror("ioctl FBIOGET_VSCREENINFO");
      close(fb_fd);
      return -1;
    }

  xres = vinfo.xres;
  yres = vinfo.yres;
  
  cmap.start = 0;
  cmap.len = 2;
  cmap.red = col;
  cmap.green = col;
  cmap.blue = col;
  cmap.transp = NULL;
  
  col[0] = 0;
  col[1] = 0xffff;
  
  /*
  if(var.bits_per_pixel==8) 
    {
      if (ioctl(fb_fd, FBIOPUTCMAP, &cmap) < 0) 
	{
	  perror("ioctl FBIOPUTCMAP");
	  close(fb_fd);
	  return -1;
	}
    }
  */
  /*
  if(vinfo.bits_per_pixel != 16)
    {
      perror("Framebuffer is not 16bpp.");
      close(fb_fd);
      return -1;
    }
  */
  fbuffer = mmap(NULL, fix.smem_len, 
		 PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, 
		 fb_fd, 0);

  if (fbuffer == (char *)-1) 
    {
      perror("mmap framebuffer");
      close(fb_fd);
      return -1;
    }

  memset(fbuffer,0,fix.smem_len);
  
  return 0;
}

static void 
framebuffer_close(void)
{
  munmap(fbuffer, fix.smem_len);
  close(fb_fd);
  
  if(con_fd != -1) 
    {
    
      if (ioctl(con_fd, KDSETMODE, KD_TEXT) < 0)
	perror("KDSETMODE");
    
      if (last_vt >= 0)
	if (ioctl(con_fd, VT_ACTIVATE, last_vt))
	  perror("VT_ACTIVATE");
    
      close(con_fd);
    }
}

static void
framebuffer_blit(void)
{
  int x = 0, y = 0, location = 0;
  char  *data = NULL;
  int    size = vinfo.yres * vinfo.xres * (vinfo.bits_per_pixel/8);
  unsigned long long start_clock,finish_clock,diff_clock;
  int     cycles = 0;

  data = malloc( size );

  start_clock = GetTimeInMillis(); 

  if (vinfo.bits_per_pixel == 16)
    {
      for ( y = 0; y < vinfo.yres; y++ )
	for ( x = 0; x < vinfo.xres; x++ ) 
	  {
	    /* Random colours */
	    int b = 10;
	    int g = ( x - 100 ) / 6;   
	    int r = 31 - ( y - 100 ) / 16;
	    
	    location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
	  (y + vinfo.yoffset) * fix.line_length;
	
	    *((unsigned short int*)(data + location)) = (r<<11 | g << 5 | b);
	  }
    }
  else 
    {
      /* Assume 8 */

      for ( y = 0; y < vinfo.yres; y++ )
	for ( x = 0; x < vinfo.xres; x++ ) 
	  {
	    location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
	      (y + vinfo.yoffset) * fix.line_length;
	
	    *((unsigned char*)(data + location)) = rand() * 255;
	  }
    }

  finish_clock = GetTimeInMillis();

  if (Verbose)
    {
      printf("test-fb: Surface ( %i X %i, %i KB ) Created in %lli ms\n",
	     vinfo.xres, vinfo.yres, size / 1024, finish_clock - start_clock);
    }

  start_clock = GetTimeInMillis(); 

  if (WantMultiBlit)
    {
      for (cycles=0; cycles<TotalCycles; cycles++)
	{
	  for (y = 0; y <vinfo.yres; y++)
	    {
	      memcpy(fbuffer + ( y * vinfo.xres * 2), 
		     data + ( y * vinfo.xres * 2), 
		     vinfo.xres * 2 );
	    }
	}
    }
  else
    {
      for (cycles=0; cycles<100; cycles++)
	memcpy(fbuffer, data, size);
    }

  finish_clock = GetTimeInMillis();

  diff_clock = finish_clock - start_clock;

  printf("test-fb: Framebuffer write speed: %lli KB/Sec\n",
	 ( (unsigned long long) (size/1024) * 1000 * cycles) / diff_clock);

  if (Verbose)
    {
      printf("test-fb: Approx frame rate: %lli frames/sec\n",
	     (unsigned long long) cycles*1000 / diff_clock);
    }

  
  free(data);
}

static void
usage(void)
{
  fprintf(stderr, 
	  "test-fb " VERSION "\n"
	  "usage: test-fb [--verbose] [--multiblit] [--cycles <int>]\n");
  exit(1);
} 

int 
main (int argc, char **argv)
{
  int i;

  for (i = 1; i < argc; i++) {
    if (!strcmp ("--verbose", argv[i]) || !strcmp ("-v", argv[i])) {
      Verbose = 1;
      continue;
    }
    if (!strcmp ("--multiblit", argv[i])) {
      WantMultiBlit = 1;
      continue;
    }

    if (!strcmp ("--cycles", argv[i])) {
      if (++i>=argc) usage ();
      TotalCycles = atoi(argv[i]);
      if (TotalCycles < 1) usage();
      continue;
    }

    usage();
  }

  if (framebuffer_open(NULL, NULL) == -1)
    {
      perror("framebuffer_open failed.");
      exit(0);
    }


  framebuffer_blit();

  framebuffer_close();

  return 0;
}
