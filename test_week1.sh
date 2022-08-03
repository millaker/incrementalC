#!/bin/bash

for i in week1/valid/*.c
do
    gcc $i
    ./a.out
    expected=$?
    ./test.out $i
    base=${i%.c}.s
    gcc $base
    ./a.out
    result=$?

    echo -n "$i:"
    if [ "$expected" -ne "$result" ]
    then
        echo "FAIL"
        echo "Expect $expected, but got $result"
    else
        echo "PASS"
    fi
    rm $base
done
rm a.out
