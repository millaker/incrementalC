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

    if [ "$expected" -ne "$result" ]
    then
        printf "\033[93;1;m$i\n"
        printf "Result: "
        printf "\033[31;1;mFAIL\n"
        printf "Expect $expected, but got $result\033[0m\n"
        fail=$((fail + 1))
    else
        printf "$i\n"
        printf "Result: "
        printf "\033[92;1;mPASS\033[0m\n"
        pass=$((pass + 1))
    fi
    rm $base
done
rm a.out

for i in tests/invalid/*.c
do
    ./test.out $i 2> /dev/null
    base=${i%.c}.s
    flag=0
    for j in week1/invalid/*
    do
        if [ "$base" == "$j" ]
        then
            printf "\033[93;1;m$i\n"
            printf "Result: "
            printf "\033[31;1;mFAIL\033[0m\n"
            flag=1
            fail=$((fail + 1))
            break
        fi
    done
    if [ $flag != 1 ]
    then
        printf "$i\n"
        printf "Result: "
        printf "\033[92;1;mPASS\033[0m\n"
        pass=$((pass + 1))
    fi
done

printf "Final Result: $pass passed, $fail failed\n"
