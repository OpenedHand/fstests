/*
  test-sdl.c -- Measure fullscreen SDL write speed. 
  Version 0.4

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

  gcc `sdl-config --cflags` `sdl-config --libs`  test-sdl.c -o test-sdl

*/

#include "SDL.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define VERSION "0.4"

#define WIDTH 1152
#define HEIGHT 768

enum {
  MULTIBLIT_NONE = 0,
  MULTIBLIT_X,
  MULTIBLIT_Y,
};


static unsigned long long
GetTimeInMillis(void)
{
  struct timeval  tp;

  gettimeofday(&tp, 0);
  return (unsigned long long)(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

static void
usage(void)
{
  fprintf(stderr, 
	  "test-sdl " VERSION "\n"
	  "usage: test-sdl [-w <win width>] [-h <win height>] [--hwsurface] [--fullscreen] [--cycles <int>]\n");
  exit(1);
} 

int main(int argc, char **argv) 
{
  int WantMultiBlit = MULTIBLIT_NONE;
  SDL_VideoInfo *vid_info;
  SDL_Surface *screen, *buffer;
  Uint32 color;
  Uint16 *bufp;
  SDL_Rect dest, rect_part_src, rect_part_dest;
  int x,y,i;
  int cycles, TotalCycles = 100;
  unsigned long long start_clock, finish_clock, diff_clock; 

  int width  = WIDTH;
  int height = HEIGHT;

  int Verbose = 0;
  int  SurfaceType = SDL_SWSURFACE;

  for (i = 1; i < argc; i++) {

    if (!strcmp ("--width", argv[i]) || !strcmp ("-w", argv[i])) {
      if (++i>=argc) usage ();
      width = atoi(argv[i]);
      if (width < 1) usage();
      continue;
    }

    if (!strcmp ("--height", argv[i]) || !strcmp ("-h", argv[i])) {
      if (++i>=argc) usage ();
      height = atoi(argv[i]);
      if (height < 1) usage();
      continue;
    }

    if (!strcmp ("--cycles", argv[i]))  {
      if (++i>=argc) usage ();
      TotalCycles = atoi(argv[i]);
      if (TotalCycles < 1) usage();
      continue;
    }

    if (!strcmp ("--fullscreen", argv[i]) ) {
      SurfaceType = SDL_FULLSCREEN;
      continue;
    }

    if (!strcmp ("--hwsurface", argv[i]) ) {
      SurfaceType = SDL_HWSURFACE;
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

    if (!strcmp ("--verbose", argv[i]) || !strcmp ("-v", argv[i])) {
      Verbose = 1;
      continue;
    }

    usage();
  }

  if((SDL_Init(SDL_INIT_VIDEO)==-1)) { 
    printf("Could not initialize SDL: %s.\n", SDL_GetError());
    exit(-1);
  }

  SDL_GetVideoInfo();

  /* XXX FULLSCREEN */
  screen = SDL_SetVideoMode(width, height, 16, SurfaceType);

  buffer = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 16,
				screen->format->Rmask, screen->format->Gmask,
				screen->format->Bmask, screen->format->Amask);

  start_clock = GetTimeInMillis(); 
    
  for (x=0; x < width; x++)
    for (y=0; y < height; y++)
      {
	int b = (x + y) % 255;
	int g = x % 255;   
	int r = y % 255;

	color = SDL_MapRGB(buffer->format, r, g, b);
	bufp = (Uint16 *)buffer->pixels + y*buffer->pitch/2 + x;
	*bufp = color;
      }

  finish_clock = GetTimeInMillis();

  if (Verbose)
    {
      printf("test-sdl: Surface ( %i X %i, %i KB ) Created in %lli ms\n",
	     width, height, (width*height*2) / 1024, finish_clock - start_clock);
    }


  dest.x = 0;
  dest.y = 0;
  dest.w = buffer->w;
  dest.h = buffer->h;

  buffer = SDL_DisplayFormat(buffer);

  start_clock = GetTimeInMillis(); 

  switch (WantMultiBlit)
    {
    case MULTIBLIT_NONE:
      for (cycles=0; cycles < TotalCycles; cycles++)
	{
	  SDL_BlitSurface(buffer, NULL, screen, &dest);
	  SDL_UpdateRects(screen, 1, &dest);
	}
      break;
    case MULTIBLIT_Y:
      for (cycles=0; cycles < TotalCycles; cycles++)
      {
	rect_part_dest.x = rect_part_src.x = 0;
	rect_part_dest.w = rect_part_src.w = width;
	rect_part_dest.h = rect_part_src.h = 1;
	for (y = 0; y < height; y++) 
	{
	  i = y + cycles;
	  if (i >= height) i -= height;
	  rect_part_src.y = y; 
	  rect_part_dest.y = i; 
	  SDL_BlitSurface(buffer, &rect_part_src, screen, &rect_part_dest);
	}
	SDL_UpdateRects(screen, 1, &dest);
      }
      break;
    case MULTIBLIT_X:
      for (cycles=0; cycles < TotalCycles; cycles++)
      {
	rect_part_dest.y = rect_part_src.y = 0; 
	rect_part_dest.w = rect_part_src.w = 1;
	rect_part_dest.h = rect_part_src.h = height;
	for (x = 0; x < width; x++) 
	{
	  i = x + cycles;
	  if (i >= width) i -= width;
	  rect_part_src.x = x;
	  rect_part_dest.x = i;
	  SDL_BlitSurface(buffer, &rect_part_src, screen, &rect_part_dest);
	}
	SDL_UpdateRects(screen, 1, &dest);
      }
      break;
    }
  

  finish_clock = GetTimeInMillis();
  diff_clock  = finish_clock - start_clock;

  printf("test-sdl: write speed: %lli KB/sec\n",
	 ( (unsigned long long)((width*height*2)/1024) * 1000 * i) / diff_clock);

  if (Verbose)
    {
      printf("test-sdl: Approx frame rate: %lli frames/sec\n",
	     (unsigned long long) cycles*1000 / diff_clock);
    }


  SDL_Quit();
    
  exit(0);
}
