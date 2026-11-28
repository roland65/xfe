#! /bin/sh
set -e

# Regeneration of autoconf / automake files

rm -f config.cache
rm -f config.log
# Use the bundled gettext macros that match po/Makefile.in.in.
aclocal -I m4
autoheader
autoconf
automake -a
