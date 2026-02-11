#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct memfile {
    const char *path;
    const char *content;
} memfile;

typedef struct memfs {
    const memfile *files;
    size_t count;
    int reads;
} memfs;

static int mem_exists(void *user, const char *path) {
    memfs *fs = (memfs *)user;
    for (size_t i = 0; i < fs->count; i++) {
        if (strcmp(fs->files[i].path, path) == 0) return 1;
    }
    return 0;
}

static char *mem_read(void *user, const char *path, size_t *out_size) {
    memfs *fs = (memfs *)user;
    for (size_t i = 0; i < fs->count; i++) {
        if (strcmp(fs->files[i].path, path) == 0) {
            fs->reads += 1;
            size_t len = strlen(fs->files[i].content);
            char *buf = (char *)malloc(len + 1);
            memcpy(buf, fs->files[i].content, len + 1);
            if (out_size) *out_size = len;
            return buf;
        }
    }
    return NULL;
}

int main(void) {
    memfile files[] = {
        {"/lib/foo.rb", "def foo()\n 3\n end"},
        {"/lib/bar.rb", "def bar()\n 5\n end"},
        {"/data/config.txt", "player_name=Hero\nlevel=5\n"}
    };
    memfs fs = { files, 3, 0 };

    luby_config cfg = {0};
    cfg.vfs.user = &fs;
    cfg.vfs.exists = mem_exists;
    cfg.vfs.read = mem_read;

    luby_state *L = luby_new(&cfg);
    luby_open_base(L);
    luby_add_search_path(L, "/lib");
    luby_add_search_path(L, "/data");

    luby_value out;
    int ok = 1;
    if (luby_eval(L, "require(\"foo\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 1) ok = 0;
    if (luby_eval(L, "foo()", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_INT || out.as.i != 3) ok = 0;

    if (luby_eval(L, "require(\"foo\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 0) ok = 0;
    if (luby_eval(L, "foo()", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_INT || out.as.i != 3) ok = 0;

    if (fs.reads != 1) ok = 0;

    if (luby_eval(L, "require(\"bar.rb\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 1) ok = 0;
    if (luby_eval(L, "bar()", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_INT || out.as.i != 5) ok = 0;

    if (luby_eval(L, "load(\"foo\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 1) ok = 0;
    if (luby_eval(L, "load(\"foo\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 1) ok = 0;

    if (fs.reads != 4) ok = 0;

    // Test load_text
    if (luby_eval(L, "load_text(\"config.txt\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_STRING) { printf("FAIL: load_text type\n"); ok = 0; }
    else if (strcmp((const char *)out.as.ptr, "player_name=Hero\nlevel=5\n") != 0) {
        printf("FAIL: load_text content: got '%s'\n", (const char *)out.as.ptr);
        ok = 0;
    }

    // Test load_text with search path
    if (luby_eval(L, "load_text(\"/data/config.txt\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_STRING) { printf("FAIL: load_text direct path type\n"); ok = 0; }

    // Test load_text on nonexistent file returns nil
    if (luby_eval(L, "load_text(\"nonexistent.txt\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_NIL) { printf("FAIL: load_text nonexistent should be nil\n"); ok = 0; }

    // Test file_exists?
    if (luby_eval(L, "file_exists?(\"config.txt\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 1) { printf("FAIL: file_exists? should be true\n"); ok = 0; }

    if (luby_eval(L, "file_exists?(\"nope.txt\")", 0, "<test>", &out) != 0) ok = 0;
    if (out.type != LUBY_T_BOOL || out.as.b != 0) { printf("FAIL: file_exists? should be false\n"); ok = 0; }

    luby_free(L);
    return ok ? 0 : 1;
}
