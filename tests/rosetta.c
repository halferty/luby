/**
 * Rosetta Code solutions — classic algorithms stress-testing the Luby
 * runtime.  Each task is solved in pure Luby and the C harness asserts
 * the known-correct answer.
 *
 * Tasks drawn from https://rosettacode.org
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

static int assert_string(const char *label, luby_value v, const char *expected) {
    if (v.type != LUBY_T_STRING) {
        printf("FAIL: %s (got type=%d, expected string)\n", label, v.type);
        return 0;
    }
    const char *actual = v.as.ptr ? (const char *)v.as.ptr : "";
    if (strcmp(actual, expected) != 0) {
        printf("FAIL: %s (got \"%s\", expected \"%s\")\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<rosetta>", out);
    if (rc != 0) {
        char buf[512];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s — runtime error: %s\n", label, buf);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Ackermann function                                                  */
/*                                                                     */
/* Classic recursion stress-test.                                      */
/* ack(3, 4) = 125                                                     */
/* ------------------------------------------------------------------ */

static const char *ackermann =
    "def ack(m, n)\n"
    "  if m == 0\n"
    "    return n + 1\n"
    "  end\n"
    "  if n == 0\n"
    "    return ack(m - 1, 1)\n"
    "  end\n"
    "  ack(m - 1, ack(m, n - 1))\n"
    "end\n"
    "ack(3, 4)\n";

/* ------------------------------------------------------------------ */
/* FizzBuzz                                                            */
/*                                                                     */
/* Classic interview problem. Count how many times "FizzBuzz" appears  */
/* in the sequence 1..100.  Answer: 6  (15,30,45,60,75,90)            */
/* ------------------------------------------------------------------ */

static const char *fizzbuzz =
    "count = 0\n"
    "i = 1\n"
    "while i <= 100\n"
    "  if i % 15 == 0\n"
    "    count = count + 1\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "count\n";

/* ------------------------------------------------------------------ */
/* FizzBuzz (string version)                                           */
/*                                                                     */
/* Build the actual FizzBuzz string for 1..20, joined by commas.       */
/* Tests string concatenation and interpolation.                       */
/* ------------------------------------------------------------------ */

static const char *fizzbuzz_str =
    "result = \"\"\n"
    "i = 1\n"
    "while i <= 20\n"
    "  if i > 1\n"
    "    result = result + \",\"\n"
    "  end\n"
    "  if i % 15 == 0\n"
    "    result = result + \"FizzBuzz\"\n"
    "  elsif i % 3 == 0\n"
    "    result = result + \"Fizz\"\n"
    "  elsif i % 5 == 0\n"
    "    result = result + \"Buzz\"\n"
    "  else\n"
    "    result = result + to_s(i)\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "result\n";

/* ------------------------------------------------------------------ */
/* Fibonacci sequence                                                  */
/*                                                                     */
/* fib(30) = 832040                                                    */
/* Iterative version — tests basic loop and variable juggling.         */
/* ------------------------------------------------------------------ */

static const char *fibonacci =
    "def fib(n)\n"
    "  a = 0\n"
    "  b = 1\n"
    "  i = 0\n"
    "  while i < n\n"
    "    t = a + b\n"
    "    a = b\n"
    "    b = t\n"
    "    i = i + 1\n"
    "  end\n"
    "  a\n"
    "end\n"
    "fib(30)\n";

/* ------------------------------------------------------------------ */
/* Happy numbers                                                       */
/*                                                                     */
/* Find the 8th happy number.  Answer: 31                              */
/* A happy number reaches 1 by repeatedly summing squares of digits.   */
/* ------------------------------------------------------------------ */

static const char *happy =
    "def digit_sq_sum(n)\n"
    "  s = 0\n"
    "  while n > 0\n"
    "    d = n % 10\n"
    "    s = s + d * d\n"
    "    n = n / 10\n"
    "  end\n"
    "  s\n"
    "end\n"
    "\n"
    "def happy(n)\n"
    "  seen = []\n"
    "  while n != 1\n"
    "    n = digit_sq_sum(n)\n"
    "    # check if we've seen this before (cycle detection)\n"
    "    i = 0\n"
    "    found = false\n"
    "    while i < len(seen)\n"
    "      if seen[i] == n\n"
    "        found = true\n"
    "      end\n"
    "      i = i + 1\n"
    "    end\n"
    "    if found\n"
    "      return false\n"
    "    end\n"
    "    array_push(seen, n)\n"
    "  end\n"
    "  true\n"
    "end\n"
    "\n"
    "count = 0\n"
    "n = 1\n"
    "result = 0\n"
    "while count < 8\n"
    "  if happy(n)\n"
    "    count = count + 1\n"
    "    result = n\n"
    "  end\n"
    "  n = n + 1\n"
    "end\n"
    "result\n";

/* ------------------------------------------------------------------ */
/* Digital root                                                        */
/*                                                                     */
/* Repeatedly sum digits until a single digit remains.                 */
/* digital_root(627615) = 9, persistence = 2                           */
/* We return the digital root.                                         */
/* ------------------------------------------------------------------ */

static const char *digital_root =
    "def dr(n)\n"
    "  while n >= 10\n"
    "    s = 0\n"
    "    while n > 0\n"
    "      s = s + n % 10\n"
    "      n = n / 10\n"
    "    end\n"
    "    n = s\n"
    "  end\n"
    "  n\n"
    "end\n"
    "dr(627615)\n";

/* ------------------------------------------------------------------ */
/* Ethiopian multiplication                                            */
/*                                                                     */
/* Multiply by halving and doubling.  17 * 34 = 578                    */
/* ------------------------------------------------------------------ */

static const char *ethiopian =
    "def eth_mult(a, b)\n"
    "  result = 0\n"
    "  while a >= 1\n"
    "    if a % 2 != 0\n"
    "      result = result + b\n"
    "    end\n"
    "    a = a / 2\n"
    "    b = b * 2\n"
    "  end\n"
    "  result\n"
    "end\n"
    "eth_mult(17, 34)\n";

/* ------------------------------------------------------------------ */
/* Hailstone sequence (Collatz)                                        */
/*                                                                     */
/* Starting from 27, the sequence has 112 elements.                    */
/* ------------------------------------------------------------------ */

static const char *hailstone =
    "def hailstone_len(n)\n"
    "  count = 1\n"
    "  while n != 1\n"
    "    if n % 2 == 0\n"
    "      n = n / 2\n"
    "    else\n"
    "      n = 3 * n + 1\n"
    "    end\n"
    "    count = count + 1\n"
    "  end\n"
    "  count\n"
    "end\n"
    "hailstone_len(27)\n";

/* ------------------------------------------------------------------ */
/* Dot product                                                         */
/*                                                                     */
/* [1,3,-5] · [4,-2,-1] = 3                                           */
/* ------------------------------------------------------------------ */

static const char *dot_product =
    "def dot_prod(a, b)\n"
    "  sum = 0\n"
    "  idx = 0\n"
    "  while idx < len(a)\n"
    "    sum = sum + a[idx] * b[idx]\n"
    "    idx = idx + 1\n"
    "  end\n"
    "  sum\n"
    "end\n"
    "a = [1, 3, 0 - 5]\n"
    "b = [4, 0 - 2, 0 - 1]\n"
    "dot_prod(a, b)\n";

/* ------------------------------------------------------------------ */
/* Bubble sort                                                         */
/*                                                                     */
/* Sort [64,34,25,12,22,11,90] and return the 4th element.             */
/* Sorted: [11,12,22,25,34,64,90], answer: 25                         */
/* ------------------------------------------------------------------ */

static const char *bubble_sort =
    "def bsort(arr)\n"
    "  n = len(arr)\n"
    "  i = 0\n"
    "  while i < n\n"
    "    j = 0\n"
    "    while j < n - i - 1\n"
    "      if arr[j] > arr[j + 1]\n"
    "        tmp = arr[j]\n"
    "        arr[j] = arr[j + 1]\n"
    "        arr[j + 1] = tmp\n"
    "      end\n"
    "      j = j + 1\n"
    "    end\n"
    "    i = i + 1\n"
    "  end\n"
    "  arr\n"
    "end\n"
    "a = bsort([64, 34, 25, 12, 22, 11, 90])\n"
    "a[3]\n";

/* ------------------------------------------------------------------ */
/* Insertion sort                                                      */
/*                                                                     */
/* Sort [5,3,1,4,2] and return as comma-separated string.              */
/* ------------------------------------------------------------------ */

static const char *insertion_sort =
    "def isort(arr)\n"
    "  i = 1\n"
    "  while i < len(arr)\n"
    "    val = arr[i]\n"
    "    j = i - 1\n"
    "    while j >= 0 && arr[j] > val\n"
    "      arr[j + 1] = arr[j]\n"
    "      j = j - 1\n"
    "    end\n"
    "    arr[j + 1] = val\n"
    "    i = i + 1\n"
    "  end\n"
    "  arr\n"
    "end\n"
    "a = isort([5, 3, 1, 4, 2])\n"
    "join(map(a) {|x| to_s(x)}, \",\")\n";

/* ------------------------------------------------------------------ */
/* Roman numerals / Encode                                             */
/*                                                                     */
/* Convert 1990 to Roman numerals.  Answer: "MCMXC"                   */
/* ------------------------------------------------------------------ */

static const char *roman_encode =
    "def to_roman(n)\n"
    "  vals = [1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1]\n"
    "  syms = [\"M\", \"CM\", \"D\", \"CD\", \"C\", \"XC\", \"L\", \"XL\", \"X\", \"IX\", \"V\", \"IV\", \"I\"]\n"
    "  result = \"\"\n"
    "  i = 0\n"
    "  while i < len(vals)\n"
    "    while n >= vals[i]\n"
    "      result = result + syms[i]\n"
    "      n = n - vals[i]\n"
    "    end\n"
    "    i = i + 1\n"
    "  end\n"
    "  result\n"
    "end\n"
    "to_roman(1990)\n";

/* ------------------------------------------------------------------ */
/* Roman numerals / Encode (second test)                               */
/* 2024 => "MMXXIV"                                                    */
/* ------------------------------------------------------------------ */

static const char *roman_encode2 =
    "def to_roman(n)\n"
    "  vals = [1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1]\n"
    "  syms = [\"M\", \"CM\", \"D\", \"CD\", \"C\", \"XC\", \"L\", \"XL\", \"X\", \"IX\", \"V\", \"IV\", \"I\"]\n"
    "  result = \"\"\n"
    "  i = 0\n"
    "  while i < len(vals)\n"
    "    while n >= vals[i]\n"
    "      result = result + syms[i]\n"
    "      n = n - vals[i]\n"
    "    end\n"
    "    i = i + 1\n"
    "  end\n"
    "  result\n"
    "end\n"
    "to_roman(2024)\n";

/* ------------------------------------------------------------------ */
/* Greatest common divisor (Euclidean)                                 */
/*                                                                     */
/* gcd(1071, 462) = 21                                                 */
/* ------------------------------------------------------------------ */

static const char *gcd_euclid =
    "def gcd(a, b)\n"
    "  while b != 0\n"
    "    t = b\n"
    "    b = a % b\n"
    "    a = t\n"
    "  end\n"
    "  a\n"
    "end\n"
    "gcd(1071, 462)\n";

/* ------------------------------------------------------------------ */
/* Leap year                                                           */
/*                                                                     */
/* Count leap years in 1900..2100.  Answer: 49                         */
/* (1904,1908,...,2096 = 49; 1900 and 2100 are NOT leap years)         */
/* ------------------------------------------------------------------ */

static const char *leap_year =
    "def leap(y)\n"
    "  if y % 400 == 0\n"
    "    return true\n"
    "  end\n"
    "  if y % 100 == 0\n"
    "    return false\n"
    "  end\n"
    "  y % 4 == 0\n"
    "end\n"
    "\n"
    "count = 0\n"
    "y = 1900\n"
    "while y <= 2100\n"
    "  if leap(y)\n"
    "    count = count + 1\n"
    "  end\n"
    "  y = y + 1\n"
    "end\n"
    "count\n";

/* ------------------------------------------------------------------ */
/* Josephus problem                                                    */
/*                                                                     */
/* n=41 soldiers in a circle, every k=3 eliminated.                    */
/* Survivor position (0-indexed): 30                                   */
/* Uses the mathematical recurrence: J(1)=0, J(n)=(J(n-1)+k)%n       */
/* ------------------------------------------------------------------ */

static const char *josephus =
    "def josephus(n, k)\n"
    "  pos = 0\n"
    "  i = 2\n"
    "  while i <= n\n"
    "    pos = (pos + k) % i\n"
    "    i = i + 1\n"
    "  end\n"
    "  pos\n"
    "end\n"
    "josephus(41, 3)\n";

/* ------------------------------------------------------------------ */
/* Look-and-say sequence                                               */
/*                                                                     */
/* Start with "1", apply 7 iterations.                                 */
/* Sequence: 1, 11, 21, 1211, 111221, 312211, 13112221, 1113213211   */
/* 7th iteration (0-indexed from "1") = "1113213211"                   */
/* Tests heavy string manipulation.                                    */
/* ------------------------------------------------------------------ */

static const char *look_and_say =
    "def las(s)\n"
    "  result = \"\"\n"
    "  i = 0\n"
    "  while i < len(s)\n"
    "    ch = s[i]\n"
    "    count = 1\n"
    "    while i + count < len(s) && s[i + count] == ch\n"
    "      count = count + 1\n"
    "    end\n"
    "    result = result + to_s(count) + ch\n"
    "    i = i + count\n"
    "  end\n"
    "  result\n"
    "end\n"
    "\n"
    "s = \"1\"\n"
    "iter = 0\n"
    "while iter < 7\n"
    "  s = las(s)\n"
    "  iter = iter + 1\n"
    "end\n"
    "s\n";

/* ------------------------------------------------------------------ */
/* Luhn test                                                           */
/*                                                                     */
/* Validate credit card numbers using the Luhn algorithm.              */
/* "49927398716" is valid (return 1), "49927398717" is not (return 0). */
/* We return 1 if valid, 0 if not.                                     */
/* ------------------------------------------------------------------ */

static const char *luhn =
    "def luhn_valid(s)\n"
    "  n = len(s)\n"
    "  sum = 0\n"
    "  alt = false\n"
    "  i = n - 1\n"
    "  while i >= 0\n"
    "    d = to_i(s[i])\n"
    "    if alt\n"
    "      d = d * 2\n"
    "      if d > 9\n"
    "        d = d - 9\n"
    "      end\n"
    "    end\n"
    "    sum = sum + d\n"
    "    alt = !alt\n"
    "    i = i - 1\n"
    "  end\n"
    "  sum % 10 == 0\n"
    "end\n"
    "\n"
    "r = 0\n"
    "if luhn_valid(\"49927398716\")\n"
    "  r = r + 1\n"
    "end\n"
    "if luhn_valid(\"49927398717\")\n"
    "  r = r + 10\n"
    "end\n"
    "if luhn_valid(\"1234567812345670\")\n"
    "  r = r + 100\n"
    "end\n"
    "r\n";

/* ------------------------------------------------------------------ */
/* Sum of divisors                                                     */
/*                                                                     */
/* sigma(240) = 744                                                    */
/* ------------------------------------------------------------------ */

static const char *sum_divisors =
    "def sigma(n)\n"
    "  sum = 0\n"
    "  i = 1\n"
    "  while i * i <= n\n"
    "    if n % i == 0\n"
    "      sum = sum + i\n"
    "      if i != n / i\n"
    "        sum = sum + n / i\n"
    "      end\n"
    "    end\n"
    "    i = i + 1\n"
    "  end\n"
    "  sum\n"
    "end\n"
    "sigma(240)\n";

/* ------------------------------------------------------------------ */
/* Perfect numbers                                                     */
/*                                                                     */
/* Count perfect numbers below 10000.  Answer: 4 (6, 28, 496, 8128)  */
/* ------------------------------------------------------------------ */

static const char *perfect_numbers =
    "def sigma(n)\n"
    "  sum = 0\n"
    "  i = 1\n"
    "  while i * i <= n\n"
    "    if n % i == 0\n"
    "      sum = sum + i\n"
    "      if i != n / i\n"
    "        sum = sum + n / i\n"
    "      end\n"
    "    end\n"
    "    i = i + 1\n"
    "  end\n"
    "  sum\n"
    "end\n"
    "\n"
    "count = 0\n"
    "n = 2\n"
    "while n < 10000\n"
    "  if sigma(n) - n == n\n"
    "    count = count + 1\n"
    "  end\n"
    "  n = n + 1\n"
    "end\n"
    "count\n";

/* ------------------------------------------------------------------ */
/* Tower of Hanoi                                                      */
/*                                                                     */
/* Count the number of moves to solve for n=15 disks.                  */
/* Answer: 32767  (2^15 - 1)                                          */
/* Tests deep recursion.                                               */
/* ------------------------------------------------------------------ */

static const char *hanoi =
    "def hanoi(n, from, to, via)\n"
    "  if n == 0\n"
    "    return 0\n"
    "  end\n"
    "  hanoi(n - 1, from, via, to) + 1 + hanoi(n - 1, via, to, from)\n"
    "end\n"
    "hanoi(15, 1, 3, 2)\n";

/* ------------------------------------------------------------------ */
/* Palindrome detection                                                */
/*                                                                     */
/* Check several strings. Return count of palindromes from:            */
/* "racecar", "hello", "madam", "ab", "a", ""                         */
/* Answer: 4  (racecar, madam, a, "")                                  */
/* ------------------------------------------------------------------ */

static const char *palindrome =
    "def palindrome(s)\n"
    "  s == reverse(s)\n"
    "end\n"
    "\n"
    "count = 0\n"
    "words = [\"racecar\", \"hello\", \"madam\", \"ab\", \"a\", \"\"]\n"
    "i = 0\n"
    "while i < len(words)\n"
    "  if palindrome(words[i])\n"
    "    count = count + 1\n"
    "  end\n"
    "  i = i + 1\n"
    "end\n"
    "count\n";

/* ------------------------------------------------------------------ */
/* Factorial (recursive)                                               */
/*                                                                     */
/* 20! = 2432902008176640000                                           */
/* Tests deep recursion and 64-bit arithmetic.                         */
/* ------------------------------------------------------------------ */

static const char *factorial =
    "def fact(n)\n"
    "  if n <= 1\n"
    "    return 1\n"
    "  end\n"
    "  n * fact(n - 1)\n"
    "end\n"
    "fact(20)\n";

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    int ok = 1;
    luby_state *L = luby_new(NULL);
    if (!L) { printf("FAIL: luby_new\n"); return 1; }
    luby_open_base(L);

    luby_value out;

    printf("Ackermann function ...\n");
    if (eval_check(L, "ackermann", ackermann, &out))      { ok &= assert_int("ackermann", out, 125); }           else { ok = 0; }

    printf("FizzBuzz (count) ...\n");
    if (eval_check(L, "fizzbuzz", fizzbuzz, &out))         { ok &= assert_int("fizzbuzz", out, 6); }              else { ok = 0; }

    printf("FizzBuzz (string) ...\n");
    if (eval_check(L, "fizzbuzz_str", fizzbuzz_str, &out)) { ok &= assert_string("fizzbuzz_str", out,
        "1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz,16,17,Fizz,19,Buzz"); }                       else { ok = 0; }

    printf("Fibonacci ...\n");
    if (eval_check(L, "fibonacci", fibonacci, &out))       { ok &= assert_int("fibonacci", out, 832040); }        else { ok = 0; }

    printf("Happy numbers ...\n");
    if (eval_check(L, "happy", happy, &out))               { ok &= assert_int("happy", out, 31); }                else { ok = 0; }

    printf("Digital root ...\n");
    if (eval_check(L, "digital_root", digital_root, &out)) { ok &= assert_int("digital_root", out, 9); }         else { ok = 0; }

    printf("Ethiopian multiplication ...\n");
    if (eval_check(L, "ethiopian", ethiopian, &out))       { ok &= assert_int("ethiopian", out, 578); }           else { ok = 0; }

    printf("Hailstone sequence ...\n");
    if (eval_check(L, "hailstone", hailstone, &out))       { ok &= assert_int("hailstone", out, 112); }           else { ok = 0; }

    printf("Dot product ...\n");
    if (eval_check(L, "dot_product", dot_product, &out))   { ok &= assert_int("dot_product", out, 3); }           else { ok = 0; }

    printf("Bubble sort ...\n");
    if (eval_check(L, "bubble_sort", bubble_sort, &out))   { ok &= assert_int("bubble_sort", out, 25); }          else { ok = 0; }

    printf("Insertion sort ...\n");
    if (eval_check(L, "insertion_sort", insertion_sort, &out)) { ok &= assert_string("insertion_sort", out, "1,2,3,4,5"); } else { ok = 0; }

    printf("Roman numerals (1990) ...\n");
    if (eval_check(L, "roman_encode", roman_encode, &out))  { ok &= assert_string("roman_encode", out, "MCMXC"); } else { ok = 0; }

    printf("Roman numerals (2024) ...\n");
    if (eval_check(L, "roman_encode2", roman_encode2, &out)) { ok &= assert_string("roman_encode2", out, "MMXXIV"); } else { ok = 0; }

    printf("GCD (Euclidean) ...\n");
    if (eval_check(L, "gcd_euclid", gcd_euclid, &out))    { ok &= assert_int("gcd_euclid", out, 21); }           else { ok = 0; }

    printf("Leap year ...\n");
    if (eval_check(L, "leap_year", leap_year, &out))       { ok &= assert_int("leap_year", out, 49); }            else { ok = 0; }

    printf("Josephus problem ...\n");
    if (eval_check(L, "josephus", josephus, &out))         { ok &= assert_int("josephus", out, 30); }             else { ok = 0; }

    printf("Look-and-say sequence ...\n");
    if (eval_check(L, "look_and_say", look_and_say, &out)) { ok &= assert_string("look_and_say", out, "1113213211"); } else { ok = 0; }

    printf("Luhn test ...\n");
    if (eval_check(L, "luhn", luhn, &out))                 { ok &= assert_int("luhn", out, 101); }                else { ok = 0; }

    printf("Sum of divisors ...\n");
    if (eval_check(L, "sum_divisors", sum_divisors, &out)) { ok &= assert_int("sum_divisors", out, 744); }        else { ok = 0; }

    printf("Perfect numbers ...\n");
    if (eval_check(L, "perfect_numbers", perfect_numbers, &out)) { ok &= assert_int("perfect_numbers", out, 4); } else { ok = 0; }

    printf("Tower of Hanoi ...\n");
    if (eval_check(L, "hanoi", hanoi, &out))               { ok &= assert_int("hanoi", out, 32767); }             else { ok = 0; }

    printf("Palindrome detection ...\n");
    if (eval_check(L, "palindrome", palindrome, &out))     { ok &= assert_int("palindrome", out, 4); }            else { ok = 0; }

    printf("Factorial ...\n");
    if (eval_check(L, "factorial", factorial, &out))       { ok &= assert_int("factorial", out, 2432902008176640000LL); } else { ok = 0; }

    luby_free(L);

    printf("\n");
    if (ok) {
        printf("All Rosetta tests PASSED\n");
    } else {
        printf("Some Rosetta tests FAILED\n");
    }
    return ok ? 0 : 1;
}
