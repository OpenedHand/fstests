#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/time.h>

/* 

gcc test-gdk-idle.c -o test-gdk-idle `pkg-config --libs --cflags gtk+-2.0`

 */

#define VERSION "0.1"

static int      TotalCycles = 100;
static gboolean Verbose;
static GdkPixbuf *pixbuf;

static unsigned long long
GetTimeInMillis(void)
{
  struct timeval  tp;

  gettimeofday(&tp, 0);
  return (unsigned long long)(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

static gboolean
expose(GtkWidget      *darea,
       GdkEventExpose *event,
       gpointer        user_data)
{
  int                width, height;

  width  = darea->allocation.width;
  height = darea->allocation.height;

  gdk_draw_pixbuf (GDK_DRAWABLE(darea->window),
                   NULL,
                   pixbuf,
                   0, 0, 0, 0,
                   -1,
                   -1,
                   GDK_RGB_DITHER_NONE, 0, 0);
  
  // gtk_widget_queue_draw_area(darea, 0, 0, width, height);
  
  /* Assume this flushes ? */
  gdk_display_flush(gdk_display_get_default());

  return TRUE;
}

static gboolean
idle_cb (GtkWidget *w)
{
  static int cycles = -1;
  static unsigned long long start_clock = 0, finish_clock = 0, diff_clock;
  GdkRectangle rect;

  if (cycles == -1) cycles = TotalCycles;

  if (cycles-- == 0) {
    int size;
    finish_clock = GetTimeInMillis();
    diff_clock  = finish_clock - start_clock;

    size = w->allocation.width * w->allocation.height * 2; 	/* XXX Assumes 16bpp */
    
    printf("test-gdk-idle: write speed: %lli KB/sec\n",
           ( (unsigned long long)(size/1024) * 1000 * TotalCycles) / diff_clock);
    gtk_main_quit();
  }

  if (start_clock == 0) {
    fprintf(stderr, "Getting start time\n");
    start_clock = GetTimeInMillis();
  }

  rect.x = 0;
  rect.y = 0;
  rect.width = w->allocation.width;
  rect.height = w->allocation.height;

  gdk_window_invalidate_rect (w->window, &rect, TRUE);
  gdk_window_process_updates (w->window, TRUE);

  return TRUE;
}

static GdkPixbuf*
make_surface(GtkWidget *w)
{
  int                x, y, width, height, rowstride, n_channels = 3;
  unsigned long long start_clock, finish_clock;
  guchar            *pixels, *p;
  GdkPixbuf *pixbuf;

  width  = w->allocation.width;
  height = w->allocation.height;

  start_clock = GetTimeInMillis(); 
  
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels    = gdk_pixbuf_get_pixels (pixbuf);

  for(y=0; y < height; y++)
    for(x=0; x < width; x++)
      {
	guchar b = 10 + x ;
	guchar g = y;   
	guchar r = x - y;
	
	p = pixels + y * rowstride + x * n_channels;

	p[0] = b; p[1] = g; p[2] = r;
      }

  finish_clock = GetTimeInMillis();

  if (Verbose)
    {
      printf("test-gdk-idle: Surface ( %u X %u, %lu KB ) Created in %lli ms\n",
	     width, height, width * height * 2 / 1024, finish_clock - start_clock);
    }
  return pixbuf;
}

static void
usage(void)
{
  fprintf(stderr, 
	  "test-gdk-idle " VERSION "\n"
	  "usage: test-gdk-idle [--verbose] [--buffered] [--cycles <int>]\n");
  exit(1);
} 


int 
main(int argc, char **argv)
{
  gboolean Buffered = FALSE;
  GtkWidget *window = 0;
  GtkWidget *darea  = 0;
  
  int i;
  
  gtk_init(&argc, &argv);

  for (i = 1; i < argc; i++) {

    if (!strcmp ("--verbose", argv[i]) || !strcmp ("-v", argv[i])) {
      Verbose = TRUE;
      continue;
    }

    if (!strcmp ("--buffered", argv[i]) || !strcmp ("-b", argv[i])) {
      Buffered = TRUE;
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
  

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW(window),
			      gdk_screen_get_width(gdk_display_get_default_screen (gdk_display_get_default ())),
			      gdk_screen_get_height(gdk_display_get_default_screen (gdk_display_get_default ())));
  
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);
  
  
  darea = gtk_drawing_area_new();
  
  gtk_widget_set_double_buffered (darea, Buffered);

  gtk_container_add(GTK_CONTAINER(window), darea);

  g_signal_connect(G_OBJECT(darea), "expose_event",
		   G_CALLBACK(expose), 0);

  g_idle_add ((GSourceFunc)idle_cb, darea);

  gtk_widget_show_all(window);

  gtk_widget_map (window);
  pixbuf = make_surface (darea);

  gtk_main();  

  return 0;
}


