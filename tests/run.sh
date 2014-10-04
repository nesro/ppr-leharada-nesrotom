#!/bin/bash

BIN=./main
TEST_FAILS=0

for g in $(ls ./tests/*graph.txt); do

	test_no=$(echo $g | sed 's/[^0-9]*//g')
	diameter=$(cat ./tests/$test_no-info.txt | grep diameter | cut -d ' ' -f 2)

	echo -n "Test diameter for $test_no: "
	if $BIN $g --test-diameter $diameter; then
		echo "OK"
	else
		echo "FAIL"
		(( TEST_FAILS++ ))
	fi
done

