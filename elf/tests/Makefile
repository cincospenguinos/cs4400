
# Your `inspect` is likely to produce exactly the same output
# order as the ".expect" files, since there's a natural order
# to traversing sections and machine code. Use the first `DIFF`
# definition if so. If you need the more flexible comparison
# allowed by the assignment spec, use the second `DIFF`,
# instead.

DIFF = diff
# DIFF = racket diff.rkt

inspect: inspect.c
	gcc -Wall -O2 -o inspect inspect.c

TEST_INPUTS =   \
  f_uses_a.so   \
  fg_use_ab.so  \
  f_uses_a_g_uses_b.so \
  f_calls_e.so  \
  f_uses_a_calls_e.so \
  f_uses_a_via_e.so \
  f_calls_g_uses_a.so \
  many_uses.so

demo: inspect
	gcc -O2 -fPIC -c demo.c
	gcc -shared -o demo.so demo.o
	./inspect demo.so

test: inspect $(TEST_INPUTS)
	$(MAKE) generate-output
	$(MAKE) check-output

test-level-1: inspect $(TEST_INPUTS)
	$(MAKE) generate-output
	$(MAKE) check-output CHECK_LEVEL=-1

test-level-2: inspect $(TEST_INPUTS)
	$(MAKE) generate-output
	$(MAKE) check-output CHECK_LEVEL=-2

generate-output:
	for i in $(TEST_INPUTS) ; do  \
          ./inspect $$i > $$i.out || exit 1; \
        done

check-output:
	for i in $(TEST_INPUTS) ; do  \
          $(DIFF) -q $$i.expect$(CHECK_LEVEL) $$i.out || exit 1; \
        done

%.so: %.c
	gcc -shared -O2 -fPIC -o $@ $<

clean:
	rm -f inspect *.so *.out
