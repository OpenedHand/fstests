#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/time.h>

#include <pango/pango.h>

/* 

gcc test-gdk.c -o test-gdk `pkg-config --libs --cflags gtk+-2.0`

 */

#define VERSION "0.1"

#define DEFAULT_TEXT_STR "abcdefghijklmnopqrstuvwxyz"
#define DEFAULT_FONT     "sans serif 18"
#define DEFAULT_N_LINES  20
#define DEFAULT_N_CYCLES 100

static gchar *TextStr     = DEFAULT_TEXT_STR;
static gchar *TextFont    = DEFAULT_FONT;
static gint   TextNLines  = DEFAULT_N_LINES;
static gint   TotalCycles = 100;

static gboolean Verbose;

static unsigned long long
GetTimeInMillis(void)
{
  struct timeval  tp;

  gettimeofday(&tp, 0);
  return (unsigned long long)(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}


gboolean
nullexpose(GtkWidget      *darea,
       GdkEventExpose *event,
       gpointer        user_data)
{
  printf("null expose\n");
  return FALSE;

}

gboolean
expose(GtkWidget      *darea,
       GdkEventExpose *event,
       gpointer        user_data)
{
  int                i, j, y = 10, nchars;
  unsigned long long start_clock, finish_clock, diff_clock; 

  PangoFontDescription *fontdes;
  PangoLayout          *layout;
  GdkGC                *gc;

  gc = gdk_gc_new(darea->window);

  start_clock = GetTimeInMillis(); 
 
  if ((fontdes = pango_font_description_from_string(TextFont)) == NULL)
    {
      fprintf(stderr, "Failed to load font '%s', exiting.", TextFont);
      exit(-1);
    }

  gtk_widget_modify_font(darea, fontdes);

  layout = gtk_widget_create_pango_layout(darea, 0);

  finish_clock = GetTimeInMillis();

  if (Verbose)
    {
      printf("test-pango-layout: Font loaded, drawable + color created in %lli ms\n",
	     finish_clock - start_clock);
    }

  start_clock = GetTimeInMillis(); 

  for (j=0; j<TotalCycles; j++)
    {
      y = 0;
      for (i=0; i<TextNLines; i++)
	{
	  pango_layout_set_text (layout, TextStr, -1);

	  gdk_draw_layout (darea->window, gc, 0, y, layout);

	  y += 20; 	     /* XXX should be font size though not critical */
	}
    }

  finish_clock = GetTimeInMillis();

  diff_clock  = finish_clock - start_clock;

  nchars = strlen(TextStr) * TextNLines * TotalCycles;

  printf("test-pango-layout: Total time %lli ms, %i glyphs rendered = approx %lli glyphs per second\n",
	 diff_clock, nchars, ( 1000 * nchars ) / diff_clock);

  gtk_main_quit();

  return FALSE;
}


static void
usage(void)
{
  fprintf(stderr, 
	  "test-pango " VERSION "\n"
	  "usage: test-pango [options..]\n"
          "Options are;\n"
          "-display <X display>\n"
	  "--verbose\n"
	  "--text-str <str> text to render ( defaults to alphabet )\n"
	  "--font <str> Xft font to use ( defaults to " DEFAULT_FONT ")\n"
	  "--nlines <int> Number of lines to draw per cycle\n"
	  "--cycles <int>  number of times to runs the test ( default 100)\n"
	  );


  exit(1);
} 

int 
main(int argc, char **argv)
{
  GtkWidget *window = 0;
  GtkWidget *darea  = 0;
  
  int i;
  
  gtk_init(&argc, &argv);

  for (i = 1; i < argc; i++) {

    if (!strcmp ("--verbose", argv[i]) || !strcmp ("-v", argv[i])) {
      Verbose = TRUE;
      continue;
    }

    if (!strcmp ("--text-str", argv[i]) ) {
      if (++i>=argc) usage ();
      TextStr = argv[i];
      continue;
    }

    if (!strcmp ("--font", argv[i]) ) {
      if (++i>=argc) usage ();
      TextFont = argv[i];
      continue;
    }

    if (!strcmp ("--nlines", argv[i]) ) {
      if (++i>=argc) usage ();
      TextNLines = atoi(argv[i]);
      if (TextNLines < 1) usage();
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

  // gtk_window_maximize(GTK_WINDOW(window)); /* fullscreen  */

  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  darea = gtk_drawing_area_new();

  gtk_widget_set_double_buffered (darea, FALSE);  

  gtk_container_add(GTK_CONTAINER(window), darea);

  g_signal_connect(G_OBJECT(darea), "expose_event",
		   G_CALLBACK(expose), 0);

  gtk_widget_show_all(window);

  gtk_main();  

  return 0;
}


