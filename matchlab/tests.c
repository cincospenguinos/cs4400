#include "matchlab.c"
#include <check.h>

START_TEST(test_match_a)
{
  ck_assert_int_eq(match_a("zzz336"), 1);
}
END_TEST
