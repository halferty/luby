# Luby

A single-header embeddable Ruby-like scripting language written in C.

## Overview

Luby brings a familiar Ruby-inspired syntax to C and C++ applications. It ships as a single header file (`luby.h`) with zero external dependencies — drop it into your project, define `LUBY_IMPLEMENTATION` in one translation unit, and you have a full scripting layer with classes, blocks, metaprogramming, coroutines, and more.

## Features

- **Single header** — one file, no build system integration required
- **Ruby-like object model** — classes, modules, inheritance, singleton methods, metaclasses
- **Metaprogramming** — `method_missing`, `define_method`, `class_eval`, `instance_eval`, `send`
- **Core types** — Integer, Float, String, Symbol, Array, Hash, Proc, Class, Module
- **Blocks & iterators** — closures, `yield`, chainable Enumerable methods (`map`, `select`, `reduce`, …)
- **Coroutines** — cooperative multitasking with yield/resume from the host
- **Virtual file system** — `require`/`load` go through embedder-provided callbacks
- **Debug hooks** — line, call, and return hooks for profiling and debugging
- **Custom allocator** — plug in your own memory allocator
- **Userdata & native classes** — bind C/C++ objects with finalizers and method tables

## Quick Start

```c
#define LUBY_IMPLEMENTATION
#include "luby.h"

int main(void) {
    luby_config cfg = {0};
    luby_state *L = luby_new(&cfg);
    luby_open_base(L);

    luby_value out;
    luby_eval(L, "puts 'Hello from Luby!'", 0, "<main>", &out);

    luby_free(L);
    return 0;
}
```

Compile with any C99 (or later) compiler:

```bash
cc -o hello hello.c
```

## Building the Examples & Tests

```bash
make            # build examples and tests
./run_tests.sh  # run the test suite
```

## Documentation

| Document | Description |
|----------|-------------|
| [EMBEDDING.md](EMBEDDING.md) | How to embed Luby in a C or C++ application |
| [SPEC.md](SPEC.md) | What the language supports (types, control flow, OOP, etc.) |
| [TODO.md](TODO.md) | Planned work and known limitations |

## License

MIT — see [LICENSE](LICENSE) for details.
