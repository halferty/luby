#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = malloc(len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    
    if (out_len) *out_len = len;
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.rb>\n", argv[0]);
        return 1;
    }

    size_t code_len;
    char *code = read_file(argv[1], &code_len);
    if (!code) {
        fprintf(stderr, "Failed to read file: %s\n", argv[1]);
        return 1;
    }

    luby_state *L = luby_new(NULL);
    if (!L) {
        fprintf(stderr, "Failed to create luby_state\n");
        free(code);
        return 1;
    }

    luby_open_base(L);
    
    luby_value result;
    int rc = luby_eval(L, code, code_len, argv[1], &result);
    
    if (rc != 0) {
        char buf[512];
        luby_format_error(L, buf, sizeof(buf));
        fprintf(stderr, "Error: %s\n", buf);
        luby_free(L);
        free(code);
        return 1;
    }
    
    luby_free(L);
    free(code);
    return 0;
}
