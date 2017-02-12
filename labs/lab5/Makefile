SRCS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

CFLAGS = -O2 -g

all: $(PROGS)

%: %.c

	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm -f *.o perf_tuning perf_tuning2 perf_tunic3
