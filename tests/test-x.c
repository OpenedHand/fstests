/*
  test-x.c -- Measure fullscreen write speed under X. 
  Version 0.6

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

  gcc -Wall -O2 -L/usr/X11R6/lib -lX11 -lXext test-x.c -o test-x

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <X11/extensions/XShm.h>
#include <X11/Xmd.h> 

#define VERSION "0.6"

enum {
  MULTIBLIT_NONE = 0,
  MULTIBLIT_X,
  MULTIBLIT_Y,
};

static Display *dpy;
static int      scr;	
static Visual  *vis;
static Window   root, win;
static int      depth;
static GC       gc;

static Bool     have_shm;
static int      width, height;

static Bool     Verbose;
static int      WantMultiBlit = MULTIBLIT_NONE;
static int      TotalCycles = 100;

static unsigned long long
GetTimeInMillis(void)
{
  struct timeval  tp;

  gettimeofday(&tp, 0);
  return (unsigned long long)(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

static int
x_error_handler(Display     *display,
		XErrorEvent *e)
{
  char msg[255];

  XGetErrorText(display, e->error_code, msg, 255);
  fprintf(stderr, "test-x: X error (%#lx): %s (opcode: %i)\n",
	  e->resourceid, msg, e->request_code);
  fprintf(stderr, "test-x: Test Aborted!\n");

  exit(1);
}

static void
x_open(void)
{
  XGCValues gcv;
  Atom atoms_WINDOW_STATE, atoms_WINDOW_STATE_FULLSCREEN;

  /* Setup various X params */
  scr      = DefaultScreen(dpy);
  root     = DefaultRootWindow(dpy);
  depth    = DefaultDepth(dpy, scr);
  vis      = DefaultVisual(dpy, scr); 
  root     = RootWindow(dpy, scr);
  width    = DisplayWidth(dpy, scr);
  height   = DisplayHeight(dpy, scr);

  /*
  if (depth != 16)
    {
      fprintf(stderr, "Display depth is not 16bpp\n");
      exit(1);
    }
  */

  gcv.foreground = BlackPixel(dpy, scr);
  gcv.background = WhitePixel(dpy, scr);
  gcv.graphics_exposures = False;

  gc = XCreateGC( dpy, root, GCForeground | GCBackground | GCGraphicsExposures
		  , &gcv);

  /* Hints for a fullscreen window */
  atoms_WINDOW_STATE
    = XInternAtom(dpy, "_NET_WM_STATE",False);
  atoms_WINDOW_STATE_FULLSCREEN
    = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN",False);

  /* Create the Window */
  win = XCreateSimpleWindow(dpy, root, 0, 0,
			    width, height, 0,
			    BlackPixel(dpy, scr),
			    WhitePixel(dpy, scr));

  /* Set the hints needed */
  XChangeProperty(dpy, win, atoms_WINDOW_STATE, XA_ATOM, 32, 
		  PropModeReplace, 
		  (unsigned char *) &atoms_WINDOW_STATE_FULLSCREEN, 1);

  XMapWindow(dpy, win);

  XFlush(dpy);
}

static void
x_blit(void)
{
  int bitmap_pad;
  int x,y,i;

  XImage *ximg;   
  XShmSegmentInfo shminfo;
  Bool shm_success = False;
  unsigned long long start_clock, finish_clock, diff_clock; 
  int size, cycles = 0;

  /* Create the X Image */
  if (have_shm)
    {
      ximg = XShmCreateImage(dpy, vis, depth, 
			     ZPixmap, NULL, &shminfo,
			     width, height );
      
      shminfo.shmid=shmget(IPC_PRIVATE,
			   ximg->bytes_per_line * ximg->height,
			   IPC_CREAT|0777);
      shminfo.shmaddr = ximg->data = shmat(shminfo.shmid,0,0);
      
      if (ximg->data == (char *)-1)
	{
	  fprintf(stderr, "SHM can't attach SHM Segment for Shared XImage, falling back to XImages\n");
	  XDestroyImage(ximg);
	  shmctl(shminfo.shmid, IPC_RMID, 0);
	}
      else
	{
	  shminfo.readOnly=True;
	  XShmAttach(dpy, &shminfo);
	  shm_success = True;
	}
    }
  
  if (!shm_success)
    {
      bitmap_pad = 16;
      
      ximg = XCreateImage( dpy, vis, depth, 
			   ZPixmap, 0, 0,
			   width, height, bitmap_pad, 0);
      
      ximg->data = malloc( ximg->bytes_per_line * height );
    }

  size = ximg->bytes_per_line * ximg->height;

  start_clock = GetTimeInMillis(); 

  if (depth == 16)
    {
      for(y=0; y < height; y++)
	for(x=0; x < width; x++)
	  {
	    int b = 10;
	    int g = ( x - 100 ) / 6;   
	    int r = 31 - ( y - 100 ) / 16;
	
	    XPutPixel(ximg, x, y, (r<<11 | g << 5 | b));
	  }
    }
  else
    {
      for(y=0; y < height; y++)
	for(x=0; x < width; x++)
	  {
	    XPutPixel(ximg, x, y, rand() * 255);
	  }
    }

  finish_clock = GetTimeInMillis();

  if (Verbose)
    {
      printf("test-x: Surface ( %i X %i, %i KB ) Created in %lli ms\n",
	     width, height, size / 1024, finish_clock - start_clock);
    }

  if (WantMultiBlit == MULTIBLIT_Y
      && height < TotalCycles)
    {
      fprintf(stderr, 
	      "test-x: *warning* Reducing Total Blits to fit Display Size\n");
      TotalCycles = height;
    }

  if (WantMultiBlit == MULTIBLIT_X
      && width < TotalCycles)
    {
      fprintf(stderr, 
	      "test-x: *warning* Reducing Total Blits to fit Display Size\n");
      TotalCycles = width;
    }

  start_clock = GetTimeInMillis(); 

  if (!shm_success)
    {
      switch (WantMultiBlit)
	{
	case MULTIBLIT_NONE:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    XPutImage( dpy, win, gc, ximg, 0, 0, 0, 0, width, height);
	  break;
	case MULTIBLIT_Y:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    {
	      for (y = 0; y < height; y++) 
		{
		  i = y + cycles;
		  if (i >= height) i -= height;
		  XPutImage(dpy, win, gc, ximg, 0, y, 0, i, width, 1 );
		}
	    }
	  break;
	case MULTIBLIT_X:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    {
	      for (x = 0; x < width; x++) 
		{
		  i = x + cycles;
		  if (i >= width) i -= width;
		  XPutImage(dpy, win, gc, ximg, x, 0, i, 0, 1, height );
		}
	    }
	  break;
	}
    }
  else
    {
      switch (WantMultiBlit)
	{
	case MULTIBLIT_NONE:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    {
	      XShmPutImage( dpy, win, gc, ximg, 0, 0, 0, 0, width, height, 1);
	      XSync(dpy, False);
	    }
	      
	  break;
	case MULTIBLIT_Y:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    {
	      for (y = 0; y < height; y++) 
		{
		  i = y + cycles;
		  if (i >= height) i -= height;
		  XShmPutImage(dpy, win, gc, ximg, 0, y, 0, i, width, 1, 1);
		}

	      /*
	      XShmPutImage(dpy, win, gc, ximg, 0, 0, 0, cycles, 
			width, height-cycles, 1 );
	      XShmPutImage(dpy, win, gc, ximg, 0, height-cycles, 
			0, 0, width, cycles, 1 );
	      */

	      XSync(dpy, False);
	    }
	  break;
	case MULTIBLIT_X:
	  for (cycles=0; cycles < TotalCycles; cycles++)
	    {
	      for (x = 0; x < width; x++) 
		{
		  i = x + cycles;
		  if (i >= width) i -= width;
		  XShmPutImage(dpy, win, gc, ximg, x, 0, i, 0, 1, height, 1);
		}

	      /*
	      XShmPutImage(dpy, win, gc, ximg, 0, 0, cycles, 0,
			width-cycles, height, 1 );
	      XShmPutImage(dpy, win, gc, ximg, width-cycles, 0, 
			0, 0, cycles, height, 1 );
	      */

	      XSync(dpy, False);
	    }
	  break;
	}
    }

  finish_clock = GetTimeInMillis();
  diff_clock  = finish_clock - start_clock;

  printf("test-x: %s write speed: %lli KB/sec\n",
	 shm_success ? "X-SHM" : "Plain X",
	 ( (unsigned long long)(size/1024) * 1000 * cycles) / diff_clock);

  if (Verbose)
    {
      printf("test-x: Approx frame rate: %lli frames/sec\n",
	     (unsigned long long) cycles*1000 / diff_clock);
    }


  /* Clean up */
  if (!shm_success)
    {
      XDestroyImage (ximg);
    }
  else
    {
      XSync(dpy, False);
      XShmDetach(dpy, &shminfo);
      XDestroyImage (ximg);
      shmdt(shminfo.shmaddr);
      shmctl(shminfo.shmid, IPC_RMID, 0);
    }
  
  ximg = NULL;	
}

static void
x_close(void)
{
  XCloseDisplay(dpy);
}

static void
usage(void)
{
  fprintf(stderr, 
	  "test-x " VERSION "\n"
	  "usage: test-x [-display <X display>] [--verbose] [--no-shm] [--multiblit=<y|x>] [--cycles <int>]\n");
  exit(1);
} 

int 
main (int argc, char **argv)
{
  char *dpy_name = NULL;
  int i;

  have_shm = True;

  for (i = 1; i < argc; i++) {

    if (!strcmp ("-display", argv[i]) || !strcmp ("-d", argv[i])) {
      if (++i>=argc) usage ();
      dpy_name = argv[i++];
      continue;
    }

    if (!strcmp ("--verbose", argv[i]) || !strcmp ("-v", argv[i])) {
      Verbose = True;
      continue;
    }

    if (!strcmp ("--no-shm", argv[i]) ) {
      have_shm = False;
      continue;
    }

    if (!strcmp ("--multiblit=x", argv[i]) ) {
      WantMultiBlit = MULTIBLIT_X;
      continue;
    }

    if (!strcmp ("--multiblit=y", argv[i]) ) {
      WantMultiBlit = MULTIBLIT_Y;
      continue;
    }

    if (!strcmp ("--cycles", argv[i]))  {
      if (++i>=argc) usage ();
      TotalCycles = atoi(argv[i]);
      if (TotalCycles < 1) usage();
      continue;
    }

    usage();
  }

  if ((dpy = XOpenDisplay(dpy_name)) == NULL) {
    fprintf(stderr, "Cannot connect to X server on display %s.",
	    dpy_name);
    exit(1);
  }

  XSetErrorHandler(x_error_handler);

  x_open();

  x_blit();

  x_close();

  return 0;
}
