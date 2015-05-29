#!/bin/sh

config=$1
if test -z "$config"
then
	config='workloads/simple.pg';
else
	shift
fi

set -x
#strace -s 80 -o srv.strace \
./src/server/server \
	--config $config \
	--cfg_dirs workloads/include \
	--verb_lvl 10 \
	--log /tmp/ts.log \
	--console /tmp/ts.con $* &
sleep 3

#strace -s 80 -o clt.strace \
./src/client/client \
	--config $config \
	--cfg_dirs workloads/include \
	--verb_lvl 10 \
	--log /tmp/tc.log \
	--console /tmp/tc.con $* &
sleep 3

set +x
a='';
while tail /tmp/ts.con /tmp/tc.con && test -z "$a"
do
	echo -n "<enter> to continue, <nonempty string> to stop: "
	read a
done
set -x

killall -INT client server
sleep 10;

tail /tmp/t[sc].con

./src/logextractors/lx /tmp/ts.log | fgrep rate | paste - -
./src/logextractors/lx /tmp/tc.log | fgrep rate | paste - -

set +x
