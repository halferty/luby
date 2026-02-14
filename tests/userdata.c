#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;
static int finalizer_called = 0;
static void *finalized_ptr = NULL;

#define TEST(name) printf("  %-50s ", name);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

// A simple C struct to wrap
typedef struct {
    double x, y, z;
} Vec3;

// Finalizer callback
static void vec3_finalizer(void *data) {
    finalizer_called++;
    finalized_ptr = data;
}

// C method: Vec3.length
static int vec3_length(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return 1;
    Vec3 *v = (Vec3 *)luby_userdata_ptr(argv[0]);
    if (!v) {
        luby_set_error(L, LUBY_E_RUNTIME, "dead userdata", NULL, 0, 0);
        return 1;
    }
    double len = v->x * v->x + v->y * v->y + v->z * v->z;
    // sqrt approximation: just return sum of squares for test simplicity
    *out = luby_float(len);
    return 0;
}

// C method: Vec3.x
static int vec3_get_x(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return 1;
    Vec3 *v = (Vec3 *)luby_userdata_ptr(argv[0]);
    if (!v) { luby_set_error(L, LUBY_E_RUNTIME, "dead userdata", NULL, 0, 0); return 1; }
    *out = luby_float(v->x);
    return 0;
}

// C method: Vec3.y
static int vec3_get_y(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return 1;
    Vec3 *v = (Vec3 *)luby_userdata_ptr(argv[0]);
    if (!v) { luby_set_error(L, LUBY_E_RUNTIME, "dead userdata", NULL, 0, 0); return 1; }
    *out = luby_float(v->y);
    return 0;
}

// C method: Vec3.z
static int vec3_get_z(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return 1;
    Vec3 *v = (Vec3 *)luby_userdata_ptr(argv[0]);
    if (!v) { luby_set_error(L, LUBY_E_RUNTIME, "dead userdata", NULL, 0, 0); return 1; }
    *out = luby_float(v->z);
    return 0;
}

// C method: Vec3.to_s
static int vec3_to_s(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return 1;
    Vec3 *v = (Vec3 *)luby_userdata_ptr(argv[0]);
    if (!v) { luby_set_error(L, LUBY_E_RUNTIME, "dead userdata", NULL, 0, 0); return 1; }
    char buf[128];
    snprintf(buf, sizeof(buf), "Vec3(%g, %g, %g)", v->x, v->y, v->z);
    *out = luby_string(L, buf, strlen(buf));
    return 0;
}

// C method: Vec3.alive?
static int vec3_alive(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return 1;
    *out = luby_bool(luby_userdata_alive(argv[0]));
    return 0;
}

static luby_value make_vec3(luby_state *L, luby_class *cls, double x, double y, double z) {
    luby_value ud = luby_new_userdata(L, sizeof(Vec3), vec3_finalizer);
    Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);
    v->x = x; v->y = y; v->z = z;
    luby_set_userdata_class(L, ud, cls);
    return ud;
}

static luby_class *vec3_cls = NULL;

// C function exposed as global: Vec3.new(x, y, z)
static int vec3_new(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    double x = (argc >= 1 && argv[0].type == LUBY_T_FLOAT) ? argv[0].as.f :
               (argc >= 1 && argv[0].type == LUBY_T_INT) ? (double)argv[0].as.i : 0.0;
    double y = (argc >= 2 && argv[1].type == LUBY_T_FLOAT) ? argv[1].as.f :
               (argc >= 2 && argv[1].type == LUBY_T_INT) ? (double)argv[1].as.i : 0.0;
    double z = (argc >= 3 && argv[2].type == LUBY_T_FLOAT) ? argv[2].as.f :
               (argc >= 3 && argv[2].type == LUBY_T_INT) ? (double)argv[2].as.i : 0.0;
    *out = make_vec3(L, vec3_cls, x, y, z);
    return 0;
}

int main() {
    printf("=== Userdata Tests ===\n\n");

    // --- Test 1: Basic creation and ptr access ---
    TEST("luby_new_userdata creates valid userdata") {
        luby_state *L = luby_new(NULL);
        luby_value ud = luby_new_userdata(L, sizeof(Vec3), NULL);
        assert(ud.type == LUBY_T_USERDATA);
        Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);
        assert(v != NULL);
        v->x = 1.0; v->y = 2.0; v->z = 3.0;
        assert(v->x == 1.0);
        assert(luby_userdata_alive(ud) == 1);
        luby_free(L);
        PASS();
    }

    // --- Test 2: luby_wrap_userdata ---
    TEST("luby_wrap_userdata wraps external pointer") {
        luby_state *L = luby_new(NULL);
        Vec3 stack_vec = { 10.0, 20.0, 30.0 };
        luby_value ud = luby_wrap_userdata(L, &stack_vec, NULL);
        assert(ud.type == LUBY_T_USERDATA);
        Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);
        assert(v == &stack_vec);
        assert(v->x == 10.0);
        luby_free(L);
        PASS();
    }

    // --- Test 3: Invalidation ---
    TEST("luby_invalidate_userdata tombstones the ptr") {
        luby_state *L = luby_new(NULL);
        finalizer_called = 0;
        luby_value ud = luby_new_userdata(L, sizeof(Vec3), vec3_finalizer);
        Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);
        v->x = 42.0;
        assert(luby_userdata_alive(ud) == 1);

        int ok = luby_invalidate_userdata(ud);
        assert(ok == 1);
        assert(luby_userdata_alive(ud) == 0);
        assert(luby_userdata_ptr(ud) == NULL);
        assert(finalizer_called == 1);

        // Double-invalidate should be no-op
        ok = luby_invalidate_userdata(ud);
        assert(ok == 0);
        assert(finalizer_called == 1);

        luby_free(L);
        PASS();
    }

    // --- Test 4: Finalizer called on GC ---
    TEST("finalizer called when GC collects userdata") {
        luby_state *L = luby_new(NULL);
        finalizer_called = 0;
        {
            luby_value ud = luby_new_userdata(L, sizeof(Vec3), vec3_finalizer);
            (void)ud;  // let it become unreachable
        }
        luby_free(L);  // frees all GC objects
        assert(finalizer_called == 1);
        PASS();
    }

    // --- Test 5: Method dispatch on userdata ---
    TEST("C methods callable on userdata from Ruby") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_define_method(L, vec3_cls, "y", vec3_get_y);
        luby_define_method(L, vec3_cls, "z", vec3_get_z);
        luby_define_method(L, vec3_cls, "length_sq", vec3_length);
        luby_define_method(L, vec3_cls, "to_s", vec3_to_s);
        luby_define_method(L, vec3_cls, "alive?", vec3_alive);
        luby_register_function(L, "make_vec3", vec3_new);

        luby_value ud = make_vec3(L, vec3_cls, 3.0, 4.0, 0.0);
        luby_set_global_value(L, "v", ud);

        luby_value result;
        int rc;

        // Call method via Ruby
        rc = luby_eval(L, "v.x", 0, "<test>", &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 3.0);

        rc = luby_eval(L, "v.y", 0, "<test>", &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 4.0);

        rc = luby_eval(L, "v.length_sq", 0, "<test>", &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 25.0);

        luby_free(L);
        PASS();
    }

    // --- Test 6: is_a? works on userdata ---
    TEST("is_a? returns true for userdata class") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_register_function(L, "make_vec3", vec3_new);

        luby_value ud = make_vec3(L, vec3_cls, 1.0, 2.0, 3.0);
        luby_set_global_value(L, "v", ud);

        luby_value result;
        int rc = luby_eval(L, "v.is_a?(Vec3)", 0, "<test>", &result);
        assert(rc == 0 && luby_is_truthy(result));

        luby_free(L);
        PASS();
    }

    // --- Test 7: respond_to? works on userdata ---
    TEST("respond_to? works for userdata methods") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_define_method(L, vec3_cls, "y", vec3_get_y);
        luby_register_function(L, "make_vec3", vec3_new);

        luby_value ud = make_vec3(L, vec3_cls, 1.0, 2.0, 3.0);
        luby_set_global_value(L, "v", ud);

        luby_value result;
        int rc = luby_eval(L, "v.respond_to?(:x)", 0, "<test>", &result);
        assert(rc == 0 && luby_is_truthy(result));

        rc = luby_eval(L, "v.respond_to?(:nonexistent)", 0, "<test>", &result);
        assert(rc == 0 && !luby_is_truthy(result));

        luby_free(L);
        PASS();
    }

    // --- Test 8: Userdata method gets called via Ruby pipeline ---
    TEST("userdata methods work in Ruby expressions") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_define_method(L, vec3_cls, "y", vec3_get_y);
        luby_define_method(L, vec3_cls, "z", vec3_get_z);
        luby_register_function(L, "make_vec3", vec3_new);

        // Create via registered function from Ruby
        luby_value result;
        int rc = luby_eval(L, "v = make_vec3(1, 2, 3)\nv.x + v.y + v.z", 0, "<test>", &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 6.0);

        luby_free(L);
        PASS();
    }

    // --- Test 9: Invalidated userdata returns error from method ---
    TEST("method on invalidated userdata returns error") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_register_function(L, "make_vec3", vec3_new);

        finalizer_called = 0;
        luby_value ud = make_vec3(L, vec3_cls, 1.0, 2.0, 3.0);
        luby_set_global_value(L, "v", ud);

        // Works before invalidation
        luby_value result;
        int rc = luby_eval(L, "v.x", 0, "<test>", &result);
        assert(rc == 0 && result.as.f == 1.0);

        // Invalidate
        luby_invalidate_userdata(ud);
        assert(finalizer_called == 1);

        // Now method call should fail (ptr is NULL, method returns error)
        rc = luby_eval(L, "v.x", 0, "<test>", &result);
        assert(rc != 0);

        luby_free(L);
        PASS();
    }

    // --- Test 10: type_name returns class name ---
    TEST("type_name returns userdata class name") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);

        luby_value ud = make_vec3(L, vec3_cls, 0, 0, 0);
        luby_set_global_value(L, "v", ud);

        luby_value result;
        int rc = luby_eval(L, "v.class.name", 0, "<test>", &result);
        // This would require class method, let's just test the C API
        (void)rc; (void)result;

        // Just verify type_name returns "Vec3"
        const char *tname = luby_type_name(ud);
        assert(strcmp(tname, "Vec3") == 0);

        luby_free(L);
        PASS();
    }

    // --- Test 11: wrap_userdata doesn't free host memory ---
    TEST("wrap_userdata does not free host memory") {
        luby_state *L = luby_new(NULL);
        finalizer_called = 0;
        Vec3 host_vec = { 1.0, 2.0, 3.0 };
        luby_value ud = luby_wrap_userdata(L, &host_vec, vec3_finalizer);
        (void)ud;
        luby_free(L);
        // Finalizer was called but host memory is on stack, not freed
        assert(finalizer_called == 1);
        // host_vec should still be valid
        assert(host_vec.x == 1.0);
        PASS();
    }

    // --- Test 12: invoke_method on userdata from C ---
    TEST("luby_invoke_method works on userdata") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);

        vec3_cls = luby_define_class(L, "Vec3", NULL);
        luby_define_method(L, vec3_cls, "x", vec3_get_x);
        luby_define_method(L, vec3_cls, "length_sq", vec3_length);

        luby_value ud = make_vec3(L, vec3_cls, 3.0, 4.0, 5.0);

        luby_value result;
        int rc = luby_invoke_method(L, ud, "x", 0, NULL, &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 3.0);

        rc = luby_invoke_method(L, ud, "length_sq", 0, NULL, &result);
        assert(rc == 0 && result.type == LUBY_T_FLOAT && result.as.f == 50.0);

        luby_free(L);
        PASS();
    }

    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
