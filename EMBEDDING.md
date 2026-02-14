# Embedding Luby

This guide covers integrating Luby into a C or C++ application. Luby is a single-header library — include `luby.h` and define `LUBY_IMPLEMENTATION` in exactly one translation unit.

## Minimal Example

```c
#define LUBY_IMPLEMENTATION
#include "luby.h"

int main(void) {
    luby_config cfg = {0};
    luby_state *L = luby_new(&cfg);
    luby_open_base(L);        // register built-in methods (puts, Array#map, etc.)

    luby_value out;
    luby_eval(L, "puts(1 + 2)", 0, "<main>", &out);

    luby_free(L);
    return 0;
}
```

Pass `0` for the `len` parameter of `luby_eval` and it will call `strlen` for you. The `filename` argument is used in error messages.

## C++ Notes

Luby's public API is wrapped in `extern "C"`, so it works directly from C++ with no extra wrappers. Just `#include "luby.h"` as usual.

---

## Lifecycle

| Function | Purpose |
|----------|---------|
| `luby_new(cfg)` | Create a new interpreter state. Pass `NULL` for defaults. |
| `luby_open_base(L)` | Register the built-in standard library. |
| `luby_free(L)` | Destroy the state and release all memory. |

---

## Evaluating Code

```c
int luby_eval(luby_state *L, const char *code, size_t len,
              const char *filename, luby_value *out);
```

Returns `0` on success. On failure, inspect the error with `luby_last_error` or `luby_format_error`.

### Loading Files

```c
int luby_require(luby_state *L, const char *path, luby_value *out);
int luby_load(luby_state *L, const char *path, luby_value *out);
```

`luby_require` loads a file at most once (like Ruby's `require`). `luby_load` always re-evaluates.

---

## Creating Values

| Function | Creates |
|----------|---------|
| `luby_nil()` | `nil` |
| `luby_bool(b)` | `true` / `false` |
| `luby_int(v)` | Integer (int64_t) |
| `luby_float(v)` | Float (double) |
| `luby_string(L, s, len)` | String (copied into VM) |
| `luby_symbol(L, s, len)` | Symbol (interned) |

---

## Globals

```c
luby_set_global_value(L, "player_name", luby_string(L, "Alice", 5));

luby_value v = luby_get_global_value(L, "player_name");
```

---

## Arrays & Hashes

```c
// Arrays
luby_value arr = luby_array_new(L);
luby_array_push_value(L, arr, luby_int(42));
size_t n = luby_array_len(arr);
luby_value elem;
luby_array_get(arr, 0, &elem);

// Hashes
luby_value h = luby_hash_new(L);
luby_hash_set_value(L, h, luby_symbol(L, "hp", 2), luby_int(100));
luby_value hp;
luby_hash_get_value(h, luby_symbol(L, "hp", 2), &hp);
```

---

## Registering Native Functions

```c
int my_add(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    // argv[0] and argv[1] are the arguments
    *out = luby_int(argv[0].as.i + argv[1].as.i);
    return 0;  // 0 = success
}

luby_register_function(L, "my_add", my_add);
```

After registration, Luby scripts can call `my_add(3, 4)`.

### Native Modules

```c
luby_register_module(L, "math_extras", math_extras_loader);
```

The loader function is called the first time a script does `require "math_extras"`.

---

## Defining Classes & Methods

```c
luby_class *cls = luby_define_class(L, "Enemy", NULL);   // NULL superclass = Object
luby_define_method(L, cls, "attack", enemy_attack_fn);
luby_define_method(L, cls, "hp", enemy_get_hp);
```

Methods receive `(luby_state *L, int argc, const luby_value *argv, luby_value *out)`. When called as `enemy.attack(target)`, `argv[0]` is the receiver (`enemy`), `argv[1]` is `target`, and `argc` is 2.

---

## Userdata — Wrapping C Structs

Userdata lets you hand a C pointer to the Ruby world. Methods dispatch through the userdata's assigned class, and the host can **invalidate** a userdata at any time (e.g., when the underlying C object is destroyed).

### Creating Userdata

Two modes:

```c
// 1. VM-owned: allocates sizeof(Vec3) bytes, GC frees them
luby_value ud = luby_new_userdata(L, sizeof(Vec3), vec3_finalizer);
Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);
v->x = 1; v->y = 2; v->z = 3;

// 2. Wrapped: the host owns the memory, GC will NOT free it
Vec3 *host_vec = get_from_engine();
luby_value ud = luby_wrap_userdata(L, host_vec, vec3_finalizer);
```

The finalizer callback (`void (*)(void *)`) is called exactly once — either when the userdata is explicitly invalidated, or when the GC collects it. Pass `NULL` if no cleanup is needed.

### Assigning a Class

```c
luby_class *cls = luby_define_class(L, "Vec3", NULL);
luby_define_method(L, cls, "x", vec3_get_x);
luby_define_method(L, cls, "length", vec3_length);

luby_set_userdata_class(L, ud, cls);
```

Once a class is assigned, Ruby code can call methods on the userdata:

```ruby
v = make_vec3(1, 2, 3)
puts v.x          # dispatches to vec3_get_x
puts v.is_a?(Vec3) # true
puts v.respond_to?(:x) # true
```

### Invalidation

When the host C object is destroyed (entity killed, resource freed, etc.), invalidate the userdata so Ruby code gets a clean error instead of a dangling pointer:

```c
luby_invalidate_userdata(ud);
// Calls the finalizer, sets data pointer to NULL, marks as dead.
// Any subsequent method call from Ruby will get an error.
// Double-invalidate is a safe no-op (returns 0).
```

From C, always check before dereferencing:

```c
Vec3 *v = (Vec3 *)luby_userdata_ptr(ud);  // returns NULL if dead
if (!v) { /* handle dead userdata */ }
```

### Checking Liveness

```c
int alive = luby_userdata_alive(ud);  // 1 = alive, 0 = dead
```

### API Summary

| Function | Purpose |
|----------|---------|
| `luby_new_userdata(L, size, fin)` | Allocate VM-owned userdata |
| `luby_wrap_userdata(L, ptr, fin)` | Wrap an external pointer (host-owned) |
| `luby_userdata_ptr(v)` | Get the data pointer (NULL if dead) |
| `luby_userdata_alive(v)` | Check if still valid |
| `luby_invalidate_userdata(v)` | Tombstone: call finalizer, null pointer |
| `luby_set_userdata_class(L, v, cls)` | Assign a class for method dispatch |

---

## Calling Into Luby From C

```c
// Call a global function
luby_value args[] = { luby_int(10) };
luby_value result;
luby_invoke_global(L, "double_it", 1, args, &result);

// Call a method on an object
luby_invoke_method(L, some_obj, "to_s", 0, NULL, &result);
```

---

## Coroutines

```c
luby_coroutine *co = luby_coroutine_new(L, func_value);

int yielded;
luby_value result;
luby_coroutine_resume(L, co, 0, NULL, &result, &yielded);
// yielded == 1 if the coroutine yielded rather than returned
```

For script-side cooperative concurrency, the `Fiber` class is available after `luby_open_base`:

```ruby
f = Fiber.new { Fiber.yield(1); 2 }
f.resume  #=> 1
f.resume  #=> 2
```

---

## Error Handling

```c
if (luby_eval(L, code, 0, file, &out) != 0) {
    char buf[256];
    luby_format_error(L, buf, sizeof(buf));
    fprintf(stderr, "%s\n", buf);   // e.g. "<main>:3: NameError: undefined variable 'x'"
    luby_clear_error(L);
}
```

| Function | Purpose |
|----------|---------|
| `luby_last_error(L)` | Returns the `luby_error` struct |
| `luby_format_error(L, buf, n)` | Writes a human-readable error string |
| `luby_error_code_string(code)` | Converts an error code enum to a name |
| `luby_clear_error(L)` | Resets the error state |

---

## Virtual File System

Supply VFS callbacks so `require` and `load` work in sandboxed or embedded environments (e.g., reading from a pak file):

```c
int my_exists(void *user, const char *path) { /* ... */ }
char *my_read(void *user, const char *path, size_t *out_size) { /* ... */ }

luby_config cfg = {
    .vfs = {
        .user   = my_context,
        .exists = my_exists,
        .read   = my_read,
        .stat   = NULL,  // optional
    },
};
luby_state *L = luby_new(&cfg);
```

---

## Custom Allocator

```c
void *my_alloc(void *user, void *ptr, size_t size) {
    if (size == 0) { free(ptr); return NULL; }
    return realloc(ptr, size);
}

luby_config cfg = {
    .alloc      = my_alloc,
    .alloc_user = NULL,
};
```

---

## Debug Hooks

```c
void my_hook(luby_state *L, luby_hook_event event,
             const char *file, int line, void *user) {
    if (event == LUBY_HOOK_LINE)
        printf("%s:%d\n", file, line);
}

luby_set_hook(L, my_hook, NULL);
```

Hook events: `LUBY_HOOK_LINE`, `LUBY_HOOK_CALL`, `LUBY_HOOK_RETURN`.

---

## Search Paths

```c
luby_add_search_path(L, "scripts/");
luby_add_search_path(L, "lib/");
// Now  require "utils"  will look for scripts/utils.rb, lib/utils.rb, etc.

luby_clear_search_paths(L);  // reset
```

---

## Memory Model

- All allocations go through the configured allocator (default: malloc/realloc/free).
- Values returned to the host are owned by the VM — do not free them yourself.
- Heap objects (strings, arrays, hashes, userdata, etc.) are tracked by the GC. Objects become eligible for collection when no longer reachable from globals, the stack, or the root set.
- Userdata finalizers are called when the GC collects the userdata, or when you explicitly call `luby_invalidate_userdata`. The finalizer is never called twice.
- Use the allocator hook to track or cap memory usage if needed.
