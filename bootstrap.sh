#!/bin/sh
# generate fresh configure and Makefile.ins
# only developers might need to run this script

run() {
	sh -e -x -c "$@"
}

mkdir -p cfgaux

run "aclocal --warnings=all"

#workaround for Automake 1.5
if grep m4_regex aclocal.m4 >/dev/null; then
    perl -i.bak -p -e 's/m4_patsubst/m4_bpatsubst/g; s/m4_regexp/m4_bregexp/g;' aclocal.m4
fi

run "autoheader --warnings=all"
# remove package-specific stuff from config.h

# we need to run this manually because it does not create ltmain.sh when
# called from automake for some unknown reason (is it called at all?)
run "libtoolize --automake --force --copy"

run "automake --foreign --add-missing --copy --force-missing --no-force --warnings=all"

# cannot add --warnings=all until AX_CHECK_ZLIB is updated to avoid warnings
run "autoconf"

# leave grep output on the console to supply macro details.
if grep -E 'AX_CREATE_STDINT_H|AX_CHECK_ZLIB|AX_SAVE_FLAGS|AX_RESTORE_FLAGS' configure; then
	echo "Error: Macros from Autoconf Macro Archive not expanded."
	echo "       Do you need to install the Archive?"
	echo "       Moving configire script to configure.invalid for now."
	mv -i configure configure.invalid
	exit 1
fi

echo "done."
