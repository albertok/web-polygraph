#!/bin/sh

#
# tests whether the specified .pg files can be compiled with pgl_test
# pre-processes known .pg files to set user-specified parameters
# useful to check the effect of PGL changes on old files
#

pgl_test=../src/pgl/pgl_test
includes=include

files=$*
if test -z "$files"
then
	files=*.pg
fi


for i in $files
do
	re='s/nothing/nothing/';
	
	case $i in
		polymix-*) {
			re="s|peak_req_rate = .*;|peak_req_rate = 1000/sec;|;
				s|FillRate = .*;|FillRate = 50% * TheBench.peak_req_rate;|;
				s|ProxyCacheSize = .*;|ProxyCacheSize = 10GB;|;
				s|Resolver.servers = .*;|Resolver.servers = [ '127.0.0.1:53' ];|";
		};;
		webaxe-1*) {
			re='s|PeakRate = .*;|PeakRate = 1000/sec;|;
				s|FillRate = .*;|FillRate = 50% * PeakRate;|;
				s|CacheSize = .*;|CacheSize = 10GB;|;
				s|WSS = .*;|WSS = 1GB;|;
				';
		};;
		webaxe-[34]*) {
			re='s|peak_req_rate = .*;|peak_req_rate = 1000/sec;|;
				s|FillRate = .*;|FillRate = 50% * TheBench.peak_req_rate;|;
				s|CacheSize = .*;|CacheSize = 10GB;|;
				s|WSS = .*;|WSS = 1GB;|;
				';
		};;
	esac

	tmp=/tmp/pgtest-$i

	echo "$pgl_test $tmp $includes"

	if ! sed "$re" $i > $tmp
	then
		echo "error preprocessing $i ($tmp), exiting"
		exit 1
	fi

	if ! $pgl_test $tmp $includes > /dev/null
	then
		echo "error compiling $i ($tmp), exiting"
		exit 2
	fi

	#rm $tmp
done;
