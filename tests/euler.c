/**
 * Project Euler solutions — stress-testing the Luby runtime with real
 * algorithms.  Each problem is solved in pure Luby and the C harness
 * asserts the known-correct answer.
 */
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

static int assert_int(const char *label, luby_value v, int64_t expected) {
    if (v.type != LUBY_T_INT || v.as.i != expected) {
        printf("FAIL: %s (got type=%d val=%lld, expected %lld)\n",
               label, v.type, (long long)v.as.i, (long long)expected);
        return 0;
    }
    return 1;
}

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<euler>", out);
    if (rc != 0) {
        char buf[512];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s — runtime error: %s\n", label, buf);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Problem 1 — Multiples of 3 or 5                                    */
/*                                                                     */
/* Find the sum of all the multiples of 3 or 5 below 1000.            */
/* Answer: 233168                                                      */
/* ------------------------------------------------------------------ */

static const char *euler1 =
    "sum = 0\n"
    "i = 1\n"
    "while i < 1000\n"
    "  if i % 3 == 0 || i % 5 == 0\n"
    "    sum = sum + i\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "sum\n";

/* ------------------------------------------------------------------ */
/* Problem 2 — Even Fibonacci numbers                                  */
/*                                                                     */
/* Sum of even-valued Fibonacci terms ≤ 4 000 000.                    */
/* Answer: 4613732                                                     */
/* ------------------------------------------------------------------ */

static const char *euler2 =
    "a = 1\n"
    "b = 2\n"
    "sum = 0\n"
    "while a <= 4000000\n"
    "  if a % 2 == 0\n"
    "    sum = sum + a\n"
    "  end\n"
    "  t = a + b\n"
    "  a = b\n"
    "  b = t\n"
    "end\n"
    "sum\n";

/* ------------------------------------------------------------------ */
/* Problem 3 — Largest prime factor                                    */
/*                                                                     */
/* Largest prime factor of 600851475143.                               */
/* Answer: 6857                                                        */
/*                                                                     */
/* NOTE: 600851475143 requires 64-bit integers.  Luby uses int64_t    */
/* internally, so this is a great stress test.                         */
/* ------------------------------------------------------------------ */

static const char *euler3 =
    "n = 600851475143\n"
    "d = 2\n"
    "while d * d <= n\n"
    "  while n % d == 0\n"
    "    n = n / d\n"
    "  end\n"
    "  d = d + 1\n"
    "end\n"
    "n\n";

/* ------------------------------------------------------------------ */
/* Problem 4 — Largest palindrome product                              */
/*                                                                     */
/* Largest palindrome from the product of two 3-digit numbers.         */
/* Answer: 906609                                                      */
/* ------------------------------------------------------------------ */

static const char *euler4 =
    "def reverse_num(n)\n"
    "  rev = 0\n"
    "  while n > 0\n"
    "    rev = rev * 10 + n % 10\n"
    "    n = n / 10\n"
    "  end\n"
    "  rev\n"
    "end\n"
    "\n"
    "def is_palindrome(n)\n"
    "  n == reverse_num(n)\n"
    "end\n"
    "\n"
    "best = 0\n"
    "i = 999\n"
    "while i >= 100\n"
    "  j = i\n"
    "  while j >= 100\n"
    "    p = i * j\n"
    "    if p > best && is_palindrome(p)\n"
    "      best = p\n"
    "    end\n"
    "    j = j - 1\n"
    "  end\n"
    "  i = i - 1\n"
    "end\n"
    "best\n";

/* ------------------------------------------------------------------ */
/* Problem 5 — Smallest multiple                                       */
/*                                                                     */
/* Smallest positive number evenly divisible by all 1..20.             */
/* Answer: 232792560                                                   */
/* ------------------------------------------------------------------ */

static const char *euler5 =
    "def gcd(a, b)\n"
    "  while b > 0\n"
    "    t = b\n"
    "    b = a % b\n"
    "    a = t\n"
    "  end\n"
    "  a\n"
    "end\n"
    "\n"
    "def lcm(a, b)\n"
    "  a / gcd(a, b) * b\n"
    "end\n"
    "\n"
    "result = 1\n"
    "i = 2\n"
    "while i <= 20\n"
    "  result = lcm(result, i)\n"
    "  i = i + 1\n"
    "end\n"
    "result\n";

/* ------------------------------------------------------------------ */
/* Problem 6 — Sum square difference                                   */
/*                                                                     */
/* Difference between (sum 1..100)^2 and sum of (1..100)^2.           */
/* Answer: 25164150                                                    */
/* ------------------------------------------------------------------ */

static const char *euler6 =
    "sum_sq = 0\n"
    "sq_sum = 0\n"
    "i = 1\n"
    "while i <= 100\n"
    "  sum_sq = sum_sq + i * i\n"
    "  sq_sum = sq_sum + i\n"
    "  i = i + 1\n"
    "end\n"
    "sq_sum * sq_sum - sum_sq\n";

/* ------------------------------------------------------------------ */
/* Problem 7 — 10001st prime                                           */
/*                                                                     */
/* Answer: 104743                                                      */
/* ------------------------------------------------------------------ */

static const char *euler7 =
    "def is_prime(n)\n"
    "  if n < 2\n"
    "    return false\n"
    "  end\n"
    "  if n < 4\n"
    "    return true\n"
    "  end\n"
    "  if n % 2 == 0 || n % 3 == 0\n"
    "    return false\n"
    "  end\n"
    "  d = 5\n"
    "  while d * d <= n\n"
    "    if n % d == 0 || n % (d + 2) == 0\n"
    "      return false\n"
    "    end\n"
    "    d = d + 6\n"
    "  end\n"
    "  true\n"
    "end\n"
    "\n"
    "count = 0\n"
    "num = 1\n"
    "while count < 10001\n"
    "  num = num + 1\n"
    "  if is_prime(num)\n"
    "    count = count + 1\n"
    "  end\n"
    "end\n"
    "num\n";

/* ------------------------------------------------------------------ */
/* Problem 8 — Largest product in a series                             */
/*                                                                     */
/* Find the thirteen adjacent digits in the 1000-digit number that     */
/* have the greatest product.                                          */
/* Answer: 23514624000                                                 */
/*                                                                     */
/* Uses string indexing (s[i]) and to_i() to convert digit chars.      */
/* ------------------------------------------------------------------ */

static const char *euler8 =
    "s = \"73167176531330624919225119674426574742355349194934\"\n"
    "s = s + \"96983520312774506326239578318016984801869478851843\"\n"
    "s = s + \"85861560789112949495459501737958331952853208805511\"\n"
    "s = s + \"12540698747158523863050715693290963295227443043557\"\n"
    "s = s + \"66896648950445244523161731856403098711121722383113\"\n"
    "s = s + \"62229893423380308135336276614282806444486645238749\"\n"
    "s = s + \"30358907296290491560440772390713810515859307960866\"\n"
    "s = s + \"70172427121883998797908792274921901699720888093776\"\n"
    "s = s + \"65727333001053367881220235421809751254540594752243\"\n"
    "s = s + \"52584907711670556013604839586446706324415722155397\"\n"
    "s = s + \"53697817977846174064955149290862569321978468622482\"\n"
    "s = s + \"83972241375657056057490261407972968652414535100474\"\n"
    "s = s + \"82166370484403199890008895243450658541227588666881\"\n"
    "s = s + \"16427171479924442928230863465674813919123162824586\"\n"
    "s = s + \"17866458359124566529476545682848912883142607690042\"\n"
    "s = s + \"24219022671055626321111109370544217506941658960408\"\n"
    "s = s + \"07198403850962455444362981230987879927244284909188\"\n"
    "s = s + \"84580156166097919133875499200524063689912560717606\"\n"
    "s = s + \"05886116467109405077541002256983155200055935729725\"\n"
    "s = s + \"71636269561882670428252483600823257530420752963450\"\n"
    "\n"
    "best = 0\n"
    "total = len(s)\n"
    "i = 0\n"
    "while i <= total - 13\n"
    "  product = 1\n"
    "  j = 0\n"
    "  while j < 13\n"
    "    product = product * to_i(s[i + j])\n"
    "    j = j + 1\n"
    "  end\n"
    "  if product > best\n"
    "    best = product\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "best\n";

/* ------------------------------------------------------------------ */
/* Problem 9 — Special Pythagorean triplet                             */
/*                                                                     */
/* a + b + c = 1000, a^2 + b^2 = c^2.  Find abc.                     */
/* Answer: 31875000                                                    */
/* ------------------------------------------------------------------ */

static const char *euler9 =
    "result = 0\n"
    "a = 1\n"
    "while a < 1000\n"
    "  b = a + 1\n"
    "  while b < 1000 - a\n"
    "    c = 1000 - a - b\n"
    "    if a * a + b * b == c * c\n"
    "      result = a * b * c\n"
    "    end\n"
    "    b = b + 1\n"
    "  end\n"
    "  a = a + 1\n"
    "end\n"
    "result\n";

/* ------------------------------------------------------------------ */
/* Problem 10 — Summation of primes                                    */
/*                                                                     */
/* Sum of all primes below 2 000 000.                                  */
/* Answer: 142913828922                                                */
/*                                                                     */
/* Uses a sieve of Eratosthenes — tests large array allocation and    */
/* heavy iteration.                                                    */
/* ------------------------------------------------------------------ */

static const char *euler10 =
    "limit = 2000000\n"
    "sieve = []\n"
    "i = 0\n"
    "while i < limit\n"
    "  sieve[i] = 1\n"
    "  i = i + 1\n"
    "end\n"
    "sieve[0] = 0\n"
    "sieve[1] = 0\n"
    "p = 2\n"
    "while p * p < limit\n"
    "  if sieve[p] == 1\n"
    "    j = p * p\n"
    "    while j < limit\n"
    "      sieve[j] = 0\n"
    "      j = j + p\n"
    "    end\n"
    "  end\n"
    "  p = p + 1\n"
    "end\n"
    "sum = 0\n"
    "i = 2\n"
    "while i < limit\n"
    "  if sieve[i] == 1\n"
    "    sum = sum + i\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "sum\n";

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    int ok = 1;
    luby_state *L = luby_new(NULL);
    if (!L) { printf("FAIL: luby_new\n"); return 1; }
    luby_open_base(L);

    luby_value out;

    printf("Euler 1  — Multiples of 3 or 5 ...\n");
    if (eval_check(L, "euler1", euler1, &out))  { ok &= assert_int("euler1", out, 233168); }       else { ok = 0; }

    printf("Euler 2  — Even Fibonacci numbers ...\n");
    if (eval_check(L, "euler2", euler2, &out))  { ok &= assert_int("euler2", out, 4613732); }      else { ok = 0; }

    printf("Euler 3  — Largest prime factor ...\n");
    if (eval_check(L, "euler3", euler3, &out))  { ok &= assert_int("euler3", out, 6857); }         else { ok = 0; }

    printf("Euler 4  — Largest palindrome product ...\n");
    if (eval_check(L, "euler4", euler4, &out))  { ok &= assert_int("euler4", out, 906609); }       else { ok = 0; }

    printf("Euler 5  — Smallest multiple ...\n");
    if (eval_check(L, "euler5", euler5, &out))  { ok &= assert_int("euler5", out, 232792560); }    else { ok = 0; }

    printf("Euler 6  — Sum square difference ...\n");
    if (eval_check(L, "euler6", euler6, &out))  { ok &= assert_int("euler6", out, 25164150); }     else { ok = 0; }

    printf("Euler 7  — 10001st prime ...\n");
    if (eval_check(L, "euler7", euler7, &out))  { ok &= assert_int("euler7", out, 104743); }       else { ok = 0; }

    printf("Euler 8  — Largest product in a series ...\n");
    if (eval_check(L, "euler8", euler8, &out))  { ok &= assert_int("euler8", out, 23514624000LL); } else { ok = 0; }

    printf("Euler 9  — Special Pythagorean triplet ...\n");
    if (eval_check(L, "euler9", euler9, &out))  { ok &= assert_int("euler9", out, 31875000); }     else { ok = 0; }

    printf("Euler 10 — Summation of primes ...\n");
    if (eval_check(L, "euler10", euler10, &out)) { ok &= assert_int("euler10", out, 142913828922LL); } else { ok = 0; }

    luby_free(L);

    printf("\n");
    if (ok) {
        printf("All Euler tests PASSED\n");
    } else {
        printf("Some Euler tests FAILED\n");
    }
    return ok ? 0 : 1;
}
