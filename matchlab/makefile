CC=gcc
LOCAL_LIB=~/lib/usr/local

make: matchlab.c
	$(CC) -O2 -g -Wall matchlab.c

test: matchlab.c match_tests.c
	$(CC) -o -Wall tests tests.c
