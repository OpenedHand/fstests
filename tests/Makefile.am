AM_CFLAGS = @GCC_FLAGS@

noinst_PROGRAMS= test-fb           \
	      test-gdk          \
	      test-gdk-idle          \
	      test-gtk-layout   \
              test-pango-layout \
              test-x            \
              test-freetype     \
              test-pango        \
	      test-pango-gdk    \
	      test-pango-gtk    \
	      test-freetype-cached \
              test-xft

# fullscreen blit to framebuffer
test_fb_SOURCES = test-fb.c

# fullscreen blit to gtk drawing area via gtk pixbufs
test_gdk_SOURCES = test-gdk.c
test_gdk_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_gdk_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@

# fullscreen blit to gtk drawing area via gtk pixbufs in an idle handler
test_gdk_idle_SOURCES = test-gdk-idle.c
test_gdk_idle_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_gdk_idle_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@

# fullscreen blit to x window via shm
test_x_SOURCES = test-x.c
test_x_LDADD = @XLIBS_LIBS@
test_x_CFLAGS = @XLIBS_CFLAGS@

# freetype text tests
test_freetype_SOURCES = test-freetype.c
test_freetype_LDADD = @FREETYPE_LIBS@
test_freetype_CFLAGS = @FREETYPE_CFLAGS@

# as above but pregenrates glyphs
test_freetype_cached_SOURCES = test-freetype-cached.c
test_freetype_cached_LDADD = @FREETYPE_LIBS@
test_freetype_cached_CFLAGS = @FREETYPE_CFLAGS@

# draw strings via xft
test_xft_SOURCES = test-xft.c
test_xft_LDADD = @XLIBS_LIBS@ @PANGO_LIBS@
test_xft_CFLAGS = @XLIBS_CFLAGS@ @PANGO_CFLAGS@

# draws pango strings with just x
test_pango_SOURCES = test-pango.c
test_pango_LDADD = @PANGO_LIBS@
test_pango_CFLAGS = @PANGO_CFLAGS@

#below are as above but on 

# Draws a list of Pango glyphs onto a GtkWindow with pango_xft_render
test_pango_gdk_SOURCES = test-pango-gdk.c
test_pango_gdk_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_pango_gdk_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@

# Draws a list of Pango glyphs onto a GtkDrawingArea with pango_xft_render
test_pango_gtk_SOURCES = test-pango-gtk.c
test_pango_gtk_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_pango_gtk_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@

# Draws a list of Pango glyphs onto a GtkWindow with gdk_draw_glyphs
# can probably go
# test_gtk_SOURCES = test-gtk.c
# test_gtk_LDADD = @GTK_LIBS@ @PANGO_LIBS@
# test_gtk_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@


test_pango_layout_SOURCES = test-pango-layout.c
test_pango_layout_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_pango_layout_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@

# As above PangoLayout onto a GtkWindow with gdk_draw_layout XXX TOGO ?
test_gtk_layout_SOURCES = test-gtk-layout.c
test_gtk_layout_LDADD = @GTK_LIBS@ @PANGO_LIBS@
test_gtk_layout_CFLAGS = @GTK_CFLAGS@ @PANGO_CFLAGS@




# test_sdl_SOURCES = test-sdl.c

