#!/bin/bash

TRACE_DIR=/home/zongxin/zong_trace_print

count=0
zong(){
    sudo rm -rf  $TRACE_DIR
    sudo lttng create zx --output=$TRACE_DIR
    sudo lttng enable-channel --kernel zong --subbuf-size=8M
    sudo lttng enable-event --kernel --channel=zong --all
    sudo lttng start

    out=$(sudo taskset -c 1-3 chrt -r 10 ./lal_cg_perf 100 1)
    echo "+++++++++++++++"$out

    sudo lttng destroy
    count=$(echo $out | grep -E '1[0-9]\.[0-9]{2}|[2-9][0-9]+\.[0-9]{2}')
}

while [ !$count ]
do
        zong
done