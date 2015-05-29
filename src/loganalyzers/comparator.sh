
delta=$1
if test -z "$delta"
then
	delta=0
fi

set -e -x

#rm /tmp/polyrep/test[12]/*html
#./reporter --label test1 /tmp/t[cs]1.log
#./reporter --label test2 /tmp/t[cs]2.log

cat /tmp/polyrep/test1/*html > /tmp/t1.html
cat /tmp/polyrep/test2/*html > /tmp/t2.html

cp comparator.css /tmp/t.html
./comparator -delta $delta /tmp/t{1,2,2,1}.html >> /tmp/t.html

set +x
