#!/bin/bash

cd build/

gcc -g -o mapreduce.o ../mapreduce.c -Wall -Werror -pthread -O -c
gcc -g -o wordcount.o ../wordcount.c -Wall -Werror -pthread -O -c
gcc -g -o queue.o ../queue.c -Wall -Werror -pthread -O -c

cd ../

gcc -g -o wordcount build/mapreduce.o build/wordcount.o build/queue.o -Wall -Werror -pthread -O
