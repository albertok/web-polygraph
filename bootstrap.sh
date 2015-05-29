#!/bin/sh
# generate fresh configure and Makefile.ins
# only developers might need to run this script

run() {
	sh -e -x -c "$@"
}

run aclocal

#workaround for Automake 1.5
if grep m4_regex aclocal.m4 >/dev/null; then
    perl -i.bak -p -e 's/m4_patsubst/m4_bpatsubst/g; s/m4_regexp/m4_bregexp/g;' aclocal.m4
fi

run autoheader
# remove package-specific stuff from config.h

# we need to run this manually because it does not create ltmain.sh when
# called from automake for some unknown reason (is it called at all?)
run "libtoolize --automake --force --copy"

run "automake --foreign --add-missing --copy --force-missing --no-force"
# --warnings all


run autoconf

echo "done."
