AM_CFLAGS = @GCC_FLAGS@ 

bin_PROGRAMS= test-fb           \
	      test-gdk          \
              test-pango-layout \
              test-x            \
              test-freetype     \
              test-pango        \
              test-xft


test_fb_SOURCES = test-fb.c

test_gdk_SOURCES = test-gdk.c
test_gdk_LDADD = @GTK_LIBS@
test_gdk_CFLAGS = @GTK_CFLAGS@

test_pango_layout_SOURCES = test-pango-layout.c
test_pango_layout_LDADD = @GTK_LIBS@
test_pango_layout_CFLAGS = @GTK_CFLAGS@

test_x_SOURCES = test-x.c
test_x_LDADD = @XLIBS_LIBS@
test_x_CFLAGS = @XLIBS_CFLAGS@

test_freetype_SOURCES = test-freetype.c
test_freetype_LDADD = @FREETYPE_LIBS@
test_freetype_CFLAGS = @FREETYPE_CFLAGS@

test_pango_SOURCES = test-pango.c
test_pango_LDADD = @PANGO_LIBS@
test_pango_CFLAGS = @PANGO_CFLAGS@

# test_sdl_SOURCES = test-sdl.c

test_xft_SOURCES = test-xft.c
test_xft_LDADD = @XLIBS_LIBS@
test_xft_CFLAGS = @XLIBS_CFLAGS@
