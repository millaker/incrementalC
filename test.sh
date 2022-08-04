#!/bin/bash
declare -i fail=0
declare -i pass=0
for i in tests/valid/*.c
do
    gcc $i
    ./a.out
    expected=$?
    ./test.out $i
    base=${i%.c}.s
    gcc $base
    ./a.out
    result=$?

    echo -n "$i:  "
    if [ "$expected" -ne "$result" ]
    then
        echo "FAIL"
        echo "Expect $expected, but got $result"
        fail=$((fail + 1))
    else
        echo "PASS"
        pass=$((pass + 1))
    fi
    rm $base
done
rm a.out

for i in tests/invalid/*.c
do
    ./test.out $i 2> /dev/null
    base=${i%.c}.s
    echo -n "$i:  "
    flag=0
    for j in week1/invalid/*
    do
        if [ "$base" == "$j" ]
        then
            echo "FAIL"
            flag=1
            fail=$((fail + 1))
            break
        fi
    done
    if [ $flag != 1 ]
    then
        echo "PASS"
        pass=$((pass + 1))
    fi
done

printf "\n-------------------------------------------------\n"
printf "Result: $pass passed, $fail failed\n"
