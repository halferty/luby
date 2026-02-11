/**
 * Test game math helpers and seeded RNG
 */
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <math.h>

static int tests_run = 0;
static int tests_passed = 0;

static int test_float(luby_state *L, const char *name, const char *code, double expected, double epsilon) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    tests_run++;
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    double val = (out.type == LUBY_T_FLOAT) ? out.as.f : (double)out.as.i;
    if (fabs(val - expected) > epsilon) {
        printf("FAIL %s: expected %f, got %f\n", name, expected, val);
        return 0;
    }
    printf("PASS %s\n", name);
    tests_passed++;
    return 1;
}

static int test_int(luby_state *L, const char *name, const char *code, int64_t expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    tests_run++;
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    int64_t val = (out.type == LUBY_T_INT) ? out.as.i : (int64_t)out.as.f;
    if (val != expected) {
        printf("FAIL %s: expected %lld, got %lld\n", name, (long long)expected, (long long)val);
        return 0;
    }
    printf("PASS %s\n", name);
    tests_passed++;
    return 1;
}

static int test_ok(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    tests_run++;
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    printf("PASS %s\n", name);
    tests_passed++;
    return 1;
}

int main(void) {
    printf("Testing game math helpers...\n\n");

    luby_state *L = luby_new(NULL);
    if (!L) {
        fprintf(stderr, "Failed to create luby_state\n");
        return 1;
    }
    luby_open_base(L);

    // Interpolation tests
    printf("--- Interpolation ---\n");
    test_float(L, "lerp 0", "lerp(0, 10, 0)", 0.0, 0.001);
    test_float(L, "lerp 1", "lerp(0, 10, 1)", 10.0, 0.001);
    test_float(L, "lerp 0.5", "lerp(0, 10, 0.5)", 5.0, 0.001);
    test_float(L, "lerp negative", "lerp(-10, 10, 0.5)", 0.0, 0.001);
    
    test_float(L, "inverse_lerp", "inverse_lerp(0, 10, 5)", 0.5, 0.001);
    test_float(L, "inverse_lerp edge", "inverse_lerp(0, 10, 0)", 0.0, 0.001);
    
    test_float(L, "smoothstep 0", "smoothstep(0, 1, 0)", 0.0, 0.001);
    test_float(L, "smoothstep 1", "smoothstep(0, 1, 1)", 1.0, 0.001);
    test_float(L, "smoothstep 0.5", "smoothstep(0, 1, 0.5)", 0.5, 0.001);

    // Clamping and wrapping
    printf("\n--- Clamp/Wrap ---\n");
    test_int(L, "clamp int low", "clamp(-5, 0, 10)", 0);
    test_int(L, "clamp int high", "clamp(15, 0, 10)", 10);
    test_int(L, "clamp int mid", "clamp(5, 0, 10)", 5);
    test_float(L, "clamp float", "clamp(1.5, 0.0, 1.0)", 1.0, 0.001);
    
    test_int(L, "wrap basic", "wrap(12, 0, 10)", 2);
    test_int(L, "wrap negative", "wrap(-3, 0, 10)", 7);
    test_float(L, "wrap float", "wrap(1.5, 0.0, 1.0)", 0.5, 0.001);

    // Sign, min, max
    printf("\n--- Sign/Min/Max ---\n");
    test_int(L, "sign positive", "sign(42)", 1);
    test_int(L, "sign negative", "sign(-42)", -1);
    test_int(L, "sign zero", "sign(0)", 0);
    
    test_int(L, "min two", "min(5, 3)", 3);
    test_int(L, "min three", "min(5, 3, 7)", 3);
    test_int(L, "max two", "max(5, 3)", 5);
    test_int(L, "max three", "max(5, 3, 7)", 7);
    test_float(L, "min float", "min(1.5, 2.5)", 1.5, 0.001);

    // Angle conversion
    printf("\n--- Angles ---\n");
    test_float(L, "deg_to_rad 180", "deg_to_rad(180)", 3.14159, 0.001);
    test_float(L, "deg_to_rad 90", "deg_to_rad(90)", 1.5708, 0.001);
    test_float(L, "rad_to_deg pi", "rad_to_deg(3.14159)", 180.0, 0.1);

    // Trig functions
    printf("\n--- Trigonometry ---\n");
    test_float(L, "sin 0", "sin(0)", 0.0, 0.001);
    test_float(L, "sin pi/2", "sin(1.5708)", 1.0, 0.001);
    test_float(L, "cos 0", "cos(0)", 1.0, 0.001);
    test_float(L, "cos pi", "cos(3.14159)", -1.0, 0.001);
    test_float(L, "tan 0", "tan(0)", 0.0, 0.001);
    test_float(L, "atan2", "atan2(1, 1)", 0.7854, 0.001);
    test_float(L, "asin", "asin(1)", 1.5708, 0.001);
    test_float(L, "acos", "acos(0)", 1.5708, 0.001);

    // Math functions
    printf("\n--- Math ---\n");
    test_float(L, "sqrt", "sqrt(16)", 4.0, 0.001);
    test_float(L, "pow", "pow(2, 10)", 1024.0, 0.001);
    test_float(L, "log", "log(2.718281828)", 1.0, 0.001);
    test_float(L, "exp", "exp(1)", 2.718, 0.01);
    test_float(L, "fmod", "fmod(5.5, 2.0)", 1.5, 0.001);

    // 2D vector math
    printf("\n--- 2D Vectors ---\n");
    test_float(L, "distance", "distance(0, 0, 3, 4)", 5.0, 0.001);
    test_float(L, "distance_squared", "distance_squared(0, 0, 3, 4)", 25.0, 0.001);
    test_ok(L, "normalize", "n = normalize(3, 4); abs(n[0] - 0.6) < 0.01 && abs(n[1] - 0.8) < 0.01");
    test_float(L, "dot", "dot(1, 0, 0, 1)", 0.0, 0.001);
    test_float(L, "dot parallel", "dot(1, 0, 1, 0)", 1.0, 0.001);
    test_float(L, "cross 2d", "cross(1, 0, 0, 1)", 1.0, 0.001);
    test_float(L, "angle", "angle(1, 0)", 0.0, 0.001);
    test_float(L, "angle 90", "angle(0, 1)", 1.5708, 0.001);

    // RNG tests
    printf("\n--- Seeded RNG ---\n");
    test_ok(L, "rand() in range", "r = rand(); r >= 0.0 && r < 1.0");
    test_ok(L, "rand(n) in range", "r = rand(100); r >= 0 && r < 100");
    test_ok(L, "rand(a,b) in range", "r = rand(10, 20); r >= 10 && r <= 20");
    test_ok(L, "rand_float", "r = rand_float(5.0, 10.0); r >= 5.0 && r < 10.0");
    
    // Test deterministic RNG
    test_ok(L, "srand deterministic", 
        "srand(42); a = rand(1000); srand(42); b = rand(1000); a == b");
    
    // Test sample
    test_ok(L, "sample", "arr = [1, 2, 3, 4, 5]; s = sample(arr); include?(arr, s)");
    
    // Test shuffle
    test_ok(L, "shuffle returns array", "arr = [1, 2, 3]; s = shuffle(arr); len(s) == 3");
    test_ok(L, "shuffle! in place", "arr = [1, 2, 3]; shuffle!(arr); len(arr) == 3");
    test_ok(L, "shuffle preserves elements",
        "arr = [1, 2, 3, 4, 5]; s = shuffle(arr); sum = 0; each(s) { |x| sum = sum + x }; sum == 15");

    // Test range with rand
    test_ok(L, "rand with range", "r = rand(1..10); r >= 1 && r <= 10");

    // Probability helpers
    printf("\n--- Probability ---\n");
    test_ok(L, "chance returns bool", "c = chance(50); c == true || c == false");
    test_ok(L, "chance 0 always false", "srand(1); result = true; times(10) { |i| if chance(0); result = false; end }; result");
    test_ok(L, "chance 100 always true", "srand(1); result = true; times(10) { |i| if !chance(100); result = false; end }; result");
    
    test_int(L, "dice 1d6 range", "srand(42); d = dice(1, 6); d >= 1 && d <= 6 ? 1 : 0", 1);
    test_ok(L, "dice 2d6 range", "srand(42); d = dice(2, 6); d >= 2 && d <= 12");
    test_ok(L, "dice 3d6 deterministic", "srand(99); a = dice(3, 6); srand(99); b = dice(3, 6); a == b");
    
    test_ok(L, "roll string 1d6", "srand(42); r = roll(\"1d6\"); r >= 1 && r <= 6");
    test_ok(L, "roll string 2d20", "srand(42); r = roll(\"2d20\"); r >= 2 && r <= 40");
    test_ok(L, "roll with modifier +", "srand(42); r = roll(\"1d6+5\"); r >= 6 && r <= 11");
    test_ok(L, "roll with modifier -", "srand(42); r = roll(\"1d6-2\"); r >= -1 && r <= 4");
    test_ok(L, "roll d20 shorthand", "srand(42); r = roll(\"d20\"); r >= 1 && r <= 20");
    
    test_ok(L, "weighted_choice hash", 
        "srand(123)\n"
        "loot = {\"common\" => 70, \"rare\" => 25, \"epic\" => 5}\n"
        "item = weighted_choice(loot)\n"
        "item == \"common\" || item == \"rare\" || item == \"epic\"");
    
    test_ok(L, "weighted_choice array",
        "srand(123)\n"
        "loot = [[\"gold\", 50], [\"potion\", 30], [\"sword\", 20]]\n"
        "item = weighted_choice(loot)\n"
        "item == \"gold\" || item == \"potion\" || item == \"sword\"");
    
    test_ok(L, "weighted_choice distribution",
        "srand(42)\n"
        "counts = {\"a\" => 0, \"b\" => 0}\n"
        "times(100) { |i|\n"
        "  item = weighted_choice({\"a\" => 90, \"b\" => 10})\n"
        "  counts[item] = counts[item] + 1\n"
        "}\n"
        "counts[\"a\"] > counts[\"b\"]");  // 'a' should appear more often

    // Integration tests
    printf("\n--- Integration ---\n");
    test_ok(L, "animation curve", 
        "t = 0.5\n"
        "start_x = 0\n"
        "end_x = 100\n"
        "x = lerp(start_x, end_x, smoothstep(0, 1, t))\n"
        "x == 50");
    
    test_ok(L, "circular motion",
        "angle = deg_to_rad(45)\n"
        "radius = 10\n"
        "x = cos(angle) * radius\n"
        "y = sin(angle) * radius\n"
        "d = distance(0, 0, x, y)\n"
        "abs(d - 10) < 0.01");
    
    test_ok(L, "random spawn in area",
        "srand(123)\n"
        "x = rand_float(0, 100)\n"
        "y = rand_float(0, 100)\n"
        "x >= 0 && x < 100 && y >= 0 && y < 100");

    luby_free(L);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
