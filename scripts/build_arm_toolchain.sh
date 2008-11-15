#!/bin/sh
#
# Copyright (c) 2008 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.
#
# Build an ARM cross-compiler toolchain (including binutils, gcc and
# newlib) on autopilot.

ROOT=`pwd`
SRCDIR=$ROOT/src
BUILDDIR=$ROOT/build
PREFIX=$ROOT/install

GCC_URL=ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/releases/gcc-4.3.2/gcc-4.3.2.tar.bz2
GCC_VERSION=4.3.2
GCC_DIR=gcc-$GCC_VERSION

BINUTILS_URL=http://ftp.gnu.org/gnu/binutils/binutils-2.19.tar.bz2
BINUTILS_VERSION=2.19
BINUTILS_DIR=binutils-$BINUTILS_VERSION

NEWLIB_URL=ftp://sources.redhat.com/pub/newlib/newlib-1.16.0.tar.gz
NEWLIB_VERSION=1.16.0
NEWLIB_DIR=newlib-$NEWLIB_VERSION

echo "I will build an arm-elf cross-compiler:

  Prefix: $PREFIX
  Sources: $SRCDIR
  Build files: $BUILDDIR

Press ^C now if you do NOT want to do this."
read IGNORE

#
# Helper functions.
#
ensure_source()
{
    URL=$1
    FILE=$(basename $1)

    if [ ! -e $FILE ]; then
	wget -O$FILE $URL
    fi
}

unpack_source()
{
(
    cd $SRCDIR
    ARCHIVE_SUFFIX=${1##*.}
    if [ "$ARCHIVE_SUFFIX" = "gz" ]; then
      tar zxvf $1
    elif [ "$ARCHIVE_SUFFIX" = "bz2" ]; then
      tar jxvf $1
    else
      echo "Unknown archive format for $1"
      exit 1
    fi
)
}

# Create all the directories we need.
mkdir -p $SRCDIR $BUILDDIR $PREFIX

(
cd $SRCDIR

# First grab all the source files...
ensure_source $GCC_URL
ensure_source $BINUTILS_URL
ensure_source $NEWLIB_URL

# ... And unpack the sources.
unpack_source $(basename $GCC_URL)
unpack_source $(basename $BINUTILS_URL)
unpack_source $(basename $NEWLIB_URL)
)

# Set the PATH to include the binaries we're going to build.
OLD_PATH=$PATH
export PATH=$PREFIX/bin:$PATH

#
# Stage 1: Build binutils
#
(
(
# Binutils must first be patched to be buildable by gcc 4.3.
#
# I'm not sure of the exact resolution (looks like there is some
# debate on the GCC mailing list), but this workaround is fine for our
# uses, since ARM7 doesn't have these fancy registers anyway.
cd $SRCDIR/$BINUTILS_DIR

patch -p1 <<EOF
--- binutils-2.19/gas/config/tc-arm.c	2008-08-13 01:39:30.000000000 +0200
+++ binutils-2.19.fixed/gas/config/tc-arm.c	2008-11-15 06:00:18.000000000 +0100
@@ -3456,7 +3456,7 @@

       if (reg == FAIL)
 	{
-	  as_bad (_(reg_expected_msgs[REG_TYPE_MMXWR]));
+	  as_bad ("Expected register");
 	  goto error;
 	}

@@ -3470,7 +3470,7 @@
 	  hi_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWR);
 	  if (hi_reg == FAIL)
 	    {
-	      as_bad (_(reg_expected_msgs[REG_TYPE_MMXWR]));
+	      as_bad ("Expected register");
 	      goto error;
 	    }
 	  else if (reg >= hi_reg)
@@ -3588,7 +3588,7 @@

       if (reg == FAIL)
 	{
-	  as_bad (_(reg_expected_msgs[REG_TYPE_MMXWCG]));
+	  as_bad ("Expected register");
 	  goto error;
 	}

@@ -3603,7 +3603,7 @@
 	  hi_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWCG);
 	  if (hi_reg == FAIL)
 	    {
-	      as_bad (_(reg_expected_msgs[REG_TYPE_MMXWCG]));
+	      as_bad ("Expected register");
 	      goto error;
 	    }
 	  else if (reg >= hi_reg)
@@ -3709,7 +3709,7 @@
   reg = arm_reg_parse (&input_line_pointer, REG_TYPE_RN);
   if (reg == FAIL)
     {
-      as_bad (_(reg_expected_msgs[REG_TYPE_RN]));
+      as_bad ("Expected register");
       ignore_rest_of_line ();
       return;
     }
EOF
) || exit 1

# Build binutils.
mkdir -p $BUILDDIR/$BINUTILS_DIR
cd $BUILDDIR/$BINUTILS_DIR

$SRCDIR/$BINUTILS_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft \
    && make all install
) || exit 1

#
# Stage 2: Patch the GCC multilib rules, then build the gcc compiler only
#
(
MULTILIB_CONFIG=$SRCDIR/$GCC_DIR/gcc/config/arm/t-arm-elf

echo "

MULTILIB_OPTIONS += mno-thumb-interwork/mthumb-interwork
MULTILIB_DIRNAMES += normal interwork

" >> $MULTILIB_CONFIG

mkdir -p $BUILDDIR/$GCC_DIR
cd $BUILDDIR/$GCC_DIR

$SRCDIR/$GCC_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft \
    --enable-languages="c,c++" --with-newlib \
    --with-headers=$SRCDIR/$NEWLIB_DIR/newlib/libc/include \
    && make all-gcc install-gcc
) || exit 1

#
# Stage 3: Build and install newlib
#
(
mkdir -p $BUILDDIR/$NEWLIB_DIR
cd $BUILDDIR/$NEWLIB_DIR

$SRCDIR/$NEWLIB_DIR/configure --target=arm-elf --prefix=$PREFIX \
    --enable-interwork --enable-multilib --with-float=soft \
    && make all install
) || exit 1

#
# Stage 4: Build and install the rest of GCC.
#
(
cd $BUILDDIR/$GCC_DIR

make all install
) || exit 1

export PATH=$OLD_PATH

echo "
Build complete! Add $PREFIX/bin to your PATH to make arm-elf-gcc and friends
accessible directly.
"
