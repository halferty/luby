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

## Defining Classes & Userdata

```c
luby_class *cls = luby_define_class(L, "Enemy", "Object");
luby_define_method(L, cls, "attack", enemy_attack_fn);

// Wrap a C struct as userdata with a destructor
luby_value ud = luby_new_userdata(L, sizeof(MyStruct), my_finalizer);
MyStruct *ptr = (MyStruct *)luby_userdata_ptr(ud);
```

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
- There is currently **no garbage collector**; heap objects live until `luby_free` is called. Avoid unbounded allocations in long-running states until GC is implemented.
- Use the allocator hook to track or cap memory usage if needed.
