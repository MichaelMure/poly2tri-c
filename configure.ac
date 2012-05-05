# Declare the package details
AC_INIT([poly2tri-c], [0.9], [lightningismyname@gmail.com])

# Output the auxilary build scripts to a subdirectory
AC_CONFIG_AUX_DIR([build-aux])

# Relax some of the GNU requirements
# Initialize Automake with the following settings:
# Report all potential warnings and report all warnings as errors
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Find a C compiler
AC_PROG_CC

# Find a library make
AC_PROG_RANLIB

# Find GLib support via pkg-config
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.28])

CFLAGS="$CFLAGS $GLIB_CFLAGS"
LDFLAGS="$LDFLAGS $GLIB_LIBS"
LIBS="$LIBS $GLIB_LIBS"

# Output this configuration header file
AC_CONFIG_HEADERS([config.h])

# Output these files
AC_CONFIG_FILES([
	bin/Makefile		\
	p2t/sweep/Makefile	\
	p2t/common/Makefile	\
	p2t/Makefile		\
	render/Makefile		\
	refine/Makefile		\
	Makefile		\
	])

# Now really output
AC_OUTPUT
