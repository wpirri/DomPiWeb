#!/bin/sh
set -x
cat $1/conf/$3.tab | while read line
do
	line = `echo $line | sed "s/\r//g" | sed "s/\//g"`
	if [ "X${3}" = "Xserver" ]
	then
		grep "$line" $2/$3.tab >/dev/null 2>&1 || echo "$line$4" >> $2/$3.tab
	else
		grep "$line" $2/$3.tab >/dev/null 2>&1 || echo "$line" >> $2/$3.tab
	fi
done
