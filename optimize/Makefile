CC = gcc
CFLAGS = -Wall -O2
LIBS = -lm

OBJS = driver.o kernels.o fcyc.o clock.o

all: driver

driver: $(OBJS) config.h defs.h fcyc.h
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o driver

clean: 
	-rm -f $(OBJS) driver core *~ *.o
	-rm -f *.s
	-rm -f *.image *.png

assem:
	$(CC) -Wall -Og -S kernels.c
