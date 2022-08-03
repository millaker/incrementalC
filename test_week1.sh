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

for i in week1/invalid/*.c
do
    ./test.out $i 2> /dev/null
    base=${i%.c}.s
    echo -n "$i: "
    flag=0
    for j in week1/invalid/*
    do
        if [ "$base" == "$j" ]
        then
            echo "FAIL"
            flag=1
            break
        fi
    done
    if [ $flag != 1 ]
    then
        echo "PASS"
    fi
done
