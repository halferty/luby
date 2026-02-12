#ifndef LUBY_H
#define LUBY_H

// Luby - single-header embeddable Ruby-like interpreter (subset)
// This file is intended to be included by both the host and (optionally) one
// compilation unit defining LUBY_IMPLEMENTATION.

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <math.h>

// ----------------------------- Configuration ------------------------------

#ifndef LUBY_API
#define LUBY_API
#endif

#ifndef LUBY_ASSERT
#include <assert.h>
#define LUBY_ASSERT(x) assert(x)
#endif

// ------------------------------ Forward Types ------------------------------

typedef struct luby_state luby_state;
typedef struct luby_value luby_value;
typedef struct luby_string_view luby_string_view;
typedef struct luby_error luby_error;
typedef struct luby_coroutine luby_coroutine;
typedef struct luby_proc luby_proc;
typedef struct luby_class luby_class;
typedef struct luby_module luby_module;
typedef struct luby_class_obj luby_class_obj;
typedef struct luby_gc_obj luby_gc_obj;

// ------------------------------ Value Model --------------------------------

typedef enum luby_type {
    LUBY_T_NIL = 0,
    LUBY_T_BOOL,
    LUBY_T_INT,
    LUBY_T_FLOAT,
    LUBY_T_STRING,
    LUBY_T_SYMBOL,
    LUBY_T_ARRAY,
    LUBY_T_HASH,
    LUBY_T_OBJECT,
    LUBY_T_PROC,
    LUBY_T_CLASS,
    LUBY_T_MODULE,
    LUBY_T_CMETHOD,
    LUBY_T_RANGE
} luby_type;

struct luby_string_view {
    const char *data;
    size_t length;
};

struct luby_value {
    luby_type type;
    union {
        int64_t i;
        double f;
        int b;
        void *ptr;
    } as;
};

// ------------------------------ Error Model --------------------------------

typedef enum luby_error_code {
    LUBY_E_OK = 0,
    LUBY_E_PARSE,
    LUBY_E_RUNTIME,
    LUBY_E_OOM,
    LUBY_E_IO,
    LUBY_E_TYPE,
    LUBY_E_NAME
} luby_error_code;

struct luby_error {
    luby_error_code code;
    const char *message;
    const char *file;
    int line;
    int column;
};

typedef enum luby_visibility {
    LUBY_VIS_PUBLIC = 0,
    LUBY_VIS_PROTECTED,
    LUBY_VIS_PRIVATE
} luby_visibility;

// ------------------------------- Lexer ------------------------------------

typedef enum luby_token_kind {
    LUBY_TOK_EOF = 0,
    LUBY_TOK_ERROR,
    LUBY_TOK_NEWLINE,

    // Literals
    LUBY_TOK_IDENTIFIER,
    LUBY_TOK_CONSTANT,
    LUBY_TOK_IVAR,
    LUBY_TOK_CVAR,
    LUBY_TOK_GVAR,
    LUBY_TOK_INTEGER,
    LUBY_TOK_FLOAT,
    LUBY_TOK_STRING,
    LUBY_TOK_STRING_PART,     // string segment before/between interpolations
    LUBY_TOK_STRING_END,      // final string segment after last interpolation
    LUBY_TOK_INTERP_START,    // #{ - start of interpolation
    LUBY_TOK_INTERP_END,      // } - end of interpolation (context-sensitive)
    LUBY_TOK_SYMBOL,

    // Keywords
    LUBY_TOK_CLASS,
    LUBY_TOK_MODULE,
    LUBY_TOK_DEF,
    LUBY_TOK_END,
    LUBY_TOK_IF,
    LUBY_TOK_ELSIF,
    LUBY_TOK_ELSE,
    LUBY_TOK_UNLESS,
    LUBY_TOK_WHILE,
    LUBY_TOK_UNTIL,
    LUBY_TOK_FOR,
    LUBY_TOK_IN,
    LUBY_TOK_CASE,
    LUBY_TOK_WHEN,
    LUBY_TOK_THEN,
    LUBY_TOK_DO,
    LUBY_TOK_YIELD,
    LUBY_TOK_RETURN,
    LUBY_TOK_BREAK,
    LUBY_TOK_NEXT,
    LUBY_TOK_REDO,
    LUBY_TOK_SUPER,
    LUBY_TOK_SELF,
    LUBY_TOK_TRUE,
    LUBY_TOK_FALSE,
    LUBY_TOK_NIL,
    LUBY_TOK_AND,
    LUBY_TOK_OR,
    LUBY_TOK_NOT,
    LUBY_TOK_BEGIN,
    LUBY_TOK_RESCUE,
    LUBY_TOK_ENSURE,
    LUBY_TOK_RAISE,
    LUBY_TOK_REQUIRE,
    LUBY_TOK_LOAD,
    LUBY_TOK_INCLUDE,
    LUBY_TOK_PREPEND,
    LUBY_TOK_EXTEND,
    LUBY_TOK_ATTR_READER,
    LUBY_TOK_ATTR_WRITER,
    LUBY_TOK_ATTR_ACCESSOR,
    LUBY_TOK_FILE,       // __FILE__
    LUBY_TOK_LINE,       // __LINE__
    LUBY_TOK_PRIVATE,
    LUBY_TOK_PUBLIC,
    LUBY_TOK_PROTECTED,
    LUBY_TOK_ALIAS,

    // Punctuation / operators
    LUBY_TOK_LPAREN,
    LUBY_TOK_RPAREN,
    LUBY_TOK_LBRACE,
    LUBY_TOK_RBRACE,
    LUBY_TOK_LBRACKET,
    LUBY_TOK_RBRACKET,
    LUBY_TOK_COMMA,
    LUBY_TOK_DOT,
    LUBY_TOK_COLON,
    LUBY_TOK_SEMI,
    LUBY_TOK_PIPE,
    LUBY_TOK_AMP,
    LUBY_TOK_PLUS,
    LUBY_TOK_MINUS,
    LUBY_TOK_STAR,
    LUBY_TOK_SLASH,
    LUBY_TOK_PERCENT,
    LUBY_TOK_CARET,
    LUBY_TOK_BANG,
    LUBY_TOK_TILDE,
    LUBY_TOK_EQ,
    LUBY_TOK_EQEQ,
    LUBY_TOK_NEQ,
    LUBY_TOK_LT,
    LUBY_TOK_LTE,
    LUBY_TOK_GT,
    LUBY_TOK_GTE,
    LUBY_TOK_ANDAND,
    LUBY_TOK_OROR,
    LUBY_TOK_SHL,
    LUBY_TOK_SHR,
    LUBY_TOK_QUESTION,
    LUBY_TOK_HASHROCKET, // =>
    LUBY_TOK_RANGE_INCL, // ..
    LUBY_TOK_RANGE_EXCL, // ...
    LUBY_TOK_SAFE_NAV,   // &.
    LUBY_TOK_COLONCOLON, // ::
    LUBY_TOK_PLUSEQ,     // +=
    LUBY_TOK_MINUSEQ,    // -=
    LUBY_TOK_STAREQ,     // *=
    LUBY_TOK_SLASHEQ,    // /=
    LUBY_TOK_PERCENTEQ,  // %=
    LUBY_TOK_ORASSIGN,   // ||=
    LUBY_TOK_ANDASSIGN,  // &&=
    LUBY_TOK_ARROW       // ->
} luby_token_kind;

typedef struct luby_token {
    luby_token_kind kind;
    luby_string_view lexeme;
    int line;
    int column;
} luby_token;

typedef struct luby_lexer {
    const char *src;
    size_t length;
    size_t pos;
    int line;
    int column;
    const char *filename;
    // Interpolation state
    int in_interp_string;         // 1 if inside an interpolated string
    int interp_brace_depth;       // track nested braces within #{}
    const char *interp_string_start; // start of current string segment
} luby_lexer;

// ------------------------------ AST/Parser ---------------------------------

typedef enum luby_ast_kind {
    LUBY_AST_NIL = 0,
    LUBY_AST_LITERAL,
    LUBY_AST_BOOL,
    LUBY_AST_INT,
    LUBY_AST_FLOAT,
    LUBY_AST_STRING,
    LUBY_AST_INTERP_STRING,   // interpolated string: list of string/expr parts
    LUBY_AST_SYMBOL,
    LUBY_AST_IDENT,
    LUBY_AST_CONST,
    LUBY_AST_IVAR,            // instance variable @x
    LUBY_AST_RANGE,           // 1..3 or 1...3

    LUBY_AST_ARRAY,
    LUBY_AST_HASH,
    LUBY_AST_PAIR,

    LUBY_AST_CALL,
    LUBY_AST_INDEX,
    LUBY_AST_DEF,
    LUBY_AST_CLASS,
    LUBY_AST_MODULE,
    LUBY_AST_BLOCK,
    LUBY_AST_LAMBDA,
    LUBY_AST_IF,
    LUBY_AST_TERNARY,         // cond ? then : else
    LUBY_AST_WHILE,
    LUBY_AST_RETURN,
    LUBY_AST_BREAK,
    LUBY_AST_NEXT,
    LUBY_AST_REDO,
    LUBY_AST_DEFAULT_PARAM,   // x = default_value
    LUBY_AST_SPLAT_PARAM,     // *args
    LUBY_AST_BLOCK_PARAM,     // &block
    LUBY_AST_BEGIN,
    LUBY_AST_ASSIGN,
    LUBY_AST_MULTI_ASSIGN,    // a, b = 1, 2
    LUBY_AST_IVAR_ASSIGN,     // @x = value
    LUBY_AST_INDEX_ASSIGN,
    LUBY_AST_BINARY,
    LUBY_AST_UNARY
} luby_ast_kind;

typedef struct luby_ast_node {
    luby_ast_kind kind;
    int line;
    int column;
    union {
        luby_string_view literal;
        struct { struct luby_ast_node **items; size_t count; } list;
        struct { struct luby_ast_node *left; struct luby_ast_node *right; } pair;
        struct { struct luby_ast_node *target; struct luby_ast_node *value; } assign;
        struct { struct luby_ast_node **targets; size_t target_count; struct luby_ast_node **values; size_t value_count; } multi_assign;
        struct { struct luby_ast_node *target; struct luby_ast_node *index; int safe; } index;
        struct { struct luby_ast_node *target; struct luby_ast_node *index; struct luby_ast_node *value; } index_assign;
        struct {
            struct luby_ast_node *recv;
            luby_string_view method;
            struct luby_ast_node **args;
            size_t argc;
            struct luby_ast_node *block;
            int safe;
        } call;
        struct { struct luby_ast_node **params; size_t param_count; struct luby_ast_node *body; } lambda;
        struct { luby_string_view name; luby_string_view super_name; struct luby_ast_node *body; } class_decl;
        struct { luby_string_view name; struct luby_ast_node *body; } module_decl;
        struct { luby_string_view name; struct luby_ast_node **params; size_t param_count; struct luby_ast_node *body; struct luby_ast_node *receiver; } defn;
        struct { struct luby_ast_node *cond; struct luby_ast_node *then_branch; struct luby_ast_node *else_branch; } if_stmt;
        struct { struct luby_ast_node *cond; struct luby_ast_node *body; } while_stmt;
        struct { struct luby_ast_node *value; } ret;
        struct { struct luby_ast_node *body; struct luby_ast_node *rescue_body; struct luby_ast_node *ensure_body; } begin;
        struct { struct luby_ast_node *start; struct luby_ast_node *end; int exclusive; } range;
        struct {
            luby_token_kind op;
            struct luby_ast_node *left;
            struct luby_ast_node *right;
        } binary;
        struct {
            luby_token_kind op;
            struct luby_ast_node *expr;
        } unary;
    } as;
} luby_ast_node;

typedef struct luby_parser {
    luby_lexer lex;
    luby_token current;
    luby_token next;
    luby_error *error;
} luby_parser;

LUBY_API luby_ast_node *luby_parse(luby_state *L, const char *code, size_t len, const char *filename, luby_error *out_error);

// ------------------------------ Bytecode -----------------------------------

typedef enum luby_op {
    LUBY_OP_NOOP = 0,
    LUBY_OP_CONST,
    LUBY_OP_POP,
    LUBY_OP_GET_LOCAL,
    LUBY_OP_SET_LOCAL,
    LUBY_OP_GET_GLOBAL,
    LUBY_OP_SET_GLOBAL,
    LUBY_OP_GET_INDEX,
    LUBY_OP_SAFE_INDEX,
    LUBY_OP_SET_INDEX,
    LUBY_OP_SET_BLOCK,
    LUBY_OP_GET_CLASS,
    LUBY_OP_SET_CLASS,
    LUBY_OP_MAKE_CLASS,
    LUBY_OP_MAKE_MODULE,
    LUBY_OP_DEF_METHOD,
    LUBY_OP_DEF_SINGLETON,  // define singleton method on receiver
    LUBY_OP_CALL,
    LUBY_OP_SAFE_CALL,
    LUBY_OP_RET,
    LUBY_OP_JUMP,
    LUBY_OP_JUMP_IF_FALSE,
    LUBY_OP_TRY,
    LUBY_OP_SET_ENSURE,
    LUBY_OP_ENTER_ENSURE,
    LUBY_OP_END_TRY,
    LUBY_OP_THROW,
    LUBY_OP_MAKE_ARRAY,
    LUBY_OP_MAKE_HASH,
    LUBY_OP_ADD,
    LUBY_OP_SUB,
    LUBY_OP_MUL,
    LUBY_OP_DIV,
    LUBY_OP_MOD,
    LUBY_OP_AND,
    LUBY_OP_OR,
    LUBY_OP_NOT,
    LUBY_OP_NEG,
    LUBY_OP_EQ,
    LUBY_OP_LT,
    LUBY_OP_LTE,
    LUBY_OP_GT,
    LUBY_OP_GTE,
    LUBY_OP_YIELD,
    LUBY_OP_CONCAT,       // concatenate N strings from stack
    LUBY_OP_GET_IVAR,     // get instance variable
    LUBY_OP_SET_IVAR,     // set instance variable
    LUBY_OP_MAKE_RANGE,   // create range from stack (a=exclusive flag)
    LUBY_OP_MULTI_UNPACK  // unpack for multi-assign (a=target_count, b=value_count)
} luby_op;

typedef struct luby_inst {
    uint8_t op;
    uint8_t a;
    uint16_t b;
    uint32_t c;
} luby_inst;

typedef struct luby_chunk {
    luby_inst *code;
    int *lines;
    size_t count;
    size_t capacity;
    luby_value *consts;
    size_t const_count;
    size_t const_capacity;
} luby_chunk;

typedef struct luby_compiler {
    luby_state *L;
    luby_chunk *chunk;
    int class_depth;
    int loop_depth;
    struct {
        size_t start;
        size_t body_start;
        size_t *breaks;
        size_t break_count;
        size_t break_capacity;
    } loops[16];
} luby_compiler;

typedef struct luby_vm_handler {
    uint32_t rescue_ip;
    uint32_t ensure_ip;
    luby_error pending;
    int pending_error;
    int phase; // 0=body,1=rescue,2=ensure
    int sp;
} luby_vm_handler;

typedef struct luby_vm_frame {
    luby_proc *proc;
    luby_chunk *chunk;
    size_t ip;
    const char *filename;
    luby_vm_handler handlers[16];
    int hcount;
    int stack_base;
    luby_value saved_block;
    luby_value saved_self;
    int self_existed;
    luby_value self_saved;
    int set_self;
    struct luby_class_obj *saved_method_class;
    const char *saved_method_name;
    int *param_existed;
    luby_value *param_saved;
    size_t param_count;
    int *local_existed;
    luby_value *local_saved;
    size_t local_count;
    // For new/initialize: return this value instead of method's return value
    luby_value return_override;
    int has_return_override;
} luby_vm_frame;

typedef struct luby_vm {
    luby_value *stack;
    int sp;
    int stack_capacity;
    luby_vm_frame *frames;
    int frame_count;
    int frame_capacity;
    int yielded;
    luby_value yield_value;
    int resume_pending;
    luby_value resume_value;
    int native_yield;
} luby_vm;

// ------------------------------ Allocator ----------------------------------

typedef void *(*luby_alloc_fn)(void *user, void *ptr, size_t size);

// ------------------------------ VFS API ------------------------------------

typedef int (*luby_vfs_exists_fn)(void *user, const char *path);
// Returns a newly allocated buffer and size; caller must free with host allocator.
typedef char *(*luby_vfs_read_fn)(void *user, const char *path, size_t *out_size);
// Optional stat (e.g., mtime); return 0 on success.
typedef int (*luby_vfs_stat_fn)(void *user, const char *path, uint64_t *out_mtime);

typedef struct luby_vfs {
    void *user;
    luby_vfs_exists_fn exists;
    luby_vfs_read_fn read;
    luby_vfs_stat_fn stat;
} luby_vfs;

// ------------------------------ Host API -----------------------------------

typedef struct luby_config {
    luby_alloc_fn alloc;  // if NULL, uses malloc/realloc/free
    void *alloc_user;
    luby_vfs vfs;
} luby_config;

// --------------------------- Native Bindings -------------------------------

typedef int (*luby_cfunc)(luby_state *L, int argc, const luby_value *argv, luby_value *out);
typedef void (*luby_finalizer)(void *user_data);

LUBY_API luby_state *luby_new(const luby_config *cfg);
LUBY_API void luby_free(luby_state *L);

LUBY_API int luby_eval(luby_state *L, const char *code, size_t len, const char *filename, luby_value *out);
LUBY_API int luby_require(luby_state *L, const char *path, luby_value *out);
LUBY_API int luby_load(luby_state *L, const char *path, luby_value *out);

LUBY_API luby_error luby_last_error(luby_state *L);
LUBY_API void luby_clear_error(luby_state *L);
LUBY_API const char *luby_error_code_string(luby_error_code code);
LUBY_API size_t luby_format_error(luby_state *L, char *buffer, size_t buffer_size);

// Globals API
LUBY_API void luby_set_global_value(luby_state *L, const char *name, luby_value v);
LUBY_API luby_value luby_get_global_value(luby_state *L, const char *name);

// Value helpers
LUBY_API luby_value luby_nil(void);
LUBY_API luby_value luby_bool(int b);
LUBY_API luby_value luby_int(int64_t v);
LUBY_API luby_value luby_float(double v);
LUBY_API luby_value luby_string(luby_state *L, const char *s, size_t len);
LUBY_API luby_value luby_symbol(luby_state *L, const char *s, size_t len);

// Array/Hash helpers (for embedders)
LUBY_API luby_value luby_array_new(luby_state *L);
LUBY_API size_t luby_array_len(luby_value arr);
LUBY_API int luby_array_get(luby_value arr, size_t index, luby_value *out);
LUBY_API int luby_array_set(luby_state *L, luby_value arr, size_t index, luby_value v);
LUBY_API int luby_array_push_value(luby_state *L, luby_value arr, luby_value v);

LUBY_API luby_value luby_hash_new(luby_state *L);
LUBY_API size_t luby_hash_len(luby_value h);
LUBY_API int luby_hash_get_value(luby_value h, luby_value key, luby_value *out);
LUBY_API int luby_hash_set_value(luby_state *L, luby_value h, luby_value key, luby_value value);

// Call into Luby-defined functions/methods from C
LUBY_API int luby_invoke_global(luby_state *L, const char *name, int argc, const luby_value *argv, luby_value *out);
LUBY_API int luby_invoke_method(luby_state *L, luby_value recv, const char *method, int argc, const luby_value *argv, luby_value *out);
// Call C-registered functions (existing API)
LUBY_API int luby_call(luby_state *L, luby_value recv, const char *method, int argc, const luby_value *argv, luby_value *out);

// Register native functions and modules
LUBY_API int luby_register_function(luby_state *L, const char *name, luby_cfunc fn);
LUBY_API int luby_register_module(luby_state *L, const char *name, luby_cfunc loader);

// Base standard library
LUBY_API void luby_open_base(luby_state *L);

// Search paths for require
LUBY_API void luby_add_search_path(luby_state *L, const char *path);
LUBY_API void luby_clear_search_paths(luby_state *L);

// Classes and userdata
LUBY_API luby_class *luby_define_class(luby_state *L, const char *name, const char *super_name);
LUBY_API int luby_define_method(luby_state *L, luby_class *cls, const char *name, luby_cfunc fn);
LUBY_API luby_value luby_new_userdata(luby_state *L, size_t size, luby_finalizer finalize);
LUBY_API void *luby_userdata_ptr(luby_value v);

// Coroutines
LUBY_API luby_coroutine *luby_coroutine_new(luby_state *L, luby_value func);
LUBY_API int luby_coroutine_resume(luby_state *L, luby_coroutine *co, int argc, const luby_value *argv, luby_value *out, int *out_yielded);
LUBY_API int luby_yield(luby_state *L, int argc, const luby_value *argv, luby_value *out);
LUBY_API int luby_native_yield(luby_state *L, luby_value value);

// Debug hooks
typedef enum luby_hook_event {
    LUBY_HOOK_LINE = 0,
    LUBY_HOOK_CALL,
    LUBY_HOOK_RETURN
} luby_hook_event;

typedef void (*luby_hook_fn)(luby_state *L, luby_hook_event event, const char *file, int line, void *user);

LUBY_API void luby_set_hook(luby_state *L, luby_hook_fn fn, void *user);

#ifdef __cplusplus
}
#endif

// ----------------------------- Implementation ------------------------------

#ifdef LUBY_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#if defined(__GNUC__) || defined(__clang__)
#define LUBY_UNUSED __attribute__((unused))
#else
#define LUBY_UNUSED
#endif

struct luby_state {
    luby_config cfg;
    luby_error last_error;
    luby_hook_fn hook;
    void *hook_user;
    luby_value current_block;
    luby_value current_class;
    luby_value current_self;
    luby_class_obj *current_method_class;
    const char *current_method_name;
    luby_visibility current_visibility;  // current visibility mode for method definitions
    luby_coroutine *current_coroutine;
    luby_vm *current_vm;
    size_t method_epoch;
    luby_string_view *global_names;
    luby_value *global_values;
    size_t global_count;
    size_t global_capacity;
    char **search_paths;
    size_t search_path_count;
    size_t search_path_capacity;
    char **loaded_paths;
    size_t loaded_count;
    size_t loaded_capacity;
    struct {
        const char **names;
        luby_cfunc *funcs;
        size_t count;
        size_t capacity;
    } cfuncs;
    char **symbol_names;
    size_t symbol_count;
    size_t symbol_capacity;
    uint64_t rng_state[2];  // xoroshiro128+ state for seeded RNG

    // Garbage collector
    luby_gc_obj *gc_objects;       // intrusive linked list of all GC objects
    size_t gc_alloc_count;         // number of GC objects allocated since last collection
    size_t gc_threshold;           // trigger collection when gc_alloc_count reaches this
    size_t gc_total;               // total number of live GC objects
    int gc_paused;                 // if non-zero, GC collection is inhibited
};

// ----------------------------- GC Header ----------------------------------

typedef enum luby_gc_type {
    LUBY_GC_STRING,
    LUBY_GC_ARRAY,
    LUBY_GC_HASH,
    LUBY_GC_CLASS,
    LUBY_GC_OBJECT,
    LUBY_GC_PROC,
    LUBY_GC_RANGE,
    LUBY_GC_COROUTINE,
    LUBY_GC_CMETHOD
} luby_gc_type;

struct luby_gc_obj {
    struct luby_gc_obj *gc_next;
    luby_gc_type gc_type;
    int gc_marked;
};

// String objects: GC header + char data (flexible array member)
// The luby_value.as.ptr for LUBY_T_STRING points to the data[] field,
// so existing code using (const char *)v.as.ptr works unchanged.
// Use LUBY_STRING_OBJ(ptr) to recover the header from a data pointer.
typedef struct luby_string_obj {
    luby_gc_obj gc;
    size_t length;
    char data[];   // flexible array member — string chars follow the header
} luby_string_obj;

#define LUBY_STRING_OBJ(cstr) \
    ((luby_string_obj *)((char *)(cstr) - offsetof(luby_string_obj, data)))

typedef struct luby_array {
    luby_gc_obj gc;
    size_t count;
    size_t capacity;
    luby_value *items;
    int frozen;
} luby_array;

typedef struct luby_hash_entry {
    luby_value key;
    luby_value value;
} luby_hash_entry;

typedef struct luby_hash {
    luby_gc_obj gc;
    size_t count;
    size_t capacity;
    luby_hash_entry *entries;
    int frozen;
} luby_hash;

struct luby_class_obj {
    luby_gc_obj gc;
    char *name;
    struct luby_class_obj *super;
    luby_hash *methods;
    luby_hash *singleton_methods;
    struct luby_class_obj **included_modules;
    size_t included_count;
    size_t included_capacity;
    struct luby_class_obj **prepended_modules;
    size_t prepended_count;
    size_t prepended_capacity;
    luby_hash *method_cache;
    size_t method_cache_epoch;
    luby_hash *singleton_cache;
    size_t singleton_cache_epoch;
    int frozen;
};

typedef struct luby_object {
    luby_gc_obj gc;
    luby_class_obj *klass;
    luby_hash *ivars;
    luby_hash *singleton_methods;
    int frozen;
    char **ivar_names;
    luby_value *ivar_values;
    size_t ivar_count;
} luby_object;

typedef struct luby_range {
    luby_gc_obj gc;
    luby_value start;
    luby_value end;
    int exclusive;
} luby_range;

struct luby_proc {
    luby_gc_obj gc;
    char **param_names;
    size_t param_count;
    luby_chunk *default_chunks;   // bytecode for default values (NULL if no default)
    int splat_index;              // index of *args param, or -1 if none
    int has_block_param;          // whether &block param exists
    char *block_param_name;       // name of block param (e.g. "block")
    char **local_names;           // names of local variables (assigned in body, excluding params)
    size_t local_count;           // number of local variable names
    luby_chunk chunk;
    int owned_by_chunk;
    luby_visibility visibility;   // method visibility
};

struct luby_coroutine {
    luby_gc_obj gc;
    luby_proc *proc;
    int done;
    int started;
    luby_vm vm;
};

struct luby_class {
    luby_class_obj *obj;
};

struct luby_module {
    luby_class_obj *obj;
};

typedef struct luby_cmethod {
    luby_gc_obj gc;
    luby_cfunc fn;
} luby_cmethod;

static void *luby_default_alloc(void *user, void *ptr, size_t size) {
    (void)user;
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, size);
}

static void *luby_alloc_raw(luby_state *L, void *ptr, size_t size) {
    luby_alloc_fn fn = L && L->cfg.alloc ? L->cfg.alloc : luby_default_alloc;
    void *user = L ? L->cfg.alloc_user : NULL;
    return fn(user, ptr, size);
}

// ------------------------------ GC Core ------------------------------------

#define LUBY_GC_INITIAL_THRESHOLD 256

static void luby_gc_collect(luby_state *L);

// Track a GC object: link into intrusive list and bump counters.
static void luby_gc_track(luby_state *L, luby_gc_obj *obj, luby_gc_type type) {
    obj->gc_type = type;
    obj->gc_marked = 0;
    obj->gc_next = L->gc_objects;
    L->gc_objects = obj;
    L->gc_alloc_count++;
    L->gc_total++;
}

// Allocate a GC-tracked object of given size and type.  Triggers collection
// when the allocation counter reaches the threshold.
static void *luby_gc_alloc(luby_state *L, size_t size, luby_gc_type type) {
    if (!L->gc_paused && L->gc_alloc_count >= L->gc_threshold) {
        luby_gc_collect(L);
    }
    void *mem = luby_alloc_raw(L, NULL, size);
    if (!mem) return NULL;
    memset(mem, 0, size);
    luby_gc_track(L, (luby_gc_obj *)mem, type);
    return mem;
}

// Allocate a GC-tracked string with data copied in.
// Returns pointer to the char data (not the header), so it's a
// drop-in replacement for luby_dup_string in luby_value.as.ptr.
static char *luby_gc_alloc_string(luby_state *L, const char *data, size_t len) {
    if (!L->gc_paused && L->gc_alloc_count >= L->gc_threshold) {
        luby_gc_collect(L);
    }
    luby_string_obj *s = (luby_string_obj *)luby_alloc_raw(L, NULL, sizeof(luby_string_obj) + len + 1);
    if (!s) return NULL;
    memset(s, 0, sizeof(luby_string_obj));
    s->length = len;
    if (data) memcpy(s->data, data, len);
    s->data[len] = '\0';
    luby_gc_track(L, &s->gc, LUBY_GC_STRING);
    return s->data;
}

// ------------------------------ GC Mark ------------------------------------

static void luby_gc_mark_obj(luby_gc_obj *obj);
static void luby_gc_mark_value(luby_value v);

static void luby_gc_mark_obj(luby_gc_obj *obj) {
    if (!obj || obj->gc_marked) return;
    obj->gc_marked = 1;

    switch (obj->gc_type) {
        case LUBY_GC_STRING:
            // Strings have no references
            break;
        case LUBY_GC_ARRAY: {
            luby_array *arr = (luby_array *)obj;
            for (size_t i = 0; i < arr->count; i++) {
                luby_gc_mark_value(arr->items[i]);
            }
            break;
        }
        case LUBY_GC_HASH: {
            luby_hash *h = (luby_hash *)obj;
            for (size_t i = 0; i < h->count; i++) {
                luby_gc_mark_value(h->entries[i].key);
                luby_gc_mark_value(h->entries[i].value);
            }
            break;
        }
        case LUBY_GC_CLASS: {
            luby_class_obj *cls = (luby_class_obj *)obj;
            if (cls->super) luby_gc_mark_obj(&cls->super->gc);
            if (cls->methods) luby_gc_mark_obj(&cls->methods->gc);
            if (cls->singleton_methods) luby_gc_mark_obj(&cls->singleton_methods->gc);
            if (cls->method_cache) luby_gc_mark_obj(&cls->method_cache->gc);
            if (cls->singleton_cache) luby_gc_mark_obj(&cls->singleton_cache->gc);
            for (size_t i = 0; i < cls->included_count; i++) {
                if (cls->included_modules[i]) luby_gc_mark_obj(&cls->included_modules[i]->gc);
            }
            for (size_t i = 0; i < cls->prepended_count; i++) {
                if (cls->prepended_modules[i]) luby_gc_mark_obj(&cls->prepended_modules[i]->gc);
            }
            break;
        }
        case LUBY_GC_OBJECT: {
            luby_object *o = (luby_object *)obj;
            if (o->klass) luby_gc_mark_obj(&o->klass->gc);
            if (o->ivars) luby_gc_mark_obj(&o->ivars->gc);
            if (o->singleton_methods) luby_gc_mark_obj(&o->singleton_methods->gc);
            for (size_t i = 0; i < o->ivar_count; i++) {
                luby_gc_mark_value(o->ivar_values[i]);
            }
            break;
        }
        case LUBY_GC_PROC: {
            luby_proc *proc = (luby_proc *)obj;
            // Mark constants in the chunk (strings, procs, etc.)
            for (size_t i = 0; i < proc->chunk.const_count; i++) {
                luby_gc_mark_value(proc->chunk.consts[i]);
            }
            // Mark default value chunks
            if (proc->default_chunks) {
                for (size_t i = 0; i < proc->param_count; i++) {
                    for (size_t j = 0; j < proc->default_chunks[i].const_count; j++) {
                        luby_gc_mark_value(proc->default_chunks[i].consts[j]);
                    }
                }
            }
            break;
        }
        case LUBY_GC_RANGE: {
            luby_range *r = (luby_range *)obj;
            luby_gc_mark_value(r->start);
            luby_gc_mark_value(r->end);
            break;
        }
        case LUBY_GC_COROUTINE: {
            luby_coroutine *co = (luby_coroutine *)obj;
            if (co->proc) luby_gc_mark_obj(&co->proc->gc);
            // Mark the coroutine's VM stack
            for (int i = 0; i < co->vm.sp; i++) {
                luby_gc_mark_value(co->vm.stack[i]);
            }
            // Mark saved values in coroutine VM frames
            for (int i = 0; i < co->vm.frame_count; i++) {
                luby_vm_frame *fr = &co->vm.frames[i];
                luby_gc_mark_value(fr->saved_block);
                luby_gc_mark_value(fr->saved_self);
                luby_gc_mark_value(fr->self_saved);
                if (fr->proc) luby_gc_mark_obj(&fr->proc->gc);
                for (size_t j = 0; j < fr->param_count; j++) {
                    if (fr->param_existed && fr->param_existed[j] >= 0) {
                        luby_gc_mark_value(fr->param_saved[j]);
                    }
                }
                for (size_t j = 0; j < fr->local_count; j++) {
                    if (fr->local_existed && fr->local_existed[j] >= 0) {
                        luby_gc_mark_value(fr->local_saved[j]);
                    }
                }
            }
            luby_gc_mark_value(co->vm.yield_value);
            luby_gc_mark_value(co->vm.resume_value);
            break;
        }
        case LUBY_GC_CMETHOD:
            // C methods have no references
            break;
    }
}

static void luby_gc_mark_value(luby_value v) {
    switch (v.type) {
        case LUBY_T_STRING:
            if (v.as.ptr) luby_gc_mark_obj(&LUBY_STRING_OBJ(v.as.ptr)->gc);
            break;
        case LUBY_T_ARRAY:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_array *)v.as.ptr)->gc);
            break;
        case LUBY_T_HASH:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_hash *)v.as.ptr)->gc);
            break;
        case LUBY_T_CLASS:
        case LUBY_T_MODULE:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_class_obj *)v.as.ptr)->gc);
            break;
        case LUBY_T_OBJECT:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_object *)v.as.ptr)->gc);
            break;
        case LUBY_T_PROC:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_proc *)v.as.ptr)->gc);
            break;
        case LUBY_T_RANGE:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_range *)v.as.ptr)->gc);
            break;
        case LUBY_T_CMETHOD:
            if (v.as.ptr) luby_gc_mark_obj(&((luby_cmethod *)v.as.ptr)->gc);
            break;
        default:
            // NIL, BOOL, INT, FLOAT, SYMBOL — no heap allocation or interned
            break;
    }
}

// Mark all roots: globals, VM stack, frame saved values, current_* pointers
static void luby_gc_mark_roots(luby_state *L) {
    // Global values
    for (size_t i = 0; i < L->global_count; i++) {
        luby_gc_mark_value(L->global_values[i]);
    }
    // Current block, class, self
    luby_gc_mark_value(L->current_block);
    luby_gc_mark_value(L->current_class);
    luby_gc_mark_value(L->current_self);
    if (L->current_method_class) luby_gc_mark_obj(&L->current_method_class->gc);
    // Current VM stack and frames
    if (L->current_vm) {
        luby_vm *vm = L->current_vm;
        for (int i = 0; i < vm->sp; i++) {
            luby_gc_mark_value(vm->stack[i]);
        }
        for (int i = 0; i < vm->frame_count; i++) {
            luby_vm_frame *fr = &vm->frames[i];
            luby_gc_mark_value(fr->saved_block);
            luby_gc_mark_value(fr->saved_self);
            luby_gc_mark_value(fr->self_saved);
            if (fr->proc) luby_gc_mark_obj(&fr->proc->gc);
            if (fr->chunk) {
                for (size_t j = 0; j < fr->chunk->const_count; j++) {
                    luby_gc_mark_value(fr->chunk->consts[j]);
                }
            }
            for (size_t j = 0; j < fr->param_count; j++) {
                if (fr->param_existed && fr->param_existed[j] >= 0) {
                    luby_gc_mark_value(fr->param_saved[j]);
                }
            }
            for (size_t j = 0; j < fr->local_count; j++) {
                if (fr->local_existed && fr->local_existed[j] >= 0) {
                    luby_gc_mark_value(fr->local_saved[j]);
                }
            }
        }
        luby_gc_mark_value(vm->yield_value);
        luby_gc_mark_value(vm->resume_value);
    }
    // Current coroutine
    if (L->current_coroutine) {
        luby_gc_mark_obj(&L->current_coroutine->gc);
    }
}

// ------------------------------ GC Sweep -----------------------------------

static void luby_vm_free(luby_state *L, luby_vm *vm);
static void luby_proc_free(luby_state *L, luby_proc *proc);

static void luby_gc_free_obj(luby_state *L, luby_gc_obj *obj) {
    switch (obj->gc_type) {
        case LUBY_GC_STRING:
            // String obj is the allocation (data[] is flexible member)
            luby_alloc_raw(L, obj, 0);
            break;
        case LUBY_GC_ARRAY: {
            luby_array *arr = (luby_array *)obj;
            if (arr->items) luby_alloc_raw(L, arr->items, 0);
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_HASH: {
            luby_hash *h = (luby_hash *)obj;
            if (h->entries) luby_alloc_raw(L, h->entries, 0);
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_CLASS: {
            luby_class_obj *cls = (luby_class_obj *)obj;
            if (cls->name) luby_alloc_raw(L, cls->name, 0);
            if (cls->included_modules) luby_alloc_raw(L, cls->included_modules, 0);
            if (cls->prepended_modules) luby_alloc_raw(L, cls->prepended_modules, 0);
            // methods, singleton_methods, method_cache, singleton_cache are GC hashes
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_OBJECT: {
            luby_object *o = (luby_object *)obj;
            if (o->ivar_names) {
                for (size_t i = 0; i < o->ivar_count; i++) {
                    luby_alloc_raw(L, o->ivar_names[i], 0);
                }
                luby_alloc_raw(L, o->ivar_names, 0);
            }
            if (o->ivar_values) luby_alloc_raw(L, o->ivar_values, 0);
            // ivars, singleton_methods are GC hashes
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_PROC: {
            luby_proc *proc = (luby_proc *)obj;
            luby_proc_free(L, proc);
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_RANGE:
            luby_alloc_raw(L, obj, 0);
            break;
        case LUBY_GC_COROUTINE: {
            luby_coroutine *co = (luby_coroutine *)obj;
            luby_vm_free(L, &co->vm);
            luby_alloc_raw(L, obj, 0);
            break;
        }
        case LUBY_GC_CMETHOD:
            luby_alloc_raw(L, obj, 0);
            break;
    }
}

static void luby_gc_sweep(luby_state *L) {
    luby_gc_obj **p = &L->gc_objects;
    while (*p) {
        if (!(*p)->gc_marked) {
            luby_gc_obj *unreached = *p;
            *p = unreached->gc_next;
            luby_gc_free_obj(L, unreached);
            L->gc_total--;
        } else {
            (*p)->gc_marked = 0;
            p = &(*p)->gc_next;
        }
    }
}

static void luby_gc_collect(luby_state *L) {
    if (!L || L->gc_paused) return;
    luby_gc_mark_roots(L);
    luby_gc_sweep(L);
    // Grow threshold: next collection after at least as many live objects again
    L->gc_threshold = L->gc_total < LUBY_GC_INITIAL_THRESHOLD ? LUBY_GC_INITIAL_THRESHOLD : L->gc_total * 2;
    L->gc_alloc_count = 0;
}

static void luby_set_error(luby_state *L, luby_error_code code, const char *message, const char *file, int line, int column) {
    if (!L) return;
    L->last_error.code = code;
    L->last_error.message = message;
    L->last_error.file = file;
    L->last_error.line = line;
    L->last_error.column = column;
}

static void luby_hash_clear(luby_hash *h) {
    if (!h) return;
    h->count = 0;
}

static int luby_string_view_eq(luby_string_view a, luby_string_view b) {
    if (a.length != b.length) return 0;
    if (a.data == b.data) return 1;
    return strncmp(a.data, b.data, a.length) == 0;
}

static int luby_cstr_eq(const char *a, const char *b) {
    if (!a || !b) return 0;
    return strcmp(a, b) == 0;
}

static char *luby_dup_string(luby_state *L, const char *data, size_t len);
static int luby_call_method(luby_state *L, luby_class_obj *cls, const char *name, luby_proc *proc, luby_value recv, int argc, const luby_value *argv, luby_value *out);

static int luby_string_list_contains(char **list, size_t count, const char *s) {
    for (size_t i = 0; i < count; i++) {
        if (list[i] && s && strcmp(list[i], s) == 0) return 1;
    }
    return 0;
}

static void luby_string_list_add(luby_state *L, char ***list, size_t *count, size_t *capacity, const char *s) {
    if (!L || !list || !count || !capacity || !s) return;
    if (*count + 1 > *capacity) {
        size_t new_cap = *capacity < 8 ? 8 : (*capacity) * 2;
        char **nl = (char **)luby_alloc_raw(L, *list, new_cap * sizeof(char *));
        if (!nl) return;
        *list = nl;
        *capacity = new_cap;
    }
    (*list)[*count] = luby_dup_string(L, s, strlen(s));
    (*count)++;
}

static const char *luby_intern_symbol(luby_state *L, const char *s, size_t len) {
    if (!L || !s) return NULL;
    if (len == 0) len = strlen(s);
    for (size_t i = 0; i < L->symbol_count; i++) {
        const char *name = L->symbol_names[i];
        if (!name) continue;
        if (strlen(name) == len && strncmp(name, s, len) == 0) return name;
    }
    if (L->symbol_count + 1 > L->symbol_capacity) {
        size_t new_cap = L->symbol_capacity < 16 ? 16 : L->symbol_capacity * 2;
        char **nl = (char **)luby_alloc_raw(L, L->symbol_names, new_cap * sizeof(char *));
        if (!nl) return NULL;
        L->symbol_names = nl;
        L->symbol_capacity = new_cap;
    }
    char *copy = (char *)luby_alloc_raw(L, NULL, len + 1);
    if (!copy) return NULL;
    memcpy(copy, s, len);
    copy[len] = '\0';
    L->symbol_names[L->symbol_count++] = copy;
    return copy;
}

static int luby_find_global(luby_state *L, luby_string_view name) {
    for (size_t i = 0; i < L->global_count; i++) {
        if (luby_string_view_eq(L->global_names[i], name)) return (int)i;
    }
    return -1;
}

static luby_value luby_get_global(luby_state *L, luby_string_view name) {
    int idx = luby_find_global(L, name);
    if (idx >= 0) return L->global_values[idx];
    return luby_nil();
}

static void luby_set_global(luby_state *L, luby_string_view name, luby_value v) {
    int idx = luby_find_global(L, name);
    if (idx >= 0) {
        L->global_values[idx] = v;
        return;
    }
    if (L->global_count + 1 > L->global_capacity) {
        size_t new_cap = L->global_capacity < 8 ? 8 : L->global_capacity * 2;
        luby_string_view *nn = (luby_string_view *)luby_alloc_raw(L, L->global_names, new_cap * sizeof(luby_string_view));
        luby_value *nv = (luby_value *)luby_alloc_raw(L, L->global_values, new_cap * sizeof(luby_value));
        if (!nn || !nv) return;
        L->global_names = nn;
        L->global_values = nv;
        L->global_capacity = new_cap;
    }
    char *cpy = luby_dup_string(L, name.data, name.length);
    if (!cpy) return;
    luby_string_view nsv = { cpy, name.length };
    L->global_names[L->global_count] = nsv;
    L->global_values[L->global_count] = v;
    L->global_count++;
}

static void luby_remove_global(luby_state *L, luby_string_view name) {
    if (!L) return;
    int idx = luby_find_global(L, name);
    if (idx < 0) return;
    luby_alloc_raw(L, (void *)L->global_names[idx].data, 0);
    for (size_t i = (size_t)idx + 1; i < L->global_count; i++) {
        L->global_names[i - 1] = L->global_names[i];
        L->global_values[i - 1] = L->global_values[i];
    }
    L->global_count--;
}

static int luby_value_eq(luby_value a, luby_value b) {
    if (a.type != b.type) return 0;
    switch (a.type) {
        case LUBY_T_NIL: return 1;
        case LUBY_T_BOOL: return a.as.b == b.as.b;
        case LUBY_T_INT: return a.as.i == b.as.i;
        case LUBY_T_FLOAT: return a.as.f == b.as.f;
        case LUBY_T_STRING:
        case LUBY_T_SYMBOL:
            if (!a.as.ptr || !b.as.ptr) return a.as.ptr == b.as.ptr;
            return strcmp((const char *)a.as.ptr, (const char *)b.as.ptr) == 0;
        default: return a.as.ptr == b.as.ptr;
    }
}

static int luby_value_is_frozen(luby_value v) {
    switch (v.type) {
        case LUBY_T_NIL:
        case LUBY_T_BOOL:
        case LUBY_T_INT:
        case LUBY_T_FLOAT:
        case LUBY_T_STRING:
        case LUBY_T_SYMBOL:
            return 1;
        case LUBY_T_ARRAY:
            return v.as.ptr ? ((luby_array *)v.as.ptr)->frozen : 0;
        case LUBY_T_HASH:
            return v.as.ptr ? ((luby_hash *)v.as.ptr)->frozen : 0;
        case LUBY_T_OBJECT:
            return v.as.ptr ? ((luby_object *)v.as.ptr)->frozen : 0;
        case LUBY_T_CLASS:
        case LUBY_T_MODULE:
            return v.as.ptr ? ((luby_class_obj *)v.as.ptr)->frozen : 0;
        default:
            return 0;
    }
}

static int luby_hash_get_value_found(luby_value h, luby_value key, luby_value *out, int *found) {
    if (found) *found = 0;
    if (h.type != LUBY_T_HASH || !h.as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *hh = (luby_hash *)h.as.ptr;
    for (size_t i = 0; i < hh->count; i++) {
        if (luby_value_eq(hh->entries[i].key, key)) {
            if (out) *out = hh->entries[i].value;
            if (found) *found = 1;
            return (int)LUBY_E_OK;
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static luby_hash *luby_hash_new_heap(luby_state *L) {
    luby_hash *h = (luby_hash *)luby_gc_alloc(L, sizeof(luby_hash), LUBY_GC_HASH);
    if (!h) return NULL;
    h->count = 0;
    h->capacity = 0;
    h->entries = NULL;
    h->frozen = 0;
    return h;
}

static luby_class_obj *luby_class_new(luby_state *L, const char *name, luby_class_obj *super) {
    // Pause GC during construction - sub-allocations could trigger collection
    // before this object is reachable from roots
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    luby_class_obj *cls = (luby_class_obj *)luby_gc_alloc(L, sizeof(luby_class_obj), LUBY_GC_CLASS);
    if (!cls) { L->gc_paused = was_paused; return NULL; }
    cls->name = name ? luby_dup_string(L, name, strlen(name)) : NULL;
    cls->super = super;
    cls->methods = luby_hash_new_heap(L);
    cls->singleton_methods = luby_hash_new_heap(L);
    cls->included_modules = NULL;
    cls->included_count = 0;
    cls->included_capacity = 0;
    cls->prepended_modules = NULL;
    cls->prepended_count = 0;
    cls->prepended_capacity = 0;
    cls->method_cache = luby_hash_new_heap(L);
    cls->method_cache_epoch = 0;
    cls->singleton_cache = luby_hash_new_heap(L);
    cls->singleton_cache_epoch = 0;
    cls->frozen = 0;
    L->gc_paused = was_paused;
    return cls;
}

static luby_object *luby_object_new(luby_state *L, luby_class_obj *cls) {
    // Pause GC during construction - sub-allocations could trigger collection
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    luby_object *obj = (luby_object *)luby_gc_alloc(L, sizeof(luby_object), LUBY_GC_OBJECT);
    if (!obj) { L->gc_paused = was_paused; return NULL; }
    obj->klass = cls;
    obj->ivars = luby_hash_new_heap(L);
    obj->singleton_methods = luby_hash_new_heap(L);
    obj->frozen = 0;
    obj->ivar_names = NULL;
    obj->ivar_values = NULL;
    obj->ivar_count = 0;
    L->gc_paused = was_paused;
    return obj;
}

static void luby_class_set_method(luby_state *L, luby_class_obj *cls, const char *name, luby_proc *proc) {
    if (!cls || !name) return;
    if (cls->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return;
    }
    luby_value key = luby_symbol(L, name, 0);
    luby_value val; val.type = LUBY_T_PROC; val.as.ptr = proc;
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->methods }, key, val);
    if (L) L->method_epoch++;
}

static int luby_class_add_include(luby_state *L, luby_class_obj *cls, luby_class_obj *mod) {
    if (!cls || !mod) return 0;
    if (cls->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return 0;
    }
    for (size_t i = 0; i < cls->included_count; i++) {
        if (cls->included_modules[i] == mod) return 1;
    }
    if (cls->included_count + 1 > cls->included_capacity) {
        size_t new_cap = cls->included_capacity < 4 ? 4 : cls->included_capacity * 2;
        luby_class_obj **nm = (luby_class_obj **)luby_alloc_raw(L, cls->included_modules, new_cap * sizeof(luby_class_obj *));
        if (!nm) return 0;
        cls->included_modules = nm;
        cls->included_capacity = new_cap;
    }
    cls->included_modules[cls->included_count++] = mod;
    if (L) L->method_epoch++;
    return 1;
}

static int luby_class_add_prepend(luby_state *L, luby_class_obj *cls, luby_class_obj *mod) {
    if (!cls || !mod) return 0;
    if (cls->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return 0;
    }
    for (size_t i = 0; i < cls->prepended_count; i++) {
        if (cls->prepended_modules[i] == mod) return 1;
    }
    if (cls->prepended_count + 1 > cls->prepended_capacity) {
        size_t new_cap = cls->prepended_capacity < 4 ? 4 : cls->prepended_capacity * 2;
        luby_class_obj **nm = (luby_class_obj **)luby_alloc_raw(L, cls->prepended_modules, new_cap * sizeof(luby_class_obj *));
        if (!nm) return 0;
        cls->prepended_modules = nm;
        cls->prepended_capacity = new_cap;
    }
    cls->prepended_modules[cls->prepended_count++] = mod;
    if (L) L->method_epoch++;
    return 1;
}

// Check if calling a method with given visibility is allowed
static int LUBY_UNUSED luby_check_visibility(luby_state *L, luby_proc *proc, luby_value recv, int has_explicit_receiver) {
    if (!proc) return 1;  // No method, will fail elsewhere
    if (proc->visibility == LUBY_VIS_PUBLIC) return 1;  // Public methods always allowed
    
    // Private methods can only be called without explicit receiver (implicit self)
    if (proc->visibility == LUBY_VIS_PRIVATE) {
        if (has_explicit_receiver) {
            luby_set_error(L, LUBY_E_RUNTIME, "private method called", NULL, 0, 0);
            return 0;
        }
        return 1;
    }
    
    // Protected methods can be called by instances of the same class hierarchy as recv
    if (proc->visibility == LUBY_VIS_PROTECTED) {
        // Check if current_self is an instance of the same class hierarchy as recv
        if (L->current_self.type == LUBY_T_OBJECT && recv.type == LUBY_T_OBJECT) {
            luby_object *self_obj = (luby_object *)L->current_self.as.ptr;
            luby_object *recv_obj = (luby_object *)recv.as.ptr;
            if (self_obj && recv_obj) {
                // Check if they share the same class hierarchy
                luby_class_obj *self_cls = self_obj->klass;
                luby_class_obj *recv_cls = recv_obj->klass;
                // Simple check: same class or one is subclass of the other
                while (self_cls) {
                    if (self_cls == recv_cls) return 1;
                    self_cls = self_cls->super;
                }
                self_cls = self_obj->klass;
                while (recv_cls) {
                    if (recv_cls == self_cls) return 1;
                    recv_cls = recv_cls->super;
                }
            }
        }
        luby_set_error(L, LUBY_E_RUNTIME, "protected method called", NULL, 0, 0);
        return 0;
    }
    
    return 1;
}

static void luby_class_set_singleton_method(luby_state *L, luby_class_obj *cls, const char *name, luby_proc *proc) {
    if (!cls || !name) return;
    if (cls->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return;
    }
    luby_value key = luby_symbol(L, name, 0);
    luby_value val; val.type = LUBY_T_PROC; val.as.ptr = proc;
    if (!cls->singleton_methods) cls->singleton_methods = luby_hash_new_heap(L);
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->singleton_methods }, key, val);
    if (L) L->method_epoch++;
}

static void luby_object_set_singleton_method(luby_state *L, luby_object *obj, const char *name, luby_proc *proc) {
    if (!obj || !name) return;
    if (obj->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return;
    }
    luby_value key = luby_symbol(L, name, 0);
    luby_value val; val.type = LUBY_T_PROC; val.as.ptr = proc;
    if (!obj->singleton_methods) obj->singleton_methods = luby_hash_new_heap(L);
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->singleton_methods }, key, val);
    if (L) L->method_epoch++;
}

static int luby_class_merge_methods(luby_state *L, luby_class_obj *target, luby_class_obj *source) {
    if (!target || !source || !source->methods) return 0;
    if (target->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return 0;
    }
    if (!target->methods) {
        target->methods = luby_hash_new_heap(L);
        if (!target->methods) return 0;
    }
    luby_hash *sm = source->methods;
    for (size_t i = 0; i < sm->count; i++) {
        luby_hash_entry *e = &sm->entries[i];
        luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = target->methods }, e->key, e->value);
    }
    if (L) L->method_epoch++;
    return 1;
}

static luby_value luby_class_lookup_method(luby_state *L, luby_class_obj *cls, const char *name);

static luby_proc *luby_class_get_method(luby_state *L, luby_class_obj *cls, const char *name) {
    luby_value v = luby_class_lookup_method(L, cls, name);
    if (v.type == LUBY_T_PROC) return (luby_proc *)v.as.ptr;
    return NULL;
}

static luby_value luby_class_lookup_method(luby_state *L, luby_class_obj *cls, const char *name) {
    if (!cls || !name) return luby_nil();
    luby_value key; key.type = LUBY_T_SYMBOL; key.as.ptr = (void *)name;

    if (L && cls->method_cache) {
        if (cls->method_cache_epoch != L->method_epoch) {
            luby_hash_clear(cls->method_cache);
            cls->method_cache_epoch = L->method_epoch;
        }
        int found = 0;
        luby_value cached = luby_nil();
        luby_hash_get_value_found((luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->method_cache }, key, &cached, &found);
        if (found) {
            return cached;
        }
    }

    luby_value result = luby_nil();
    luby_value out = luby_nil();
    if (cls->prepended_modules && cls->prepended_count > 0) {
        for (size_t i = cls->prepended_count; i > 0; i--) {
            luby_class_obj *mod = cls->prepended_modules[i - 1];
            luby_value mv = luby_class_lookup_method(L, mod, name);
            if (mv.type == LUBY_T_PROC || mv.type == LUBY_T_CMETHOD) { result = mv; break; }
        }
    }
    if (result.type == LUBY_T_NIL && cls->methods) {
        luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->methods }, key, &out);
        if (out.type == LUBY_T_PROC || out.type == LUBY_T_CMETHOD) result = out;
    }
    if (result.type == LUBY_T_NIL && cls->included_modules && cls->included_count > 0) {
        for (size_t i = cls->included_count; i > 0; i--) {
            luby_class_obj *mod = cls->included_modules[i - 1];
            luby_value mv = luby_class_lookup_method(L, mod, name);
            if (mv.type == LUBY_T_PROC || mv.type == LUBY_T_CMETHOD) { result = mv; break; }
        }
    }
    if (result.type == LUBY_T_NIL && cls->super) result = luby_class_lookup_method(L, cls->super, name);

    if (L && cls->method_cache) {
        luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->method_cache }, key, result);
    }
    return result;
}

static luby_proc *luby_class_get_singleton_method(luby_state *L, luby_class_obj *cls, const char *name) {
    if (!cls || !name) return NULL;
    luby_value key; key.type = LUBY_T_SYMBOL; key.as.ptr = (void *)name;

    if (L && cls->singleton_cache) {
        if (cls->singleton_cache_epoch != L->method_epoch) {
            luby_hash_clear(cls->singleton_cache);
            cls->singleton_cache_epoch = L->method_epoch;
        }
        int found = 0;
        luby_value cached = luby_nil();
        luby_hash_get_value_found((luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->singleton_cache }, key, &cached, &found);
        if (found) {
            if (cached.type == LUBY_T_PROC) return (luby_proc *)cached.as.ptr;
            return NULL;
        }
    }

    luby_value out = luby_nil();
    if (cls->singleton_methods) {
        luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->singleton_methods }, key, &out);
    }
    luby_proc *result = (out.type == LUBY_T_PROC) ? (luby_proc *)out.as.ptr : NULL;

    if (L && cls->singleton_cache) {
        luby_value val = result ? (luby_value){ .type = LUBY_T_PROC, .as.ptr = result } : luby_nil();
        luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->singleton_cache }, key, val);
    }
    return result;
}

static luby_proc *luby_object_get_singleton_method(luby_object *obj, const char *name) {
    if (!obj || !name) return NULL;
    luby_value key; key.type = LUBY_T_SYMBOL; key.as.ptr = (void *)name;
    luby_value out = luby_nil();
    if (obj->singleton_methods) {
        luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->singleton_methods }, key, &out);
    }
    if (out.type == LUBY_T_PROC) return (luby_proc *)out.as.ptr;
    return NULL;
}

static luby_proc *luby_class_get_method_from(luby_state *L, luby_class_obj *cls, const char *name) {
    if (!cls) return NULL;
    return luby_class_get_method(L, cls, name);
}

static int luby_class_has_method(luby_state *L, luby_class_obj *cls, const char *name) {
    return luby_class_get_method(L, cls, name) != NULL;
}

static int LUBY_UNUSED luby_call_method_direct(luby_state *L, luby_class_obj *cls, const char *name, luby_value recv, int argc, const luby_value *argv, luby_value *out) {
    if (!cls || !name) return (int)LUBY_E_TYPE;
    luby_proc *m = luby_class_get_method(L, cls, name);
    if (!m) return (int)LUBY_E_NAME;
    return luby_call_method(L, cls, name, m, recv, argc, argv, out);
}

static int luby_call_method_by_name(luby_state *L, luby_value recv, const char *name, int argc, const luby_value *argv, luby_value *out) {
    if (!L || !name) return (int)LUBY_E_TYPE;
    if (!(recv.type == LUBY_T_OBJECT || recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE)) return (int)LUBY_E_TYPE;
    luby_class_obj *cls = NULL;
    if (recv.type == LUBY_T_OBJECT && recv.as.ptr) cls = ((luby_object *)recv.as.ptr)->klass;
    else cls = (luby_class_obj *)recv.as.ptr;
    if (!cls) return (int)LUBY_E_TYPE;

    luby_proc *m = NULL;
    if (recv.type == LUBY_T_OBJECT && recv.as.ptr) {
        m = luby_object_get_singleton_method((luby_object *)recv.as.ptr, name);
    } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
        m = luby_class_get_singleton_method(L, (luby_class_obj *)recv.as.ptr, name);
    }
    if (!m) m = luby_class_get_method(L, cls, name);
    if (m) return luby_call_method(L, cls, name, m, recv, argc, argv, out);

    luby_proc *mm = luby_class_get_method(L, cls, "method_missing");
    if (mm) {
        luby_value args[16];
        int use = argc + 1;
        if (use > 16) use = 16;
        luby_value namev; namev.type = LUBY_T_SYMBOL; namev.as.ptr = (void *)name;
        args[0] = namev;
        for (int i = 1; i < use; i++) args[i] = argv[i - 1];
        return luby_call_method(L, cls, "method_missing", mm, recv, use, args, out);
    }
    return (int)LUBY_E_NAME;
}

static int luby_call_hook_if_exists(luby_state *L, luby_value recv, const char *name, luby_value arg) {
    if (!L || !name) return 0;
    if (!(recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE)) return 0;
    luby_class_obj *cls = (luby_class_obj *)recv.as.ptr;
    luby_proc *m = luby_class_get_method(L, cls, name);
    if (!m) return 0;
    luby_value out = luby_nil();
    if (luby_call_method(L, cls, name, m, recv, 1, &arg, &out) != 0) return (int)LUBY_E_RUNTIME;
    return 1;
}

static int luby_is_truthy(luby_value v) {
    if (v.type == LUBY_T_NIL) return 0;
    if (v.type == LUBY_T_BOOL) return v.as.b != 0;
    return 1;
}

static luby_cfunc luby_find_cfunc(luby_state *L, const char *name) {
    for (size_t i = 0; i < L->cfuncs.count; i++) {
        if (luby_cstr_eq(L->cfuncs.names[i], name)) return L->cfuncs.funcs[i];
    }
    return NULL;
}

static const char *luby_type_name(luby_value v) {
    switch (v.type) {
        case LUBY_T_NIL: return "nil";
        case LUBY_T_BOOL: return "bool";
        case LUBY_T_INT: return "int";
        case LUBY_T_FLOAT: return "float";
        case LUBY_T_STRING: return "string";
        case LUBY_T_SYMBOL: return "symbol";
        case LUBY_T_ARRAY: return "array";
        case LUBY_T_HASH: return "hash";
        case LUBY_T_OBJECT: return "object";
        case LUBY_T_PROC: return "proc";
        case LUBY_T_CLASS: return "class";
        case LUBY_T_MODULE: return "module";
        default: return "unknown";
    }
}

// Convert a value to a string (for interpolation). Returns allocated string.
static char *luby_value_to_string(luby_state *L, luby_value v) {
    char buf[128];
    switch (v.type) {
        case LUBY_T_NIL:
            return luby_dup_string(L, "", 0);
        case LUBY_T_BOOL:
            return luby_dup_string(L, v.as.b ? "true" : "false", v.as.b ? 4 : 5);
        case LUBY_T_INT:
            snprintf(buf, sizeof(buf), "%lld", (long long)v.as.i);
            return luby_dup_string(L, buf, strlen(buf));
        case LUBY_T_FLOAT:
            snprintf(buf, sizeof(buf), "%g", v.as.f);
            return luby_dup_string(L, buf, strlen(buf));
        case LUBY_T_STRING:
            return luby_dup_string(L, v.as.ptr ? (const char *)v.as.ptr : "", v.as.ptr ? strlen((const char *)v.as.ptr) : 0);
        case LUBY_T_SYMBOL:
            return luby_dup_string(L, v.as.ptr ? (const char *)v.as.ptr : "", v.as.ptr ? strlen((const char *)v.as.ptr) : 0);
        case LUBY_T_ARRAY: {
            // For now, just return a placeholder
            luby_array *arr = (luby_array *)v.as.ptr;
            snprintf(buf, sizeof(buf), "[Array: %zu items]", arr ? arr->count : 0);
            return luby_dup_string(L, buf, strlen(buf));
        }
        case LUBY_T_HASH: {
            luby_hash *h = (luby_hash *)v.as.ptr;
            snprintf(buf, sizeof(buf), "{Hash: %zu items}", h ? h->count : 0);
            return luby_dup_string(L, buf, strlen(buf));
        }
        default:
            snprintf(buf, sizeof(buf), "#<%s>", luby_type_name(v));
            return luby_dup_string(L, buf, strlen(buf));
    }
}

static void luby_print_value(luby_value v) {
    switch (v.type) {
        case LUBY_T_NIL: printf("nil"); break;
        case LUBY_T_BOOL: printf(v.as.b ? "true" : "false"); break;
        case LUBY_T_INT: printf("%lld", (long long)v.as.i); break;
        case LUBY_T_FLOAT: printf("%g", v.as.f); break;
        case LUBY_T_STRING:
        case LUBY_T_SYMBOL:
            printf("%s", v.as.ptr ? (const char *)v.as.ptr : "");
            break;
        case LUBY_T_ARRAY: {
            luby_array *arr = (luby_array *)v.as.ptr;
            printf("[");
            if (arr) {
                for (size_t i = 0; i < arr->count; i++) {
                    if (i > 0) printf(", ");
                    luby_print_value(arr->items[i]);
                }
            }
            printf("]");
            break;
        }
        case LUBY_T_HASH: {
            luby_hash *h = (luby_hash *)v.as.ptr;
            printf("{");
            if (h) {
                for (size_t i = 0; i < h->count; i++) {
                    if (i > 0) printf(", ");
                    luby_print_value(h->entries[i].key);
                    printf("=>");
                    luby_print_value(h->entries[i].value);
                }
            }
            printf("}");
            break;
        }
        default:
            printf("<%s>", luby_type_name(v));
            break;
    }
}

// ------------------------------ Lexer Impl --------------------------------

static int luby_is_ident_start(int c) {
    return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int luby_is_ident(int c) {
    return luby_is_ident_start(c) || (c >= '0' && c <= '9') || c == '?' || c == '!';
}

static void luby_lexer_init(luby_lexer *lex, const char *src, size_t len, const char *filename) {
    lex->src = src ? src : "";
    lex->length = len ? len : (src ? strlen(src) : 0);
    lex->pos = 0;
    lex->line = 1;
    lex->column = 1;
    lex->filename = filename ? filename : "<input>";
    lex->in_interp_string = 0;
    lex->interp_brace_depth = 0;
    lex->interp_string_start = NULL;
}

static int luby_lexer_peek(luby_lexer *lex) {
    if (lex->pos >= lex->length) return 0;
    return (unsigned char)lex->src[lex->pos];
}

static int luby_lexer_peek_next(luby_lexer *lex) {
    if (lex->pos + 1 >= lex->length) return 0;
    return (unsigned char)lex->src[lex->pos + 1];
}

static int luby_lexer_advance(luby_lexer *lex) {
    if (lex->pos >= lex->length) return 0;
    char c = lex->src[lex->pos++];
    if (c == '\n') {
        lex->line++;
        lex->column = 1;
    } else {
        lex->column++;
    }
    return (unsigned char)c;
}

static void luby_lexer_skip_ws(luby_lexer *lex) {
    for (;;) {
        int c = luby_lexer_peek(lex);
        if (c == ' ' || c == '\t' || c == '\r') {
            luby_lexer_advance(lex);
            continue;
        }
        if (c == '#') {
            while (c && c != '\n') {
                luby_lexer_advance(lex);
                c = luby_lexer_peek(lex);
            }
            continue;
        }
        break;
    }
}

static luby_token luby_make_token(luby_token_kind kind, const char *start, size_t len, int line, int column) {
    luby_token tok;
    tok.kind = kind;
    tok.lexeme.data = start;
    tok.lexeme.length = len;
    tok.line = line;
    tok.column = column;
    return tok;
}

// Continue scanning an interpolated string after the expression in #{}
static luby_token luby_lexer_continue_interp_string(luby_lexer *lex) {
    int line = lex->line;
    int column = lex->column;
    const char *start = lex->src + lex->pos;

    while (lex->pos < lex->length) {
        int c = luby_lexer_peek(lex);
        if (c == '\\') {
            luby_lexer_advance(lex);
            if (luby_lexer_peek(lex)) luby_lexer_advance(lex);
            continue;
        }
        if (c == '"') {
            // End of string
            size_t len = (size_t)(lex->src + lex->pos - start);
            luby_lexer_advance(lex); // consume closing quote
            lex->in_interp_string = 0;
            return luby_make_token(LUBY_TOK_STRING_END, start, len, line, column);
        }
        if (c == '#' && luby_lexer_peek_next(lex) == '{') {
            // Another interpolation
            size_t len = (size_t)(lex->src + lex->pos - start);
            luby_lexer_advance(lex); // #
            luby_lexer_advance(lex); // {
            lex->interp_brace_depth = 1;
            return luby_make_token(LUBY_TOK_STRING_PART, start, len, line, column);
        }
        luby_lexer_advance(lex);
    }
    // Unterminated string
    lex->in_interp_string = 0;
    return luby_make_token(LUBY_TOK_ERROR, start, (size_t)(lex->src + lex->pos - start), line, column);
}

static luby_token luby_lexer_next(luby_lexer *lex) {
    // If we're inside an interpolated string and brace depth is 0, we just closed a #{}
    // Continue scanning the rest of the string
    if (lex->in_interp_string && lex->interp_brace_depth == 0) {
        return luby_lexer_continue_interp_string(lex);
    }

    luby_lexer_skip_ws(lex);
    int line = lex->line;
    int column = lex->column;
    const char *start = lex->src + lex->pos;
    int c = luby_lexer_advance(lex);
    if (c == 0) {
        return luby_make_token(LUBY_TOK_EOF, start, 0, line, column);
    }
    if (c == '\n') {
        return luby_make_token(LUBY_TOK_NEWLINE, start, 1, line, column);
    }

    // Identifiers and keywords
    if (luby_is_ident_start(c)) {
        while (luby_is_ident(luby_lexer_peek(lex))) luby_lexer_advance(lex);
        size_t len = (size_t)(lex->src + lex->pos - start);

        // keywords
        #define LUBY_KW(s, k) if (len == sizeof(s) - 1 && strncmp(start, s, len) == 0) return luby_make_token(k, start, len, line, column)
        LUBY_KW("class", LUBY_TOK_CLASS);
        LUBY_KW("module", LUBY_TOK_MODULE);
        LUBY_KW("def", LUBY_TOK_DEF);
        LUBY_KW("end", LUBY_TOK_END);
        LUBY_KW("if", LUBY_TOK_IF);
        LUBY_KW("elsif", LUBY_TOK_ELSIF);
        LUBY_KW("else", LUBY_TOK_ELSE);
        LUBY_KW("unless", LUBY_TOK_UNLESS);
        LUBY_KW("while", LUBY_TOK_WHILE);
        LUBY_KW("until", LUBY_TOK_UNTIL);
        LUBY_KW("for", LUBY_TOK_FOR);
        LUBY_KW("in", LUBY_TOK_IN);
        LUBY_KW("case", LUBY_TOK_CASE);
        LUBY_KW("when", LUBY_TOK_WHEN);
        LUBY_KW("then", LUBY_TOK_THEN);
        LUBY_KW("do", LUBY_TOK_DO);
        LUBY_KW("yield", LUBY_TOK_YIELD);
        LUBY_KW("return", LUBY_TOK_RETURN);
        LUBY_KW("break", LUBY_TOK_BREAK);
        LUBY_KW("next", LUBY_TOK_NEXT);
        LUBY_KW("redo", LUBY_TOK_REDO);
        LUBY_KW("super", LUBY_TOK_SUPER);
        LUBY_KW("self", LUBY_TOK_SELF);
        LUBY_KW("true", LUBY_TOK_TRUE);
        LUBY_KW("false", LUBY_TOK_FALSE);
        LUBY_KW("nil", LUBY_TOK_NIL);
        LUBY_KW("and", LUBY_TOK_AND);
        LUBY_KW("or", LUBY_TOK_OR);
        LUBY_KW("not", LUBY_TOK_NOT);
        LUBY_KW("begin", LUBY_TOK_BEGIN);
        LUBY_KW("rescue", LUBY_TOK_RESCUE);
        LUBY_KW("ensure", LUBY_TOK_ENSURE);
        LUBY_KW("raise", LUBY_TOK_RAISE);
        LUBY_KW("require", LUBY_TOK_REQUIRE);
        LUBY_KW("load", LUBY_TOK_LOAD);
        LUBY_KW("include", LUBY_TOK_INCLUDE);
        LUBY_KW("prepend", LUBY_TOK_PREPEND);
        LUBY_KW("extend", LUBY_TOK_EXTEND);
        LUBY_KW("attr_reader", LUBY_TOK_ATTR_READER);
        LUBY_KW("attr_writer", LUBY_TOK_ATTR_WRITER);
        LUBY_KW("attr_accessor", LUBY_TOK_ATTR_ACCESSOR);
        LUBY_KW("__FILE__", LUBY_TOK_FILE);
        LUBY_KW("__LINE__", LUBY_TOK_LINE);
        LUBY_KW("private", LUBY_TOK_PRIVATE);
        LUBY_KW("public", LUBY_TOK_PUBLIC);
        LUBY_KW("protected", LUBY_TOK_PROTECTED);
        LUBY_KW("alias", LUBY_TOK_ALIAS);
        #undef LUBY_KW

        if (start[0] >= 'A' && start[0] <= 'Z') {
            return luby_make_token(LUBY_TOK_CONSTANT, start, len, line, column);
        }
        return luby_make_token(LUBY_TOK_IDENTIFIER, start, len, line, column);
    }

    // Instance/class/global variables
    if (c == '@') {
        if (luby_lexer_peek(lex) == '@') {
            luby_lexer_advance(lex);
            while (luby_is_ident(luby_lexer_peek(lex))) luby_lexer_advance(lex);
            return luby_make_token(LUBY_TOK_CVAR, start, (size_t)(lex->src + lex->pos - start), line, column);
        }
        while (luby_is_ident(luby_lexer_peek(lex))) luby_lexer_advance(lex);
        return luby_make_token(LUBY_TOK_IVAR, start, (size_t)(lex->src + lex->pos - start), line, column);
    }
    if (c == '$') {
        while (luby_is_ident(luby_lexer_peek(lex))) luby_lexer_advance(lex);
        return luby_make_token(LUBY_TOK_GVAR, start, (size_t)(lex->src + lex->pos - start), line, column);
    }

    // Numbers
    if (c >= '0' && c <= '9') {
        int is_float = 0;
        while (luby_lexer_peek(lex) >= '0' && luby_lexer_peek(lex) <= '9') luby_lexer_advance(lex);
        if (luby_lexer_peek(lex) == '.' && luby_lexer_peek_next(lex) != '.') {
            is_float = 1;
            luby_lexer_advance(lex);
            while (luby_lexer_peek(lex) >= '0' && luby_lexer_peek(lex) <= '9') luby_lexer_advance(lex);
        }
        return luby_make_token(is_float ? LUBY_TOK_FLOAT : LUBY_TOK_INTEGER, start, (size_t)(lex->src + lex->pos - start), line, column);
    }

    // Strings
    if (c == '"' || c == '\'') {
        int quote = c;
        int is_double = (quote == '"');
        while ((c = luby_lexer_peek(lex)) != 0) {
            if (c == '\\') {
                luby_lexer_advance(lex);
                if (luby_lexer_peek(lex)) luby_lexer_advance(lex);
                continue;
            }
            // Check for interpolation in double-quoted strings
            if (is_double && c == '#' && luby_lexer_peek_next(lex) == '{') {
                // Emit STRING_PART for content before #{
                luby_lexer_advance(lex); // consume #
                luby_lexer_advance(lex); // consume {
                lex->in_interp_string = 1;
                lex->interp_brace_depth = 1;
                // Token includes opening quote for first part
                return luby_make_token(LUBY_TOK_STRING_PART, start, (size_t)(lex->src + lex->pos - start - 2), line, column);
            }
            if (c == quote) {
                luby_lexer_advance(lex);
                break;
            }
            luby_lexer_advance(lex);
        }
        return luby_make_token(LUBY_TOK_STRING, start, (size_t)(lex->src + lex->pos - start), line, column);
    }

    // Symbols
    if (c == ':') {
        if (luby_is_ident_start(luby_lexer_peek(lex))) {
            while (luby_is_ident(luby_lexer_peek(lex))) luby_lexer_advance(lex);
            return luby_make_token(LUBY_TOK_SYMBOL, start, (size_t)(lex->src + lex->pos - start), line, column);
        }
        if (luby_lexer_peek(lex) == '"' || luby_lexer_peek(lex) == '\'') {
            luby_lexer_advance(lex);
            int quote = lex->src[lex->pos - 1];
            while ((c = luby_lexer_peek(lex)) != 0) {
                if (c == '\\') { luby_lexer_advance(lex); if (luby_lexer_peek(lex)) luby_lexer_advance(lex); continue; }
                if (c == quote) { luby_lexer_advance(lex); break; }
                luby_lexer_advance(lex);
            }
            return luby_make_token(LUBY_TOK_SYMBOL, start, (size_t)(lex->src + lex->pos - start), line, column);
        }
        if (luby_lexer_peek(lex) == ':') {
            luby_lexer_advance(lex);
            return luby_make_token(LUBY_TOK_COLONCOLON, start, 2, line, column);
        }
        return luby_make_token(LUBY_TOK_COLON, start, 1, line, column);
    }

    // Two/three-character operators
    if (c == '.' && luby_lexer_peek(lex) == '.') {
        luby_lexer_advance(lex);
        if (luby_lexer_peek(lex) == '.') {
            luby_lexer_advance(lex);
            return luby_make_token(LUBY_TOK_RANGE_EXCL, start, 3, line, column);
        }
        return luby_make_token(LUBY_TOK_RANGE_INCL, start, 2, line, column);
    }
    if (c == '=' && luby_lexer_peek(lex) == '>') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_HASHROCKET, start, 2, line, column); }
    if (c == '=' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_EQEQ, start, 2, line, column); }
    if (c == '!' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_NEQ, start, 2, line, column); }
    if (c == '<' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_LTE, start, 2, line, column); }
    if (c == '>' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_GTE, start, 2, line, column); }
    // Check 3-char ||= and &&= before 2-char || and &&
    if (c == '|' && luby_lexer_peek(lex) == '|') {
        luby_lexer_advance(lex);
        if (luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_ORASSIGN, start, 3, line, column); }
        return luby_make_token(LUBY_TOK_OROR, start, 2, line, column);
    }
    if (c == '&' && luby_lexer_peek(lex) == '&') {
        luby_lexer_advance(lex);
        if (luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_ANDASSIGN, start, 3, line, column); }
        return luby_make_token(LUBY_TOK_ANDAND, start, 2, line, column);
    }
    if (c == '<' && luby_lexer_peek(lex) == '<') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_SHL, start, 2, line, column); }
    if (c == '>' && luby_lexer_peek(lex) == '>') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_SHR, start, 2, line, column); }
    if (c == '&' && luby_lexer_peek(lex) == '.') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_SAFE_NAV, start, 2, line, column); }
    if (c == '+' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_PLUSEQ, start, 2, line, column); }
    if (c == '-' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_MINUSEQ, start, 2, line, column); }
    if (c == '*' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_STAREQ, start, 2, line, column); }
    if (c == '/' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_SLASHEQ, start, 2, line, column); }
    if (c == '%' && luby_lexer_peek(lex) == '=') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_PERCENTEQ, start, 2, line, column); }
    if (c == '-' && luby_lexer_peek(lex) == '>') { luby_lexer_advance(lex); return luby_make_token(LUBY_TOK_ARROW, start, 2, line, column); }

    // Single-character tokens
    switch (c) {
        case '(': return luby_make_token(LUBY_TOK_LPAREN, start, 1, line, column);
        case ')': return luby_make_token(LUBY_TOK_RPAREN, start, 1, line, column);
        case '{':
            if (lex->in_interp_string) lex->interp_brace_depth++;
            return luby_make_token(LUBY_TOK_LBRACE, start, 1, line, column);
        case '}':
            if (lex->in_interp_string) {
                lex->interp_brace_depth--;
                if (lex->interp_brace_depth == 0) {
                    // End of interpolation expression, continue string scanning on next call
                    return luby_make_token(LUBY_TOK_INTERP_END, start, 1, line, column);
                }
            }
            return luby_make_token(LUBY_TOK_RBRACE, start, 1, line, column);
        case '[': return luby_make_token(LUBY_TOK_LBRACKET, start, 1, line, column);
        case ']': return luby_make_token(LUBY_TOK_RBRACKET, start, 1, line, column);
        case ',': return luby_make_token(LUBY_TOK_COMMA, start, 1, line, column);
        case '.': return luby_make_token(LUBY_TOK_DOT, start, 1, line, column);
        case ';': return luby_make_token(LUBY_TOK_SEMI, start, 1, line, column);
        case '|': return luby_make_token(LUBY_TOK_PIPE, start, 1, line, column);
        case '&': return luby_make_token(LUBY_TOK_AMP, start, 1, line, column);
        case '+': return luby_make_token(LUBY_TOK_PLUS, start, 1, line, column);
        case '-': return luby_make_token(LUBY_TOK_MINUS, start, 1, line, column);
        case '*': return luby_make_token(LUBY_TOK_STAR, start, 1, line, column);
        case '/': return luby_make_token(LUBY_TOK_SLASH, start, 1, line, column);
        case '%': return luby_make_token(LUBY_TOK_PERCENT, start, 1, line, column);
        case '^': return luby_make_token(LUBY_TOK_CARET, start, 1, line, column);
        case '!': return luby_make_token(LUBY_TOK_BANG, start, 1, line, column);
        case '~': return luby_make_token(LUBY_TOK_TILDE, start, 1, line, column);
        case '=': return luby_make_token(LUBY_TOK_EQ, start, 1, line, column);
        case '<': return luby_make_token(LUBY_TOK_LT, start, 1, line, column);
        case '>': return luby_make_token(LUBY_TOK_GT, start, 1, line, column);
        case '?': return luby_make_token(LUBY_TOK_QUESTION, start, 1, line, column);
    }

    return luby_make_token(LUBY_TOK_ERROR, start, 1, line, column);
}

// ------------------------------ Parser Impl --------------------------------

static luby_ast_node *luby_new_node(luby_state *L, luby_ast_kind kind, int line, int column) {
    luby_ast_node *node = (luby_ast_node *)luby_alloc_raw(L, NULL, sizeof(luby_ast_node));
    if (!node) return NULL;
    memset(node, 0, sizeof(*node));
    node->kind = kind;
    node->line = line;
    node->column = column;
    return node;
}

// Deep-copy an AST node and all its children
static luby_ast_node *luby_dup_ast(luby_state *L, luby_ast_node *node) {
    if (!node) return NULL;
    luby_ast_node *copy = luby_new_node(L, node->kind, node->line, node->column);
    if (!copy) return NULL;
    
    switch (node->kind) {
        case LUBY_AST_NIL:
        case LUBY_AST_BOOL:
        case LUBY_AST_INT:
        case LUBY_AST_FLOAT:
        case LUBY_AST_STRING:
        case LUBY_AST_LITERAL:
        case LUBY_AST_SYMBOL:
        case LUBY_AST_IDENT:
        case LUBY_AST_CONST:
        case LUBY_AST_IVAR:
        case LUBY_AST_REDO:
        case LUBY_AST_SPLAT_PARAM:
        case LUBY_AST_BLOCK_PARAM:
            // Leaf nodes, just copy literal
            copy->as.literal = node->as.literal;
            break;
        case LUBY_AST_BREAK:
        case LUBY_AST_NEXT:
            // Break and next can have optional value expressions
            copy->as.ret.value = luby_dup_ast(L, node->as.ret.value);
            break;
        case LUBY_AST_INTERP_STRING:
        case LUBY_AST_ARRAY:
        case LUBY_AST_HASH:
        case LUBY_AST_BLOCK:
            // List nodes
            if (node->as.list.count > 0) {
                copy->as.list.items = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.list.count * sizeof(luby_ast_node *));
                if (!copy->as.list.items) { luby_alloc_raw(L, copy, 0); return NULL; }
                copy->as.list.count = node->as.list.count;
                for (size_t i = 0; i < node->as.list.count; i++) {
                    copy->as.list.items[i] = luby_dup_ast(L, node->as.list.items[i]);
                }
            }
            break;
        case LUBY_AST_PAIR:
        case LUBY_AST_RANGE:
            copy->as.pair.left = luby_dup_ast(L, node->as.pair.left);
            copy->as.pair.right = luby_dup_ast(L, node->as.pair.right);
            break;
        case LUBY_AST_ASSIGN:
        case LUBY_AST_IVAR_ASSIGN:
        case LUBY_AST_DEFAULT_PARAM:
            copy->as.assign.target = luby_dup_ast(L, node->as.assign.target);
            copy->as.assign.value = luby_dup_ast(L, node->as.assign.value);
            break;
        case LUBY_AST_MULTI_ASSIGN:
            if (node->as.multi_assign.target_count > 0) {
                copy->as.multi_assign.targets = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.multi_assign.target_count * sizeof(luby_ast_node *));
                copy->as.multi_assign.target_count = node->as.multi_assign.target_count;
                for (size_t i = 0; i < node->as.multi_assign.target_count; i++) {
                    copy->as.multi_assign.targets[i] = luby_dup_ast(L, node->as.multi_assign.targets[i]);
                }
            }
            if (node->as.multi_assign.value_count > 0) {
                copy->as.multi_assign.values = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.multi_assign.value_count * sizeof(luby_ast_node *));
                copy->as.multi_assign.value_count = node->as.multi_assign.value_count;
                for (size_t i = 0; i < node->as.multi_assign.value_count; i++) {
                    copy->as.multi_assign.values[i] = luby_dup_ast(L, node->as.multi_assign.values[i]);
                }
            }
            break;
        case LUBY_AST_INDEX:
            copy->as.index.target = luby_dup_ast(L, node->as.index.target);
            copy->as.index.index = luby_dup_ast(L, node->as.index.index);
            copy->as.index.safe = node->as.index.safe;
            break;
        case LUBY_AST_INDEX_ASSIGN:
            copy->as.index_assign.target = luby_dup_ast(L, node->as.index_assign.target);
            copy->as.index_assign.index = luby_dup_ast(L, node->as.index_assign.index);
            copy->as.index_assign.value = luby_dup_ast(L, node->as.index_assign.value);
            break;
        case LUBY_AST_CALL:
            copy->as.call.recv = luby_dup_ast(L, node->as.call.recv);
            copy->as.call.method = node->as.call.method;
            copy->as.call.safe = node->as.call.safe;
            if (node->as.call.argc > 0) {
                copy->as.call.args = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.call.argc * sizeof(luby_ast_node *));
                copy->as.call.argc = node->as.call.argc;
                for (size_t i = 0; i < node->as.call.argc; i++) {
                    copy->as.call.args[i] = luby_dup_ast(L, node->as.call.args[i]);
                }
            }
            copy->as.call.block = luby_dup_ast(L, node->as.call.block);
            break;
        case LUBY_AST_LAMBDA:
            if (node->as.lambda.param_count > 0) {
                copy->as.lambda.params = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.lambda.param_count * sizeof(luby_ast_node *));
                copy->as.lambda.param_count = node->as.lambda.param_count;
                for (size_t i = 0; i < node->as.lambda.param_count; i++) {
                    copy->as.lambda.params[i] = luby_dup_ast(L, node->as.lambda.params[i]);
                }
            }
            copy->as.lambda.body = luby_dup_ast(L, node->as.lambda.body);
            break;
        case LUBY_AST_CLASS:
            copy->as.class_decl.name = node->as.class_decl.name;
            copy->as.class_decl.super_name = node->as.class_decl.super_name;
            copy->as.class_decl.body = luby_dup_ast(L, node->as.class_decl.body);
            break;
        case LUBY_AST_MODULE:
            copy->as.module_decl.name = node->as.module_decl.name;
            copy->as.module_decl.body = luby_dup_ast(L, node->as.module_decl.body);
            break;
        case LUBY_AST_DEF:
            copy->as.defn.name = node->as.defn.name;
            copy->as.defn.receiver = luby_dup_ast(L, node->as.defn.receiver);
            if (node->as.defn.param_count > 0) {
                copy->as.defn.params = (luby_ast_node **)luby_alloc_raw(L, NULL, node->as.defn.param_count * sizeof(luby_ast_node *));
                copy->as.defn.param_count = node->as.defn.param_count;
                for (size_t i = 0; i < node->as.defn.param_count; i++) {
                    copy->as.defn.params[i] = luby_dup_ast(L, node->as.defn.params[i]);
                }
            }
            copy->as.defn.body = luby_dup_ast(L, node->as.defn.body);
            break;
        case LUBY_AST_IF:
        case LUBY_AST_TERNARY:
            copy->as.if_stmt.cond = luby_dup_ast(L, node->as.if_stmt.cond);
            copy->as.if_stmt.then_branch = luby_dup_ast(L, node->as.if_stmt.then_branch);
            copy->as.if_stmt.else_branch = luby_dup_ast(L, node->as.if_stmt.else_branch);
            break;
        case LUBY_AST_WHILE:
            copy->as.while_stmt.cond = luby_dup_ast(L, node->as.while_stmt.cond);
            copy->as.while_stmt.body = luby_dup_ast(L, node->as.while_stmt.body);
            break;
        case LUBY_AST_RETURN:
            copy->as.ret.value = luby_dup_ast(L, node->as.ret.value);
            break;
        case LUBY_AST_BEGIN:
            copy->as.begin.body = luby_dup_ast(L, node->as.begin.body);
            copy->as.begin.rescue_body = luby_dup_ast(L, node->as.begin.rescue_body);
            copy->as.begin.ensure_body = luby_dup_ast(L, node->as.begin.ensure_body);
            break;
        case LUBY_AST_BINARY:
            copy->as.binary.op = node->as.binary.op;
            copy->as.binary.left = luby_dup_ast(L, node->as.binary.left);
            copy->as.binary.right = luby_dup_ast(L, node->as.binary.right);
            break;
        case LUBY_AST_UNARY:
            copy->as.unary.op = node->as.unary.op;
            copy->as.unary.expr = luby_dup_ast(L, node->as.unary.expr);
            break;
    }
    return copy;
}

static void luby_free_ast(luby_state *L, luby_ast_node *node) {
    if (!node) return;
    switch (node->kind) {
        case LUBY_AST_NIL:
        case LUBY_AST_BOOL:
        case LUBY_AST_INT:
        case LUBY_AST_FLOAT:
        case LUBY_AST_STRING:
        case LUBY_AST_LITERAL:
        case LUBY_AST_SYMBOL:
        case LUBY_AST_IDENT:
        case LUBY_AST_CONST:
        case LUBY_AST_IVAR:
        case LUBY_AST_REDO:
            // Leaf nodes, nothing to free
            break;
        case LUBY_AST_BREAK:
        case LUBY_AST_NEXT:
            // Break and next can have optional value expressions
            luby_free_ast(L, node->as.ret.value);
            break;
        case LUBY_AST_INTERP_STRING:
        case LUBY_AST_ARRAY:
        case LUBY_AST_HASH:
        case LUBY_AST_BLOCK:
            // List nodes
            for (size_t i = 0; i < node->as.list.count; i++) {
                luby_free_ast(L, node->as.list.items[i]);
            }
            luby_alloc_raw(L, node->as.list.items, 0);
            break;
        case LUBY_AST_PAIR:
        case LUBY_AST_RANGE:
            luby_free_ast(L, node->as.pair.left);
            luby_free_ast(L, node->as.pair.right);
            break;
        case LUBY_AST_ASSIGN:
        case LUBY_AST_IVAR_ASSIGN:
            luby_free_ast(L, node->as.assign.target);
            luby_free_ast(L, node->as.assign.value);
            break;
        case LUBY_AST_DEFAULT_PARAM:
            luby_free_ast(L, node->as.assign.target);
            luby_free_ast(L, node->as.assign.value);
            break;
        case LUBY_AST_SPLAT_PARAM:
        case LUBY_AST_BLOCK_PARAM:
            // These store just a literal (name), no children to free
            break;
        case LUBY_AST_MULTI_ASSIGN:
            for (size_t i = 0; i < node->as.multi_assign.target_count; i++) {
                luby_free_ast(L, node->as.multi_assign.targets[i]);
            }
            luby_alloc_raw(L, node->as.multi_assign.targets, 0);
            for (size_t i = 0; i < node->as.multi_assign.value_count; i++) {
                luby_free_ast(L, node->as.multi_assign.values[i]);
            }
            luby_alloc_raw(L, node->as.multi_assign.values, 0);
            break;
        case LUBY_AST_INDEX:
            luby_free_ast(L, node->as.index.target);
            luby_free_ast(L, node->as.index.index);
            break;
        case LUBY_AST_INDEX_ASSIGN:
            luby_free_ast(L, node->as.index_assign.target);
            luby_free_ast(L, node->as.index_assign.index);
            luby_free_ast(L, node->as.index_assign.value);
            break;
        case LUBY_AST_CALL:
            luby_free_ast(L, node->as.call.recv);
            for (size_t i = 0; i < node->as.call.argc; i++) {
                luby_free_ast(L, node->as.call.args[i]);
            }
            luby_alloc_raw(L, node->as.call.args, 0);
            luby_free_ast(L, node->as.call.block);
            break;
        case LUBY_AST_LAMBDA:
            for (size_t i = 0; i < node->as.lambda.param_count; i++) {
                luby_free_ast(L, node->as.lambda.params[i]);
            }
            luby_alloc_raw(L, node->as.lambda.params, 0);
            luby_free_ast(L, node->as.lambda.body);
            break;
        case LUBY_AST_CLASS:
            luby_free_ast(L, node->as.class_decl.body);
            break;
        case LUBY_AST_MODULE:
            luby_free_ast(L, node->as.module_decl.body);
            break;
        case LUBY_AST_DEF:
            luby_free_ast(L, node->as.defn.receiver);
            for (size_t i = 0; i < node->as.defn.param_count; i++) {
                luby_free_ast(L, node->as.defn.params[i]);
            }
            luby_alloc_raw(L, node->as.defn.params, 0);
            luby_free_ast(L, node->as.defn.body);
            break;
        case LUBY_AST_IF:
        case LUBY_AST_TERNARY:
            luby_free_ast(L, node->as.if_stmt.cond);
            luby_free_ast(L, node->as.if_stmt.then_branch);
            luby_free_ast(L, node->as.if_stmt.else_branch);
            break;
        case LUBY_AST_WHILE:
            luby_free_ast(L, node->as.while_stmt.cond);
            luby_free_ast(L, node->as.while_stmt.body);
            break;
        case LUBY_AST_RETURN:
            luby_free_ast(L, node->as.ret.value);
            break;
        case LUBY_AST_BEGIN:
            luby_free_ast(L, node->as.begin.body);
            luby_free_ast(L, node->as.begin.rescue_body);
            luby_free_ast(L, node->as.begin.ensure_body);
            break;
        case LUBY_AST_BINARY:
            luby_free_ast(L, node->as.binary.left);
            luby_free_ast(L, node->as.binary.right);
            break;
        case LUBY_AST_UNARY:
            luby_free_ast(L, node->as.unary.expr);
            break;
    }
    luby_alloc_raw(L, node, 0);
}

static void luby_parser_init(luby_parser *p, const char *code, size_t len, const char *filename, luby_error *error_out) {
    luby_lexer_init(&p->lex, code, len, filename);
    p->current = luby_lexer_next(&p->lex);
    p->next = luby_lexer_next(&p->lex);
    p->error = error_out;
}

static void luby_parser_error(luby_parser *p, const char *message) {
    if (!p->error || p->error->code != LUBY_E_OK) return;
    p->error->code = LUBY_E_PARSE;
    p->error->message = message;
    p->error->file = p->lex.filename;
    p->error->line = p->current.line;
    p->error->column = p->current.column;
}

static void luby_parser_advance(luby_parser *p) {
    p->current = p->next;
    p->next = luby_lexer_next(&p->lex);
}

static int luby_parser_match(luby_parser *p, luby_token_kind kind) {
    if (p->current.kind == kind) {
        luby_parser_advance(p);
        return 1;
    }
    return 0;
}

static int luby_token_precedence(luby_token_kind kind) {
    switch (kind) {
        case LUBY_TOK_QUESTION: return 1;  // ternary lowest precedence
        case LUBY_TOK_OR: return 2;
        case LUBY_TOK_AND: return 3;
        case LUBY_TOK_OROR: return 2;
        case LUBY_TOK_ANDAND: return 3;
        case LUBY_TOK_EQEQ:
        case LUBY_TOK_NEQ: return 4;
        case LUBY_TOK_LT:
        case LUBY_TOK_LTE:
        case LUBY_TOK_GT:
        case LUBY_TOK_GTE: return 5;
        case LUBY_TOK_RANGE_INCL:
        case LUBY_TOK_RANGE_EXCL: return 6;  // range operators
        case LUBY_TOK_PIPE:
        case LUBY_TOK_CARET:
        case LUBY_TOK_AMP: return 7;
        case LUBY_TOK_SHL:
        case LUBY_TOK_SHR: return 8;
        case LUBY_TOK_PLUS:
        case LUBY_TOK_MINUS: return 9;
        case LUBY_TOK_STAR:
        case LUBY_TOK_SLASH:
        case LUBY_TOK_PERCENT: return 10;
        default: return 0;
    }
}

static luby_ast_node *luby_parse_expr(luby_state *L, luby_parser *p, int prec);
static luby_ast_node *luby_parse_statement(luby_state *L, luby_parser *p);
static luby_ast_node *luby_parse_block_until(luby_state *L, luby_parser *p, luby_token_kind end_kind);
static luby_ast_node *luby_parse_block_until_any(luby_state *L, luby_parser *p, luby_token_kind a, luby_token_kind b, luby_token_kind c);
static luby_ast_node *luby_parse_case(luby_state *L, luby_parser *p);
static luby_ast_node *luby_parse_break(luby_state *L, luby_parser *p, luby_ast_kind kind);
static luby_ast_node *luby_parse_redo(luby_state *L, luby_parser *p);
static luby_ast_node *luby_parse_for(luby_state *L, luby_parser *p);
static luby_ast_node *luby_parse_compound_assign(luby_state *L, luby_parser *p, luby_ast_node *lhs);
static luby_ast_node *luby_parse_conditional_assign(luby_state *L, luby_parser *p, luby_ast_node *lhs);
static luby_ast_node *luby_parse_keyword_call(luby_state *L, luby_parser *p, const char *name, size_t name_len);
static luby_ast_node *luby_parse_begin(luby_state *L, luby_parser *p);
static luby_ast_node *luby_make_unary(luby_state *L, luby_token_kind op, luby_ast_node *expr, int line, int column);
static luby_ast_node *luby_parse_block_expr(luby_state *L, luby_parser *p, luby_token_kind end_kind);
static luby_ast_node *luby_make_call(luby_state *L, luby_ast_node *recv, luby_string_view method, int line, int column);
static void luby_parse_call_args(luby_state *L, luby_parser *p, luby_ast_node *call);
static luby_ast_node *luby_parse_assignment_from(luby_state *L, luby_parser *p, luby_ast_node *lhs);

static int luby_token_is_name(luby_token_kind kind) {
    switch (kind) {
        case LUBY_TOK_IDENTIFIER:
        case LUBY_TOK_CLASS:
        case LUBY_TOK_MODULE:
        case LUBY_TOK_DEF:
        case LUBY_TOK_END:
        case LUBY_TOK_IF:
        case LUBY_TOK_ELSIF:
        case LUBY_TOK_ELSE:
        case LUBY_TOK_UNLESS:
        case LUBY_TOK_WHILE:
        case LUBY_TOK_UNTIL:
        case LUBY_TOK_FOR:
        case LUBY_TOK_IN:
        case LUBY_TOK_CASE:
        case LUBY_TOK_WHEN:
        case LUBY_TOK_THEN:
        case LUBY_TOK_DO:
        case LUBY_TOK_YIELD:
        case LUBY_TOK_RETURN:
        case LUBY_TOK_BREAK:
        case LUBY_TOK_NEXT:
        case LUBY_TOK_REDO:
        case LUBY_TOK_SUPER:
        case LUBY_TOK_SELF:
        case LUBY_TOK_TRUE:
        case LUBY_TOK_FALSE:
        case LUBY_TOK_NIL:
        case LUBY_TOK_AND:
        case LUBY_TOK_OR:
        case LUBY_TOK_NOT:
        case LUBY_TOK_BEGIN:
        case LUBY_TOK_RESCUE:
        case LUBY_TOK_ENSURE:
        case LUBY_TOK_RAISE:
        case LUBY_TOK_REQUIRE:
        case LUBY_TOK_LOAD:
        case LUBY_TOK_INCLUDE:
        case LUBY_TOK_PREPEND:
        case LUBY_TOK_EXTEND:
            return 1;
        default:
            return 0;
    }
}

static luby_ast_node *luby_parse_primary(luby_state *L, luby_parser *p) {
    luby_token tok = p->current;
    switch (tok.kind) {
        case LUBY_TOK_INTEGER: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_INT, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_FLOAT: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_FLOAT, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_STRING: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_STRING, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_STRING_PART: {
            // Interpolated string: collect parts (strings and expressions)
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_INTERP_STRING, tok.line, tok.column);
            if (!node) return NULL;

            // Temporary storage for parts
            luby_ast_node **parts = NULL;
            size_t part_count = 0;
            size_t part_cap = 0;

            // First part: the string segment before first #{}
            luby_ast_node *str_part = luby_new_node(L, LUBY_AST_STRING, tok.line, tok.column);
            if (str_part) str_part->as.literal = tok.lexeme;
            if (part_count >= part_cap) {
                part_cap = part_cap ? part_cap * 2 : 4;
                parts = (luby_ast_node **)luby_alloc_raw(L, parts, part_cap * sizeof(luby_ast_node *));
            }
            parts[part_count++] = str_part;

            // Now parse: expression, INTERP_END, then either STRING_PART or STRING_END
            for (;;) {
                // Parse expression inside #{}
                luby_ast_node *expr = luby_parse_expr(L, p, 0);
                if (part_count >= part_cap) {
                    part_cap = part_cap ? part_cap * 2 : 4;
                    parts = (luby_ast_node **)luby_alloc_raw(L, parts, part_cap * sizeof(luby_ast_node *));
                }
                parts[part_count++] = expr;

                // Expect INTERP_END (the closing })
                if (p->current.kind != LUBY_TOK_INTERP_END) {
                    luby_parser_error(p, "expected '}' to close interpolation");
                    return NULL;
                }
                luby_parser_advance(p);

                // Next should be STRING_PART (more interpolations) or STRING_END
                if (p->current.kind == LUBY_TOK_STRING_PART) {
                    tok = p->current;
                    luby_parser_advance(p);
                    str_part = luby_new_node(L, LUBY_AST_STRING, tok.line, tok.column);
                    if (str_part) str_part->as.literal = tok.lexeme;
                    if (part_count >= part_cap) {
                        part_cap = part_cap ? part_cap * 2 : 4;
                        parts = (luby_ast_node **)luby_alloc_raw(L, parts, part_cap * sizeof(luby_ast_node *));
                    }
                    parts[part_count++] = str_part;
                    // Continue loop to parse next interpolation
                } else if (p->current.kind == LUBY_TOK_STRING_END) {
                    tok = p->current;
                    luby_parser_advance(p);
                    str_part = luby_new_node(L, LUBY_AST_STRING, tok.line, tok.column);
                    if (str_part) str_part->as.literal = tok.lexeme;
                    if (part_count >= part_cap) {
                        part_cap = part_cap ? part_cap * 2 : 4;
                        parts = (luby_ast_node **)luby_alloc_raw(L, parts, part_cap * sizeof(luby_ast_node *));
                    }
                    parts[part_count++] = str_part;
                    break; // Done
                } else {
                    luby_parser_error(p, "expected string continuation after interpolation");
                    return NULL;
                }
            }

            node->as.list.items = parts;
            node->as.list.count = part_count;
            return node;
        }
        case LUBY_TOK_SYMBOL: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_SYMBOL, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_TRUE:
        case LUBY_TOK_FALSE: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_BOOL, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_NIL: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_NIL, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_FILE: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_STRING, tok.line, tok.column);
            if (node) {
                // __FILE__ returns the current filename
                size_t len = p->lex.filename ? strlen(p->lex.filename) : 0;
                node->as.literal.data = p->lex.filename;
                node->as.literal.length = len;
            }
            return node;
        }
        case LUBY_TOK_LINE: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_INT, tok.line, tok.column);
            if (node) {
                // __LINE__ returns the current line number as an integer literal
                // We need to allocate a buffer for the line number string
                char *buf = (char *)luby_alloc_raw(L, NULL, 32);
                if (buf) {
                    snprintf(buf, 32, "%d", tok.line);
                    node->as.literal.data = buf;
                    node->as.literal.length = strlen(buf);
                }
            }
            return node;
        }
        case LUBY_TOK_IDENTIFIER: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_IDENT, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_CONSTANT: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_CONST, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_SELF: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_IDENT, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_IVAR: {
            luby_parser_advance(p);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_IVAR, tok.line, tok.column);
            if (node) node->as.literal = tok.lexeme;
            return node;
        }
        case LUBY_TOK_YIELD: {
            luby_parser_advance(p);
            luby_string_view name = { "yield", 5 };
            luby_ast_node *call = luby_make_call(L, NULL, name, tok.line, tok.column);
            if (!call) return NULL;
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_parse_call_args(L, p, call);
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
            }
            return call;
        }
        case LUBY_TOK_SUPER: {
            luby_parser_advance(p);
            luby_string_view name = { "super", 5 };
            luby_ast_node *call = luby_make_call(L, NULL, name, tok.line, tok.column);
            if (!call) return NULL;
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_parse_call_args(L, p, call);
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
            }
            return call;
        }
        case LUBY_TOK_REQUIRE: {
            luby_parser_advance(p);
            luby_string_view name = { "require", 7 };
            luby_ast_node *call = luby_make_call(L, NULL, name, tok.line, tok.column);
            if (!call) return NULL;
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_parse_call_args(L, p, call);
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
            }
            return call;
        }
        case LUBY_TOK_LOAD: {
            luby_parser_advance(p);
            luby_string_view name = { "load", 4 };
            luby_ast_node *call = luby_make_call(L, NULL, name, tok.line, tok.column);
            if (!call) return NULL;
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_parse_call_args(L, p, call);
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
            }
            return call;
        }
        case LUBY_TOK_RAISE: {
            return luby_parse_keyword_call(L, p, "raise", 5);
        }
        case LUBY_TOK_ARROW: {
            // Stabby lambda: ->(x, y) { ... } or -> { ... }
            luby_parser_advance(p); // consume ->
            luby_ast_node *node = luby_new_node(L, LUBY_AST_LAMBDA, tok.line, tok.column);
            if (!node) return NULL;
            
            // Optional parameters: (x, y)
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_ast_node *params = luby_new_node(L, LUBY_AST_BLOCK, p->current.line, p->current.column);
                if (!params) return NULL;
                while (p->current.kind != LUBY_TOK_RPAREN && p->current.kind != LUBY_TOK_EOF) {
                    if (p->current.kind != LUBY_TOK_IDENTIFIER) {
                        luby_parser_error(p, "expected parameter name");
                        break;
                    }
                    luby_token name = p->current;
                    luby_parser_advance(p);
                    luby_ast_node *param = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
                    if (param) param->as.literal = name.lexeme;
                    size_t n = params->as.list.count;
                    params->as.list.items = (luby_ast_node **)luby_alloc_raw(L, params->as.list.items, (n + 1) * sizeof(luby_ast_node *));
                    if (!params->as.list.items) return NULL;
                    params->as.list.items[n] = param;
                    params->as.list.count = n + 1;
                    if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
                }
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
                node->as.lambda.params = params->as.list.items;
                node->as.lambda.param_count = params->as.list.count;
                // Free the wrapper block node (not items array or children)
                luby_alloc_raw(L, params, 0);
            }
            
            // Body must be in braces: { ... }
            if (p->current.kind != LUBY_TOK_LBRACE) {
                luby_parser_error(p, "expected '{' for lambda body");
                return NULL;
            }
            luby_parser_advance(p); // consume {
            node->as.lambda.body = luby_parse_block_until(L, p, LUBY_TOK_RBRACE);
            if (!luby_parser_match(p, LUBY_TOK_RBRACE)) {
                luby_parser_error(p, "expected '}'");
            }
            return node;
        }
        case LUBY_TOK_LPAREN: {
            luby_parser_advance(p);
            luby_ast_node *expr = luby_parse_expr(L, p, 0);
            if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                luby_parser_error(p, "expected ')'");
            }
            return expr;
        }
        case LUBY_TOK_LBRACKET: {
            luby_parser_advance(p);
            luby_ast_node *arr = luby_new_node(L, LUBY_AST_ARRAY, tok.line, tok.column);
            if (!arr) return NULL;
            while (p->current.kind != LUBY_TOK_RBRACKET && p->current.kind != LUBY_TOK_EOF) {
                luby_ast_node *item = luby_parse_expr(L, p, 0);
                size_t n = arr->as.list.count;
                arr->as.list.items = (luby_ast_node **)luby_alloc_raw(L, arr->as.list.items, (n + 1) * sizeof(luby_ast_node *));
                if (!arr->as.list.items) return NULL;
                arr->as.list.items[n] = item;
                arr->as.list.count = n + 1;
                if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
            }
            if (!luby_parser_match(p, LUBY_TOK_RBRACKET)) {
                luby_parser_error(p, "expected ']'");
            }
            return arr;
        }
        case LUBY_TOK_LBRACE: {
            luby_parser_advance(p);
            luby_ast_node *hash = luby_new_node(L, LUBY_AST_HASH, tok.line, tok.column);
            if (!hash) return NULL;
            while (p->current.kind != LUBY_TOK_RBRACE && p->current.kind != LUBY_TOK_EOF) {
                luby_ast_node *key = NULL;
                luby_ast_node *value = NULL;
                // Check for symbol shorthand: `key: value` where key is an identifier
                if (p->current.kind == LUBY_TOK_IDENTIFIER && p->next.kind == LUBY_TOK_COLON) {
                    // Convert identifier to symbol
                    luby_token keytok = p->current;
                    key = luby_new_node(L, LUBY_AST_SYMBOL, keytok.line, keytok.column);
                    if (!key) return NULL;
                    key->as.literal = keytok.lexeme;  // Store identifier text as the symbol
                    luby_parser_advance(p); // consume identifier
                    luby_parser_advance(p); // consume colon
                    value = luby_parse_expr(L, p, 0);
                } else {
                    key = luby_parse_expr(L, p, 0);
                    if (!luby_parser_match(p, LUBY_TOK_HASHROCKET)) {
                        luby_parser_error(p, "expected '=>'");
                        break;
                    }
                    value = luby_parse_expr(L, p, 0);
                }
                luby_ast_node *pair = luby_new_node(L, LUBY_AST_PAIR, tok.line, tok.column);
                if (!pair) return NULL;
                pair->as.pair.left = key;
                pair->as.pair.right = value;
                size_t n = hash->as.list.count;
                hash->as.list.items = (luby_ast_node **)luby_alloc_raw(L, hash->as.list.items, (n + 1) * sizeof(luby_ast_node *));
                if (!hash->as.list.items) return NULL;
                hash->as.list.items[n] = pair;
                hash->as.list.count = n + 1;
                if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
            }
            if (!luby_parser_match(p, LUBY_TOK_RBRACE)) {
                luby_parser_error(p, "expected '}'");
            }
            return hash;
        }
        default:
            luby_parser_error(p, "unexpected token");
            return NULL;
    }
}

static luby_ast_node *luby_parse_postfix(luby_state *L, luby_parser *p, luby_ast_node *left) {
    for (;;) {
        if (p->current.kind == LUBY_TOK_DOT || p->current.kind == LUBY_TOK_SAFE_NAV) {
            int safe = (p->current.kind == LUBY_TOK_SAFE_NAV);
            luby_parser_advance(p);
            if (safe && p->current.kind == LUBY_TOK_LBRACKET) {
                luby_parser_advance(p);
                luby_ast_node *index = luby_parse_expr(L, p, 0);
                if (!luby_parser_match(p, LUBY_TOK_RBRACKET)) {
                    luby_parser_error(p, "expected ']'");
                }
                luby_ast_node *node = luby_new_node(L, LUBY_AST_INDEX, left->line, left->column);
                if (!node) return NULL;
                node->as.index.target = left;
                node->as.index.index = index;
                node->as.index.safe = 1;
                left = node;
                continue;
            }
            if (!luby_token_is_name(p->current.kind)) {
                luby_parser_error(p, "expected method name after '.'");
                return left;
            }
            luby_token method = p->current;
            luby_parser_advance(p);
            luby_ast_node *call = luby_make_call(L, left, method.lexeme, method.line, method.column);
            if (call) call->as.call.safe = safe;
            if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
                luby_parse_call_args(L, p, call);
                if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                    luby_parser_error(p, "expected ')'");
                }
            }
            left = call;
            continue;
        }

        if (p->current.kind == LUBY_TOK_LBRACKET) {
            luby_parser_advance(p);
            luby_ast_node *index = luby_parse_expr(L, p, 0);
            if (!luby_parser_match(p, LUBY_TOK_RBRACKET)) {
                luby_parser_error(p, "expected ']'");
            }
            luby_ast_node *node = luby_new_node(L, LUBY_AST_INDEX, left->line, left->column);
            if (!node) return NULL;
            node->as.index.target = left;
            node->as.index.index = index;
            node->as.index.safe = 0;
            left = node;
            continue;
        }

        if (p->current.kind == LUBY_TOK_LPAREN) {
            luby_string_view ident_tok = left->as.literal;
            if (left->kind != LUBY_AST_IDENT && left->kind != LUBY_AST_CONST) {
                return left;
            }
            luby_parser_advance(p);
            luby_ast_node *call = luby_make_call(L, NULL, ident_tok, left->line, left->column);
            if (!call) return NULL;
            // Free the identifier node now that we've converted it to a call
            luby_alloc_raw(L, left, 0);
            luby_parse_call_args(L, p, call);
            if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
                luby_parser_error(p, "expected ')'");
            }
            left = call;
            continue;
        }

        if (p->current.kind == LUBY_TOK_LBRACE || p->current.kind == LUBY_TOK_DO) {
            luby_token_kind end_kind = p->current.kind == LUBY_TOK_LBRACE ? LUBY_TOK_RBRACE : LUBY_TOK_END;
            luby_parser_advance(p);
            luby_ast_node *block = luby_parse_block_expr(L, p, end_kind);
            if (!block) return NULL;

            if (left->kind != LUBY_AST_CALL) {
                if (left->kind == LUBY_AST_IDENT || left->kind == LUBY_AST_CONST) {
                    luby_ast_node *old_left = left;
                    luby_ast_node *call = luby_make_call(L, NULL, left->as.literal, left->line, left->column);
                    if (!call) return NULL;
                    luby_alloc_raw(L, old_left, 0);  // Free the identifier node
                    left = call;
                } else {
                    return left;
                }
            }
            left->as.call.block = block;
            continue;
        }

        break;
    }
    return left;
}

static luby_ast_node *luby_parse_unary(luby_state *L, luby_parser *p) {
    if (p->current.kind == LUBY_TOK_BANG || p->current.kind == LUBY_TOK_MINUS || p->current.kind == LUBY_TOK_PLUS || p->current.kind == LUBY_TOK_TILDE || p->current.kind == LUBY_TOK_NOT) {
        luby_token op = p->current;
        luby_parser_advance(p);
        luby_ast_node *expr = luby_parse_unary(L, p);
        luby_ast_node *node = luby_new_node(L, LUBY_AST_UNARY, op.line, op.column);
        if (node) {
            node->as.unary.op = op.kind;
            node->as.unary.expr = expr;
        }
        return node;
    }
    luby_ast_node *primary = luby_parse_primary(L, p);
    if (!primary) return NULL;
    return luby_parse_postfix(L, p, primary);
}

static luby_ast_node *luby_parse_expr(luby_state *L, luby_parser *p, int prec) {
    luby_ast_node *left = luby_parse_unary(L, p);
    if (!left) return NULL;
    for (;;) {
        int tok_prec = luby_token_precedence(p->current.kind);
        if (tok_prec <= prec) break;
        luby_token op = p->current;
        
        // Handle ternary operator: cond ? then : else
        if (op.kind == LUBY_TOK_QUESTION) {
            luby_parser_advance(p);
            luby_ast_node *then_expr = luby_parse_expr(L, p, 0);
            if (!luby_parser_match(p, LUBY_TOK_COLON)) {
                luby_parser_error(p, "expected ':' in ternary expression");
                return NULL;
            }
            luby_ast_node *else_expr = luby_parse_expr(L, p, 0);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_TERNARY, op.line, op.column);
            if (node) {
                node->as.if_stmt.cond = left;
                node->as.if_stmt.then_branch = then_expr;
                node->as.if_stmt.else_branch = else_expr;
            }
            left = node;
            continue;
        }
        
        // Handle range operators: a..b or a...b
        if (op.kind == LUBY_TOK_RANGE_INCL || op.kind == LUBY_TOK_RANGE_EXCL) {
            luby_parser_advance(p);
            luby_ast_node *right = luby_parse_expr(L, p, tok_prec);
            luby_ast_node *node = luby_new_node(L, LUBY_AST_RANGE, op.line, op.column);
            if (node) {
                node->as.range.start = left;
                node->as.range.end = right;
                node->as.range.exclusive = (op.kind == LUBY_TOK_RANGE_EXCL);
            }
            left = node;
            continue;
        }
        
        luby_parser_advance(p);
        luby_ast_node *right = luby_parse_expr(L, p, tok_prec);
        luby_ast_node *node = luby_new_node(L, LUBY_AST_BINARY, op.line, op.column);
        if (node) {
            node->as.binary.op = op.kind;
            node->as.binary.left = left;
            node->as.binary.right = right;
        }
        left = node;
    }
    return left;
}

static luby_ast_node *luby_make_unary(luby_state *L, luby_token_kind op, luby_ast_node *expr, int line, int column) {
    luby_ast_node *node = luby_new_node(L, LUBY_AST_UNARY, line, column);
    if (!node) return NULL;
    node->as.unary.op = op;
    node->as.unary.expr = expr;
    return node;
}

static luby_ast_node *luby_make_call(luby_state *L, luby_ast_node *recv, luby_string_view method, int line, int column) {
    luby_ast_node *call = luby_new_node(L, LUBY_AST_CALL, line, column);
    if (!call) return NULL;
    call->as.call.recv = recv;
    call->as.call.method = method;
    call->as.call.safe = 0;
    return call;
}

static luby_ast_node *luby_parse_keyword_call(luby_state *L, luby_parser *p, const char *name, size_t name_len) {
    luby_token tok = p->current;
    luby_parser_advance(p);
    luby_string_view method = { name, name_len };
    luby_ast_node *call = luby_make_call(L, NULL, method, tok.line, tok.column);
    if (!call) return NULL;

    if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
        luby_parse_call_args(L, p, call);
        if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
            luby_parser_error(p, "expected ')'");
        }
        return call;
    }

    if (p->current.kind == LUBY_TOK_END || p->current.kind == LUBY_TOK_EOF ||
        p->current.kind == LUBY_TOK_SEMI || p->current.kind == LUBY_TOK_NEWLINE) {
        return call;
    }

    while (p->current.kind != LUBY_TOK_END && p->current.kind != LUBY_TOK_EOF &&
           p->current.kind != LUBY_TOK_SEMI && p->current.kind != LUBY_TOK_NEWLINE) {
        luby_ast_node *arg = luby_parse_expr(L, p, 0);
        size_t n = call->as.call.argc;
        call->as.call.args = (luby_ast_node **)luby_alloc_raw(L, call->as.call.args, (n + 1) * sizeof(luby_ast_node *));
        if (!call->as.call.args) return NULL;
        call->as.call.args[n] = arg;
        call->as.call.argc = n + 1;
        if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
    }
    return call;
}

static void luby_parse_call_args(luby_state *L, luby_parser *p, luby_ast_node *call) {
    while (p->current.kind != LUBY_TOK_RPAREN && p->current.kind != LUBY_TOK_EOF) {
        luby_ast_node *arg = luby_parse_expr(L, p, 0);
        size_t n = call->as.call.argc;
        call->as.call.args = (luby_ast_node **)luby_alloc_raw(L, call->as.call.args, (n + 1) * sizeof(luby_ast_node *));
        if (!call->as.call.args) return;
        call->as.call.args[n] = arg;
        call->as.call.argc = n + 1;
        if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
    }
}

static luby_ast_node *luby_parse_block_expr(luby_state *L, luby_parser *p, luby_token_kind end_kind) {
    luby_ast_node *node = luby_new_node(L, LUBY_AST_LAMBDA, p->current.line, p->current.column);
    if (!node) return NULL;

    // Optional block params: |a, b|
    if (p->current.kind == LUBY_TOK_PIPE) {
        luby_parser_advance(p);
        luby_ast_node *params = luby_new_node(L, LUBY_AST_BLOCK, p->current.line, p->current.column);
        if (!params) return NULL;
        while (p->current.kind != LUBY_TOK_PIPE && p->current.kind != LUBY_TOK_EOF) {
            if (p->current.kind != LUBY_TOK_IDENTIFIER) {
                luby_parser_error(p, "expected block parameter");
                break;
            }
            luby_token name = p->current;
            luby_parser_advance(p);
            luby_ast_node *param = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
            if (param) param->as.literal = name.lexeme;
            size_t n = params->as.list.count;
            params->as.list.items = (luby_ast_node **)luby_alloc_raw(L, params->as.list.items, (n + 1) * sizeof(luby_ast_node *));
            if (!params->as.list.items) return NULL;
            params->as.list.items[n] = param;
            params->as.list.count = n + 1;
            if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
        }
        if (!luby_parser_match(p, LUBY_TOK_PIPE)) {
            luby_parser_error(p, "expected '|'");
        }
        node->as.lambda.params = params->as.list.items;
        node->as.lambda.param_count = params->as.list.count;
        // Free the wrapper block node (not items array or children)
        luby_alloc_raw(L, params, 0);
    }

    node->as.lambda.body = luby_parse_block_until(L, p, end_kind);
    if (!luby_parser_match(p, end_kind)) {
        luby_parser_error(p, end_kind == LUBY_TOK_RBRACE ? "expected '}'" : "expected 'end'");
    }
    return node;
}

static luby_ast_node *luby_parse_params(luby_state *L, luby_parser *p) {
    luby_ast_node *params = luby_new_node(L, LUBY_AST_BLOCK, p->current.line, p->current.column);
    if (!params) return NULL;
    while (p->current.kind != LUBY_TOK_RPAREN && p->current.kind != LUBY_TOK_EOF) {
        luby_ast_node *param = NULL;
        
        // Check for splat param: *args
        if (p->current.kind == LUBY_TOK_STAR) {
            luby_parser_advance(p);
            if (p->current.kind != LUBY_TOK_IDENTIFIER) {
                luby_parser_error(p, "expected parameter name after '*'");
                break;
            }
            luby_token name = p->current;
            luby_parser_advance(p);
            param = luby_new_node(L, LUBY_AST_SPLAT_PARAM, name.line, name.column);
            if (param) param->as.literal = name.lexeme;
        }
        // Check for block param: &block
        else if (p->current.kind == LUBY_TOK_AMP) {
            luby_parser_advance(p);
            if (p->current.kind != LUBY_TOK_IDENTIFIER) {
                luby_parser_error(p, "expected parameter name after '&'");
                break;
            }
            luby_token name = p->current;
            luby_parser_advance(p);
            param = luby_new_node(L, LUBY_AST_BLOCK_PARAM, name.line, name.column);
            if (param) param->as.literal = name.lexeme;
        }
        // Regular param or default param
        else if (p->current.kind == LUBY_TOK_IDENTIFIER) {
            luby_token name = p->current;
            luby_parser_advance(p);
            // Check for default value: x = expr
            if (luby_parser_match(p, LUBY_TOK_EQ)) {
                luby_ast_node *default_val = luby_parse_expr(L, p, 0);
                param = luby_new_node(L, LUBY_AST_DEFAULT_PARAM, name.line, name.column);
                if (param) {
                    param->as.assign.target = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
                    if (param->as.assign.target) param->as.assign.target->as.literal = name.lexeme;
                    param->as.assign.value = default_val;
                }
            } else {
                param = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
                if (param) param->as.literal = name.lexeme;
            }
        } else {
            luby_parser_error(p, "expected parameter name");
            break;
        }
        
        size_t n = params->as.list.count;
        params->as.list.items = (luby_ast_node **)luby_alloc_raw(L, params->as.list.items, (n + 1) * sizeof(luby_ast_node *));
        if (!params->as.list.items) return NULL;
        params->as.list.items[n] = param;
        params->as.list.count = n + 1;
        if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
    }
    return params;
}

static luby_ast_node *luby_parse_def(luby_state *L, luby_parser *p) {
    luby_token def_tok = p->current;
    luby_parser_advance(p);
    if (!luby_token_is_name(p->current.kind) && p->current.kind != LUBY_TOK_SELF) {
        luby_parser_error(p, "expected method name or receiver");
        return NULL;
    }
    luby_token first = p->current;
    luby_parser_advance(p);
    
    luby_ast_node *receiver = NULL;
    luby_token name;
    
    // Check for singleton method: def receiver.method_name
    if (luby_parser_match(p, LUBY_TOK_DOT)) {
        // first token was the receiver
        receiver = luby_new_node(L, LUBY_AST_IDENT, first.line, first.column);
        if (receiver) receiver->as.literal = first.lexeme;
        if (!luby_token_is_name(p->current.kind)) {
            luby_parser_error(p, "expected method name after '.'");
            return NULL;
        }
        name = p->current;
        luby_parser_advance(p);
    } else {
        // Regular method
        name = first;
    }
    
    luby_ast_node *node = luby_new_node(L, LUBY_AST_DEF, def_tok.line, def_tok.column);
    if (!node) return NULL;
    node->as.defn.name = name.lexeme;
    node->as.defn.receiver = receiver;
    if (luby_parser_match(p, LUBY_TOK_LPAREN)) {
        luby_ast_node *params = luby_parse_params(L, p);
        node->as.defn.params = params ? params->as.list.items : NULL;
        node->as.defn.param_count = params ? params->as.list.count : 0;
        // Free the wrapper block node (not the items array or children, they're now owned by node)
        if (params) luby_alloc_raw(L, params, 0);
        if (!luby_parser_match(p, LUBY_TOK_RPAREN)) {
            luby_parser_error(p, "expected ')'");
        }
    }
    node->as.defn.body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    return node;
}

static luby_ast_node *luby_parse_class(luby_state *L, luby_parser *p) {
    luby_token class_tok = p->current;
    luby_parser_advance(p);
    if (p->current.kind != LUBY_TOK_CONSTANT) {
        luby_parser_error(p, "expected class name");
        return NULL;
    }
    luby_token name = p->current;
    luby_parser_advance(p);
    luby_ast_node *node = luby_new_node(L, LUBY_AST_CLASS, class_tok.line, class_tok.column);
    if (!node) return NULL;
    node->as.class_decl.name = name.lexeme;
    if (luby_parser_match(p, LUBY_TOK_LT)) {
        if (p->current.kind != LUBY_TOK_CONSTANT) {
            luby_parser_error(p, "expected superclass name");
        } else {
            node->as.class_decl.super_name = p->current.lexeme;
            luby_parser_advance(p);
        }
    }
    node->as.class_decl.body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    return node;
}

static luby_ast_node *luby_parse_module(luby_state *L, luby_parser *p) {
    luby_token mod_tok = p->current;
    luby_parser_advance(p);
    if (p->current.kind != LUBY_TOK_CONSTANT) {
        luby_parser_error(p, "expected module name");
        return NULL;
    }
    luby_token name = p->current;
    luby_parser_advance(p);
    luby_ast_node *node = luby_new_node(L, LUBY_AST_MODULE, mod_tok.line, mod_tok.column);
    if (!node) return NULL;
    node->as.module_decl.name = name.lexeme;
    node->as.module_decl.body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    return node;
}

static luby_ast_node *luby_parse_if(luby_state *L, luby_parser *p) {
    luby_token if_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *cond = luby_parse_expr(L, p, 0);
    luby_ast_node *then_branch = luby_parse_block_until_any(L, p, LUBY_TOK_ELSE, LUBY_TOK_ELSIF, LUBY_TOK_END);
    luby_ast_node *else_branch = NULL;
    luby_ast_node *tail = NULL;
    while (p->current.kind == LUBY_TOK_ELSIF) {
        luby_parser_advance(p);
        luby_ast_node *elsif_cond = luby_parse_expr(L, p, 0);
        luby_ast_node *elsif_then = luby_parse_block_until_any(L, p, LUBY_TOK_ELSE, LUBY_TOK_ELSIF, LUBY_TOK_END);
        luby_ast_node *elsif_node = luby_new_node(L, LUBY_AST_IF, if_tok.line, if_tok.column);
        if (elsif_node) {
            elsif_node->as.if_stmt.cond = elsif_cond;
            elsif_node->as.if_stmt.then_branch = elsif_then;
        }
        if (!else_branch) {
            else_branch = elsif_node;
            tail = elsif_node;
        } else if (tail) {
            tail->as.if_stmt.else_branch = elsif_node;
            tail = elsif_node;
        }
    }
    if (p->current.kind == LUBY_TOK_ELSE) {
        luby_parser_advance(p);
        if (tail) {
            tail->as.if_stmt.else_branch = luby_parse_block_until(L, p, LUBY_TOK_END);
        } else {
            else_branch = luby_parse_block_until(L, p, LUBY_TOK_END);
        }
    }
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    luby_ast_node *node = luby_new_node(L, LUBY_AST_IF, if_tok.line, if_tok.column);
    if (!node) return NULL;
    node->as.if_stmt.cond = cond;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

static luby_ast_node *luby_parse_until(luby_state *L, luby_parser *p) {
    luby_token until_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *cond = luby_parse_expr(L, p, 0);
    luby_ast_node *body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    luby_ast_node *node = luby_new_node(L, LUBY_AST_WHILE, until_tok.line, until_tok.column);
    if (!node) return NULL;
    node->as.while_stmt.cond = luby_make_unary(L, LUBY_TOK_NOT, cond, until_tok.line, until_tok.column);
    node->as.while_stmt.body = body;
    return node;
}

static luby_ast_node *luby_parse_unless(luby_state *L, luby_parser *p) {
    luby_token unless_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *cond = luby_parse_expr(L, p, 0);
    luby_ast_node *then_branch = luby_parse_block_until_any(L, p, LUBY_TOK_ELSE, LUBY_TOK_ELSIF, LUBY_TOK_END);
    luby_ast_node *else_branch = NULL;
    if (p->current.kind == LUBY_TOK_ELSE) {
        luby_parser_advance(p);
        else_branch = luby_parse_block_until(L, p, LUBY_TOK_END);
    }
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    luby_ast_node *node = luby_new_node(L, LUBY_AST_IF, unless_tok.line, unless_tok.column);
    if (!node) return NULL;
    node->as.if_stmt.cond = luby_make_unary(L, LUBY_TOK_NOT, cond, unless_tok.line, unless_tok.column);
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

static luby_ast_node *luby_parse_while(luby_state *L, luby_parser *p) {
    luby_token while_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *cond = luby_parse_expr(L, p, 0);
    luby_ast_node *body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    luby_ast_node *node = luby_new_node(L, LUBY_AST_WHILE, while_tok.line, while_tok.column);
    if (!node) return NULL;
    node->as.while_stmt.cond = cond;
    node->as.while_stmt.body = body;
    return node;
}

static luby_ast_node *luby_parse_for(luby_state *L, luby_parser *p) {
    luby_token for_tok = p->current;
    luby_parser_advance(p);

    luby_ast_node *params = luby_new_node(L, LUBY_AST_BLOCK, for_tok.line, for_tok.column);
    if (!params) return NULL;
    while (p->current.kind == LUBY_TOK_IDENTIFIER) {
        luby_token name = p->current;
        luby_parser_advance(p);
        luby_ast_node *param = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
        if (param) param->as.literal = name.lexeme;
        size_t n = params->as.list.count;
        params->as.list.items = (luby_ast_node **)luby_alloc_raw(L, params->as.list.items, (n + 1) * sizeof(luby_ast_node *));
        if (!params->as.list.items) return NULL;
        params->as.list.items[n] = param;
        params->as.list.count = n + 1;
        if (!luby_parser_match(p, LUBY_TOK_COMMA)) break;
    }
    if (params->as.list.count == 0) {
        luby_parser_error(p, "expected identifier after 'for'");
        return params;
    }
    if (!luby_parser_match(p, LUBY_TOK_IN)) {
        luby_parser_error(p, "expected 'in' after for variables");
        return params;
    }

    luby_ast_node *iterable = luby_parse_expr(L, p, 0);
    if (p->current.kind == LUBY_TOK_DO) {
        luby_parser_advance(p);
    }
    luby_ast_node *body = luby_parse_block_until(L, p, LUBY_TOK_END);
    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }

    luby_ast_node *lambda = luby_new_node(L, LUBY_AST_LAMBDA, for_tok.line, for_tok.column);
    if (!lambda) return NULL;
    lambda->as.lambda.params = params->as.list.items;
    lambda->as.lambda.param_count = params->as.list.count;
    lambda->as.lambda.body = body;
    // Free the wrapper BLOCK node (we moved its contents to lambda)
    luby_alloc_raw(L, params, 0);

    luby_string_view method = { "each", 4 };
    luby_ast_node *call = luby_make_call(L, iterable, method, for_tok.line, for_tok.column);
    if (!call) return NULL;
    call->as.call.block = lambda;
    return call;
}

static luby_ast_node *luby_parse_return(luby_state *L, luby_parser *p) {
    luby_token ret_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *node = luby_new_node(L, LUBY_AST_RETURN, ret_tok.line, ret_tok.column);
    if (!node) return NULL;
    if (p->current.kind != LUBY_TOK_END && p->current.kind != LUBY_TOK_EOF && p->current.kind != LUBY_TOK_SEMI) {
        node->as.ret.value = luby_parse_expr(L, p, 0);
    }
    return node;
}

static luby_ast_node *luby_parse_break(luby_state *L, luby_parser *p, luby_ast_kind kind) {
    luby_token tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *node = luby_new_node(L, kind, tok.line, tok.column);
    if (!node) return NULL;
    if (p->current.kind != LUBY_TOK_END && p->current.kind != LUBY_TOK_EOF &&
        p->current.kind != LUBY_TOK_SEMI && p->current.kind != LUBY_TOK_NEWLINE) {
        node->as.ret.value = luby_parse_expr(L, p, 0);
    }
    return node;
}

static luby_ast_node *luby_parse_alias(luby_state *L, luby_parser *p) {
    luby_token alias_tok = p->current;
    luby_parser_advance(p);  // consume 'alias'
    
    // Parse new name (identifier or symbol)
    luby_ast_node *new_name = NULL;
    if (p->current.kind == LUBY_TOK_IDENTIFIER || p->current.kind == LUBY_TOK_CONSTANT) {
        luby_token name_tok = p->current;
        luby_parser_advance(p);
        new_name = luby_new_node(L, LUBY_AST_SYMBOL, name_tok.line, name_tok.column);
        if (new_name) new_name->as.literal = name_tok.lexeme;
    } else if (p->current.kind == LUBY_TOK_SYMBOL) {
        luby_token name_tok = p->current;
        luby_parser_advance(p);
        new_name = luby_new_node(L, LUBY_AST_SYMBOL, name_tok.line, name_tok.column);
        if (new_name) new_name->as.literal = name_tok.lexeme;
    } else {
        luby_parser_error(p, "expected identifier or symbol after alias");
        return NULL;
    }
    
    // Parse old name (identifier or symbol)
    luby_ast_node *old_name = NULL;
    if (p->current.kind == LUBY_TOK_IDENTIFIER || p->current.kind == LUBY_TOK_CONSTANT) {
        luby_token name_tok = p->current;
        luby_parser_advance(p);
        old_name = luby_new_node(L, LUBY_AST_SYMBOL, name_tok.line, name_tok.column);
        if (old_name) old_name->as.literal = name_tok.lexeme;
    } else if (p->current.kind == LUBY_TOK_SYMBOL) {
        luby_token name_tok = p->current;
        luby_parser_advance(p);
        old_name = luby_new_node(L, LUBY_AST_SYMBOL, name_tok.line, name_tok.column);
        if (old_name) old_name->as.literal = name_tok.lexeme;
    } else {
        luby_parser_error(p, "expected identifier or symbol as second argument to alias");
        return NULL;
    }
    
    // Create a call node for alias(new_name, old_name)
    luby_string_view method = { "alias", 5 };
    luby_ast_node *call = luby_make_call(L, NULL, method, alias_tok.line, alias_tok.column);
    if (!call) return NULL;
    
    call->as.call.args = (luby_ast_node **)luby_alloc_raw(L, NULL, 2 * sizeof(luby_ast_node *));
    if (!call->as.call.args) return NULL;
    call->as.call.args[0] = new_name;
    call->as.call.args[1] = old_name;
    call->as.call.argc = 2;
    
    return call;
}

static luby_ast_node *luby_parse_redo(luby_state *L, luby_parser *p) {
    luby_token tok = p->current;
    luby_parser_advance(p);
    return luby_new_node(L, LUBY_AST_REDO, tok.line, tok.column);
}

static luby_ast_node *luby_parse_begin(luby_state *L, luby_parser *p) {
    luby_token begin_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *body = luby_parse_block_until_any(L, p, LUBY_TOK_RESCUE, LUBY_TOK_ENSURE, LUBY_TOK_END);
    luby_ast_node *rescue_body = NULL;
    luby_ast_node *ensure_body = NULL;

    if (p->current.kind == LUBY_TOK_RESCUE) {
        luby_parser_advance(p);
        rescue_body = luby_parse_block_until_any(L, p, LUBY_TOK_ENSURE, LUBY_TOK_END, LUBY_TOK_EOF);
    }

    if (p->current.kind == LUBY_TOK_ENSURE) {
        luby_parser_advance(p);
        ensure_body = luby_parse_block_until(L, p, LUBY_TOK_END);
    }

    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }

    luby_ast_node *node = luby_new_node(L, LUBY_AST_BEGIN, begin_tok.line, begin_tok.column);
    if (!node) return NULL;
    node->as.begin.body = body;
    node->as.begin.rescue_body = rescue_body;
    node->as.begin.ensure_body = ensure_body;
    return node;
}

static luby_ast_node *luby_parse_case(luby_state *L, luby_parser *p) {
    luby_token case_tok = p->current;
    luby_parser_advance(p);
    luby_ast_node *case_expr = luby_parse_expr(L, p, 0);
    if (!case_expr) return NULL;

    while (p->current.kind == LUBY_TOK_NEWLINE || p->current.kind == LUBY_TOK_SEMI) {
        luby_parser_advance(p);
    }

    luby_ast_node *root = NULL;
    luby_ast_node *tail = NULL;

    if (p->current.kind != LUBY_TOK_WHEN) {
        luby_parser_error(p, "expected 'when'");
        return NULL;
    }

    while (p->current.kind == LUBY_TOK_WHEN) {
        luby_parser_advance(p);
        luby_ast_node *cond = NULL;
        int first_comparison = 1;
        do {
            luby_ast_node *when_expr = luby_parse_expr(L, p, 0);
            if (!when_expr) return NULL;
            luby_ast_node *eq = luby_new_node(L, LUBY_AST_BINARY, case_tok.line, case_tok.column);
            if (!eq) return NULL;
            eq->as.binary.op = LUBY_TOK_EQEQ;
            // First comparison uses the original case_expr, others use copies
            if (first_comparison) {
                eq->as.binary.left = luby_dup_ast(L, case_expr);
                first_comparison = 0;
            } else {
                eq->as.binary.left = luby_dup_ast(L, case_expr);
            }
            eq->as.binary.right = when_expr;
            if (!cond) {
                cond = eq;
            } else {
                luby_ast_node *or_node = luby_new_node(L, LUBY_AST_BINARY, case_tok.line, case_tok.column);
                if (!or_node) return NULL;
                or_node->as.binary.op = LUBY_TOK_OR;
                or_node->as.binary.left = cond;
                or_node->as.binary.right = eq;
                cond = or_node;
            }
        } while (luby_parser_match(p, LUBY_TOK_COMMA));

        if (p->current.kind == LUBY_TOK_THEN) {
            luby_parser_advance(p);
        }

        luby_ast_node *body = luby_parse_block_until_any(L, p, LUBY_TOK_WHEN, LUBY_TOK_ELSE, LUBY_TOK_END);
        luby_ast_node *if_node = luby_new_node(L, LUBY_AST_IF, case_tok.line, case_tok.column);
        if (!if_node) return NULL;
        if_node->as.if_stmt.cond = cond;
        if_node->as.if_stmt.then_branch = body;

        if (!root) {
            root = if_node;
            tail = if_node;
        } else if (tail) {
            tail->as.if_stmt.else_branch = if_node;
            tail = if_node;
        }
    }

    if (p->current.kind == LUBY_TOK_ELSE) {
        luby_parser_advance(p);
        luby_ast_node *else_body = luby_parse_block_until(L, p, LUBY_TOK_END);
        if (tail) {
            tail->as.if_stmt.else_branch = else_body;
        } else {
            root = else_body;
        }
    }

    if (!luby_parser_match(p, LUBY_TOK_END)) {
        luby_parser_error(p, "expected 'end'");
    }
    // Free the original case_expr since all uses are now copies
    luby_free_ast(L, case_expr);
    return root;
}

static luby_ast_node *LUBY_UNUSED luby_parse_assignment(luby_state *L, luby_parser *p) {
    luby_token name = p->current;
    luby_parser_advance(p); // ident
    luby_parser_advance(p); // '='
    luby_ast_node *target = luby_new_node(L, LUBY_AST_IDENT, name.line, name.column);
    if (target) target->as.literal = name.lexeme;
    luby_ast_node *value = luby_parse_expr(L, p, 0);
    luby_ast_node *node = luby_new_node(L, LUBY_AST_ASSIGN, name.line, name.column);
    if (!node) return NULL;
    node->as.assign.target = target;
    node->as.assign.value = value;
    return node;
}

static luby_ast_node *luby_parse_statement(luby_state *L, luby_parser *p) {
    switch (p->current.kind) {
        case LUBY_TOK_DEF: return luby_parse_def(L, p);
        case LUBY_TOK_CLASS: return luby_parse_class(L, p);
        case LUBY_TOK_MODULE: return luby_parse_module(L, p);
        case LUBY_TOK_IF: return luby_parse_if(L, p);
        case LUBY_TOK_UNLESS: return luby_parse_unless(L, p);
        case LUBY_TOK_WHILE: return luby_parse_while(L, p);
        case LUBY_TOK_UNTIL: return luby_parse_until(L, p);
        case LUBY_TOK_FOR: return luby_parse_for(L, p);
        case LUBY_TOK_CASE: return luby_parse_case(L, p);
        case LUBY_TOK_RETURN: return luby_parse_return(L, p);
        case LUBY_TOK_BREAK: return luby_parse_break(L, p, LUBY_AST_BREAK);
        case LUBY_TOK_NEXT: return luby_parse_break(L, p, LUBY_AST_NEXT);
        case LUBY_TOK_REDO: return luby_parse_redo(L, p);
        case LUBY_TOK_BEGIN: return luby_parse_begin(L, p);
        case LUBY_TOK_INCLUDE: return luby_parse_keyword_call(L, p, "include", 7);
        case LUBY_TOK_PREPEND: return luby_parse_keyword_call(L, p, "prepend", 7);
        case LUBY_TOK_EXTEND: return luby_parse_keyword_call(L, p, "extend", 6);
        case LUBY_TOK_ATTR_READER: return luby_parse_keyword_call(L, p, "attr_reader", 11);
        case LUBY_TOK_ATTR_WRITER: return luby_parse_keyword_call(L, p, "attr_writer", 11);
        case LUBY_TOK_ATTR_ACCESSOR: return luby_parse_keyword_call(L, p, "attr_accessor", 13);
        case LUBY_TOK_PRIVATE: return luby_parse_keyword_call(L, p, "private", 7);
        case LUBY_TOK_PUBLIC: return luby_parse_keyword_call(L, p, "public", 6);
        case LUBY_TOK_PROTECTED: return luby_parse_keyword_call(L, p, "protected", 9);
        case LUBY_TOK_ALIAS: return luby_parse_alias(L, p);
        default:
            {
                luby_ast_node *expr = luby_parse_expr(L, p, 0);
                // Check for multiple assignment: a, b = ...
                if (p->current.kind == LUBY_TOK_COMMA) {
                    // Collect targets into a list
                    luby_ast_node **targets = NULL;
                    size_t target_count = 0;
                    size_t target_cap = 0;
                    // Add first target
                    if (target_count + 1 > target_cap) {
                        target_cap = target_cap < 4 ? 4 : target_cap * 2;
                        targets = (luby_ast_node **)luby_alloc_raw(L, targets, target_cap * sizeof(luby_ast_node *));
                    }
                    targets[target_count++] = expr;
                    // Parse remaining targets
                    while (p->current.kind == LUBY_TOK_COMMA) {
                        luby_parser_advance(p); // consume ','
                        luby_ast_node *target = luby_parse_expr(L, p, 0);
                        if (target_count + 1 > target_cap) {
                            target_cap = target_cap * 2;
                            targets = (luby_ast_node **)luby_alloc_raw(L, targets, target_cap * sizeof(luby_ast_node *));
                        }
                        targets[target_count++] = target;
                    }
                    if (p->current.kind != LUBY_TOK_EQ) {
                        luby_parser_error(p, "expected '=' in multiple assignment");
                        return expr;
                    }
                    luby_parser_advance(p); // consume '='
                    // Parse values
                    luby_ast_node **values = NULL;
                    size_t value_count = 0;
                    size_t value_cap = 0;
                    luby_ast_node *first_val = luby_parse_expr(L, p, 0);
                    if (value_cap < 4) { value_cap = 4; values = (luby_ast_node **)luby_alloc_raw(L, values, value_cap * sizeof(luby_ast_node *)); }
                    values[value_count++] = first_val;
                    while (p->current.kind == LUBY_TOK_COMMA) {
                        luby_parser_advance(p);
                        luby_ast_node *val = luby_parse_expr(L, p, 0);
                        if (value_count + 1 > value_cap) {
                            value_cap = value_cap * 2;
                            values = (luby_ast_node **)luby_alloc_raw(L, values, value_cap * sizeof(luby_ast_node *));
                        }
                        values[value_count++] = val;
                    }
                    luby_ast_node *node = luby_new_node(L, LUBY_AST_MULTI_ASSIGN, expr->line, expr->column);
                    if (!node) return NULL;
                    node->as.multi_assign.targets = targets;
                    node->as.multi_assign.target_count = target_count;
                    node->as.multi_assign.values = values;
                    node->as.multi_assign.value_count = value_count;
                    return node;
                }
                if (p->current.kind == LUBY_TOK_EQ) {
                    expr = luby_parse_assignment_from(L, p, expr);
                }
                // Compound assignment: +=, -=, *=, /=, %=
                else if (p->current.kind >= LUBY_TOK_PLUSEQ && p->current.kind <= LUBY_TOK_PERCENTEQ) {
                    expr = luby_parse_compound_assign(L, p, expr);
                }
                // Conditional assignment: ||=, &&=
                else if (p->current.kind == LUBY_TOK_ORASSIGN || p->current.kind == LUBY_TOK_ANDASSIGN) {
                    expr = luby_parse_conditional_assign(L, p, expr);
                }
                // Statement modifiers: expr if/unless cond
                if (p->current.kind == LUBY_TOK_IF) {
                    luby_parser_advance(p); // consume 'if'
                    luby_ast_node *cond = luby_parse_expr(L, p, 0);
                    luby_ast_node *ifn = luby_new_node(L, LUBY_AST_IF, expr->line, expr->column);
                    if (!ifn) return NULL;
                    ifn->as.if_stmt.cond = cond;
                    ifn->as.if_stmt.then_branch = expr;
                    ifn->as.if_stmt.else_branch = NULL;
                    return ifn;
                }
                if (p->current.kind == LUBY_TOK_UNLESS) {
                    luby_parser_advance(p); // consume 'unless'
                    luby_ast_node *cond = luby_parse_expr(L, p, 0);
                    // unless x == if !x, so negate the condition
                    luby_ast_node *notcond = luby_make_unary(L, LUBY_TOK_NOT, cond, cond->line, cond->column);
                    luby_ast_node *ifn = luby_new_node(L, LUBY_AST_IF, expr->line, expr->column);
                    if (!ifn) return NULL;
                    ifn->as.if_stmt.cond = notcond;
                    ifn->as.if_stmt.then_branch = expr;
                    ifn->as.if_stmt.else_branch = NULL;
                    return ifn;
                }
                if (p->current.kind == LUBY_TOK_WHILE) {
                    luby_parser_advance(p); // consume 'while'
                    luby_ast_node *cond = luby_parse_expr(L, p, 0);
                    luby_ast_node *wn = luby_new_node(L, LUBY_AST_WHILE, expr->line, expr->column);
                    if (!wn) return NULL;
                    wn->as.while_stmt.cond = cond;
                    wn->as.while_stmt.body = expr;
                    return wn;
                }
                if (p->current.kind == LUBY_TOK_UNTIL) {
                    luby_parser_advance(p); // consume 'until'
                    luby_ast_node *cond = luby_parse_expr(L, p, 0);
                    // until is like while-not, reuse WHILE with negated condition
                    luby_ast_node *notcond = luby_make_unary(L, LUBY_TOK_NOT, cond, cond->line, cond->column);
                    luby_ast_node *wn = luby_new_node(L, LUBY_AST_WHILE, expr->line, expr->column);
                    if (!wn) return NULL;
                    wn->as.while_stmt.cond = notcond;
                    wn->as.while_stmt.body = expr;
                    return wn;
                }
                return expr;
            }
    }
}

// Handle conditional assignment: x ||= y and x &&= y
// x ||= y means: if x is nil or false, assign y to x
// x &&= y means: if x is truthy, assign y to x
static luby_ast_node *luby_parse_conditional_assign(luby_state *L, luby_parser *p, luby_ast_node *lhs) {
    if (!lhs) return NULL;
    luby_token op = p->current;
    int is_or = (op.kind == LUBY_TOK_ORASSIGN);
    luby_parser_advance(p); // consume ||= or &&=
    luby_ast_node *rhs = luby_parse_expr(L, p, 0);
    
    // Create binary node: lhs || rhs or lhs && rhs
    luby_ast_node *binop = luby_new_node(L, LUBY_AST_BINARY, op.line, op.column);
    if (!binop) return NULL;
    binop->as.binary.op = is_or ? LUBY_TOK_OROR : LUBY_TOK_ANDAND;
    binop->as.binary.left = lhs;
    binop->as.binary.right = rhs;
    
    // Create assignment: lhs = binop
    if (lhs->kind == LUBY_AST_IDENT) {
        luby_ast_node *target = luby_new_node(L, LUBY_AST_IDENT, lhs->line, lhs->column);
        if (target) target->as.literal = lhs->as.literal;
        luby_ast_node *node = luby_new_node(L, LUBY_AST_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = target;
        node->as.assign.value = binop;
        return node;
    }
    if (lhs->kind == LUBY_AST_IVAR) {
        luby_ast_node *target = luby_new_node(L, LUBY_AST_IVAR, lhs->line, lhs->column);
        if (target) target->as.literal = lhs->as.literal;
        luby_ast_node *node = luby_new_node(L, LUBY_AST_IVAR_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = target;
        node->as.assign.value = binop;
        return node;
    }
    luby_parser_error(p, "invalid conditional assignment target");
    return lhs;
}

// Convert compound assignment op to binary op
static luby_token_kind luby_compound_to_binary(luby_token_kind kind) {
    switch (kind) {
        case LUBY_TOK_PLUSEQ: return LUBY_TOK_PLUS;
        case LUBY_TOK_MINUSEQ: return LUBY_TOK_MINUS;
        case LUBY_TOK_STAREQ: return LUBY_TOK_STAR;
        case LUBY_TOK_SLASHEQ: return LUBY_TOK_SLASH;
        case LUBY_TOK_PERCENTEQ: return LUBY_TOK_PERCENT;
        default: return LUBY_TOK_PLUS;
    }
}

// Handle compound assignment: x += y becomes x = x + y
static luby_ast_node *luby_parse_compound_assign(luby_state *L, luby_parser *p, luby_ast_node *lhs) {
    if (!lhs) return NULL;
    luby_token op = p->current;
    luby_parser_advance(p); // consume +=, -=, etc.
    luby_ast_node *rhs = luby_parse_expr(L, p, 0);
    
    // Create binary node: lhs op rhs
    luby_ast_node *binop = luby_new_node(L, LUBY_AST_BINARY, op.line, op.column);
    if (!binop) return NULL;
    binop->as.binary.op = luby_compound_to_binary(op.kind);
    binop->as.binary.left = lhs;
    binop->as.binary.right = rhs;
    
    // Create assignment: lhs = binop
    if (lhs->kind == LUBY_AST_IDENT) {
        // Need to create a fresh copy of lhs for the assignment target
        luby_ast_node *target = luby_new_node(L, LUBY_AST_IDENT, lhs->line, lhs->column);
        if (target) target->as.literal = lhs->as.literal;
        luby_ast_node *node = luby_new_node(L, LUBY_AST_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = target;
        node->as.assign.value = binop;
        return node;
    }
    if (lhs->kind == LUBY_AST_IVAR) {
        luby_ast_node *target = luby_new_node(L, LUBY_AST_IVAR, lhs->line, lhs->column);
        if (target) target->as.literal = lhs->as.literal;
        luby_ast_node *node = luby_new_node(L, LUBY_AST_IVAR_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = target;
        node->as.assign.value = binop;
        return node;
    }
    luby_parser_error(p, "invalid compound assignment target");
    return lhs;
}

static luby_ast_node *luby_parse_assignment_from(luby_state *L, luby_parser *p, luby_ast_node *lhs) {
    if (!lhs) return NULL;
    luby_parser_advance(p); // consume '='
    luby_ast_node *rhs = luby_parse_expr(L, p, 0);
    if (lhs->kind == LUBY_AST_IDENT) {
        luby_ast_node *node = luby_new_node(L, LUBY_AST_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = lhs;
        node->as.assign.value = rhs;
        return node;
    }
    if (lhs->kind == LUBY_AST_IVAR) {
        luby_ast_node *node = luby_new_node(L, LUBY_AST_IVAR_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.assign.target = lhs;
        node->as.assign.value = rhs;
        return node;
    }
    if (lhs->kind == LUBY_AST_INDEX) {
        luby_ast_node *node = luby_new_node(L, LUBY_AST_INDEX_ASSIGN, lhs->line, lhs->column);
        if (!node) return NULL;
        node->as.index_assign.target = lhs->as.index.target;
        node->as.index_assign.index = lhs->as.index.index;
        node->as.index_assign.value = rhs;
        // Free the wrapper INDEX node (but not its children, we just moved them)
        luby_alloc_raw(L, lhs, 0);
        return node;
    }
    luby_parser_error(p, "invalid assignment target");
    return lhs;
}

static luby_ast_node *luby_parse_block_until(luby_state *L, luby_parser *p, luby_token_kind end_kind) {
    luby_ast_node *block = luby_new_node(L, LUBY_AST_BLOCK, p->current.line, p->current.column);
    if (!block) return NULL;
    while (p->current.kind != end_kind && p->current.kind != LUBY_TOK_EOF) {
        while (p->current.kind == LUBY_TOK_NEWLINE || p->current.kind == LUBY_TOK_SEMI) {
            luby_parser_advance(p);
        }
        if (p->current.kind == end_kind || p->current.kind == LUBY_TOK_EOF) break;
        luby_ast_node *stmt = luby_parse_statement(L, p);
        if (!stmt) break;
        size_t n = block->as.list.count;
        block->as.list.items = (luby_ast_node **)luby_alloc_raw(L, block->as.list.items, (n + 1) * sizeof(luby_ast_node *));
        if (!block->as.list.items) return NULL;
        block->as.list.items[n] = stmt;
        block->as.list.count = n + 1;
        if (p->current.kind == LUBY_TOK_SEMI || p->current.kind == LUBY_TOK_NEWLINE) {
            luby_parser_advance(p);
        }
    }
    return block;
}

static luby_ast_node *luby_parse_block_until_any(luby_state *L, luby_parser *p, luby_token_kind a, luby_token_kind b, luby_token_kind c) {
    luby_ast_node *block = luby_new_node(L, LUBY_AST_BLOCK, p->current.line, p->current.column);
    if (!block) return NULL;
    while (p->current.kind != a && p->current.kind != b && p->current.kind != c && p->current.kind != LUBY_TOK_EOF) {
        while (p->current.kind == LUBY_TOK_NEWLINE || p->current.kind == LUBY_TOK_SEMI) {
            luby_parser_advance(p);
        }
        if (p->current.kind == a || p->current.kind == b || p->current.kind == c || p->current.kind == LUBY_TOK_EOF) break;
        luby_ast_node *stmt = luby_parse_statement(L, p);
        if (!stmt) break;
        size_t n = block->as.list.count;
        block->as.list.items = (luby_ast_node **)luby_alloc_raw(L, block->as.list.items, (n + 1) * sizeof(luby_ast_node *));
        if (!block->as.list.items) return NULL;
        block->as.list.items[n] = stmt;
        block->as.list.count = n + 1;
        if (p->current.kind == LUBY_TOK_SEMI || p->current.kind == LUBY_TOK_NEWLINE) {
            luby_parser_advance(p);
        }
    }
    return block;
}

LUBY_API luby_ast_node *luby_parse(luby_state *L, const char *code, size_t len, const char *filename, luby_error *out_error) {
    if (out_error) {
        out_error->code = LUBY_E_OK;
        out_error->message = NULL;
        out_error->file = filename;
        out_error->line = 0;
        out_error->column = 0;
    }

    luby_parser p;
    luby_parser_init(&p, code, len, filename, out_error);

    return luby_parse_block_until(L, &p, LUBY_TOK_EOF);
}

// ------------------------------ Compiler -----------------------------------

static void luby_chunk_free(luby_state *L, luby_chunk *chunk);

static void luby_chunk_init(luby_chunk *chunk) {
    memset(chunk, 0, sizeof(*chunk));
}

static void luby_proc_free(luby_state *L, luby_proc *proc) {
    if (!proc) return;
    for (size_t i = 0; i < proc->param_count; i++) {
        luby_alloc_raw(L, proc->param_names[i], 0);
    }
    luby_alloc_raw(L, proc->param_names, 0);
    for (size_t i = 0; i < proc->local_count; i++) {
        luby_alloc_raw(L, proc->local_names[i], 0);
    }
    luby_alloc_raw(L, proc->local_names, 0);
    if (proc->block_param_name) luby_alloc_raw(L, proc->block_param_name, 0);
    if (proc->default_chunks) {
        for (size_t i = 0; i < proc->param_count; i++) {
            luby_chunk_free(L, &proc->default_chunks[i]);
        }
        luby_alloc_raw(L, proc->default_chunks, 0);
    }
    luby_chunk_free(L, &proc->chunk);
    // Note: the proc struct itself is freed by the GC sweep, not here.
}

static void luby_chunk_free(luby_state *L, luby_chunk *chunk) {
    if (!chunk) return;
    // String consts are GC-tracked; symbol consts are interned; proc consts are GC-tracked.
    // Only free the infrastructure arrays (code, lines, consts).
    luby_alloc_raw(L, chunk->code, 0);
    luby_alloc_raw(L, chunk->lines, 0);
    luby_alloc_raw(L, chunk->consts, 0);
    memset(chunk, 0, sizeof(*chunk));
}

static uint32_t luby_chunk_add_const(luby_state *L, luby_chunk *chunk, luby_value v) {
    if (chunk->const_count + 1 > chunk->const_capacity) {
        size_t new_cap = chunk->const_capacity < 8 ? 8 : chunk->const_capacity * 2;
        luby_value *nv = (luby_value *)luby_alloc_raw(L, chunk->consts, new_cap * sizeof(luby_value));
        if (!nv) return 0;
        chunk->consts = nv;
        chunk->const_capacity = new_cap;
    }
    chunk->consts[chunk->const_count] = v;
    return (uint32_t)chunk->const_count++;
}

static void luby_chunk_emit(luby_state *L, luby_chunk *chunk, luby_op op, uint8_t a, uint16_t b, uint32_t c, int line) {
    if (chunk->count + 1 > chunk->capacity) {
        size_t new_cap = chunk->capacity < 16 ? 16 : chunk->capacity * 2;
        luby_inst *ni = (luby_inst *)luby_alloc_raw(L, chunk->code, new_cap * sizeof(luby_inst));
        if (!ni) return;
        chunk->code = ni;
        int *nl = (int *)luby_alloc_raw(L, chunk->lines, new_cap * sizeof(int));
        if (!nl) return;
        chunk->lines = nl;
        chunk->capacity = new_cap;
    }
    luby_inst inst;
    inst.op = (uint8_t)op;
    inst.a = a;
    inst.b = b;
    inst.c = c;
    chunk->code[chunk->count++] = inst;
    chunk->lines[chunk->count - 1] = line;
}

static size_t luby_chunk_emit_jump(luby_state *L, luby_chunk *chunk, luby_op op, int line) {
    luby_chunk_emit(L, chunk, op, 0, 0, 0, line);
    return chunk->count - 1;
}

static void luby_chunk_patch_jump(luby_chunk *chunk, size_t at, size_t target) {
    if (!chunk || at >= chunk->count) return;
    chunk->code[at].c = (uint32_t)target;
}

static const uint32_t LUBY_IP_NONE = 0xFFFFFFFFu;

static int luby_loop_add_break(luby_compiler *C, size_t at) {
    if (!C || C->loop_depth <= 0) return 0;
    int idx = C->loop_depth - 1;
    size_t count = C->loops[idx].break_count;
    if (count + 1 > C->loops[idx].break_capacity) {
        size_t new_cap = C->loops[idx].break_capacity < 8 ? 8 : C->loops[idx].break_capacity * 2;
        size_t *nb = (size_t *)luby_alloc_raw(C->L, C->loops[idx].breaks, new_cap * sizeof(size_t));
        if (!nb) return 0;
        C->loops[idx].breaks = nb;
        C->loops[idx].break_capacity = new_cap;
    }
    C->loops[idx].breaks[count] = at;
    C->loops[idx].break_count = count + 1;
    return 1;
}

static int luby_emit_nil(luby_compiler *C, int line) {
    luby_value v = luby_nil();
    uint32_t idx = luby_chunk_add_const(C->L, C->chunk, v);
    luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONST, 0, 0, idx, line);
    return 1;
}

static int luby_vm_handle_error(luby_state *L, luby_vm_handler *handlers, int *hcount, size_t *ip, int *sp) {
    while (*hcount > 0) {
        luby_vm_handler *h = &handlers[*hcount - 1];
        *sp = h->sp;
        if (h->phase == 0 && h->rescue_ip != LUBY_IP_NONE) {
            h->phase = 1;
            luby_clear_error(L);
            *ip = h->rescue_ip;
            return 1;
        }
        if (h->ensure_ip != LUBY_IP_NONE && h->phase < 2) {
            h->phase = 2;
            h->pending = L->last_error;
            h->pending_error = 1;
            luby_clear_error(L);
            *ip = h->ensure_ip;
            return 1;
        }
        (*hcount)--;
    }
    return 0;
}

static char *luby_dup_string(luby_state *L, const char *data, size_t len) {
    char *buf = (char *)luby_alloc_raw(L, NULL, len + 1);
    if (!buf) return NULL;
    memcpy(buf, data, len);
    buf[len] = '\0';
    return buf;
}

static void luby_vm_init(luby_vm *vm) {
    if (!vm) return;
    memset(vm, 0, sizeof(*vm));
}

static void luby_vm_free(luby_state *L, luby_vm *vm) {
    if (!vm) return;
    for (int i = vm->frame_count - 1; i >= 0; i--) {
        luby_vm_frame *f = &vm->frames[i];
        if (f->param_existed) {
            luby_alloc_raw(L, f->param_existed, 0);
        }
        if (f->param_saved) {
            luby_alloc_raw(L, f->param_saved, 0);
        }
    }
    luby_alloc_raw(L, vm->frames, 0);
    luby_alloc_raw(L, vm->stack, 0);
    memset(vm, 0, sizeof(*vm));
}

static int luby_vm_ensure_stack(luby_state *L, luby_vm *vm, int need) {
    if (!vm) return 0;
    if (vm->sp + need <= vm->stack_capacity) return 1;
    int new_cap = vm->stack_capacity < 256 ? 256 : vm->stack_capacity;
    while (vm->sp + need > new_cap) new_cap *= 2;
    luby_value *ns = (luby_value *)luby_alloc_raw(L, vm->stack, (size_t)new_cap * sizeof(luby_value));
    if (!ns) return 0;
    vm->stack = ns;
    vm->stack_capacity = new_cap;
    return 1;
}

static int luby_vm_ensure_frames(luby_state *L, luby_vm *vm) {
    if (!vm) return 0;
    if (vm->frame_count + 1 <= vm->frame_capacity) return 1;
    int new_cap = vm->frame_capacity < 8 ? 8 : vm->frame_capacity * 2;
    luby_vm_frame *nf = (luby_vm_frame *)luby_alloc_raw(L, vm->frames, (size_t)new_cap * sizeof(luby_vm_frame));
    if (!nf) return 0;
    vm->frames = nf;
    vm->frame_capacity = new_cap;
    return 1;
}

static int luby_execute_chunk(luby_state *L, luby_chunk *chunk, luby_value *out, const char *filename);

static int luby_vm_push_frame(luby_state *L, luby_vm *vm, luby_proc *proc, luby_chunk *chunk, const char *filename,
    luby_value recv, luby_class_obj *method_class, const char *method_name, int argc, const luby_value *argv, luby_value block, int set_self) {
    if (!L || !vm || !chunk) return 0;
    if (!luby_vm_ensure_frames(L, vm)) return 0;

    luby_vm_frame *f = &vm->frames[vm->frame_count++];
    memset(f, 0, sizeof(*f));
    f->proc = proc;
    f->chunk = chunk;
    f->ip = 0;
    f->filename = filename ? filename : "<chunk>";
    f->hcount = 0;
    f->stack_base = vm->sp;

    f->saved_block = L->current_block;
    f->saved_self = L->current_self;
    f->saved_method_class = L->current_method_class;
    f->saved_method_name = L->current_method_name;
    f->set_self = set_self;
    f->self_existed = 0;
    f->self_saved = luby_nil();

    if (set_self) {
        luby_string_view self_name = { "self", 4 };
        int self_idx = luby_find_global(L, self_name);
        if (self_idx >= 0) {
            f->self_existed = 1;
            f->self_saved = L->global_values[self_idx];
        }
        luby_set_global(L, self_name, recv);
        L->current_self = recv;
    }

    L->current_method_class = method_class;
    L->current_method_name = method_name;
    L->current_block = block;

    f->param_count = (proc && proc->param_count > 0) ? proc->param_count : 0;
    if (f->param_count > 0) {
        f->param_existed = (int *)luby_alloc_raw(L, NULL, f->param_count * sizeof(int));
        f->param_saved = (luby_value *)luby_alloc_raw(L, NULL, f->param_count * sizeof(luby_value));
        if (!f->param_existed || !f->param_saved) {
            luby_alloc_raw(L, f->param_existed, 0);
            luby_alloc_raw(L, f->param_saved, 0);
            vm->frame_count--;
            return 0;
        }
        
        int splat_idx = proc->splat_index;
        size_t regular_count = (splat_idx >= 0) ? (size_t)splat_idx : f->param_count;
        
        for (size_t i = 0; i < f->param_count; i++) {
            luby_string_view name = { proc->param_names[i], proc->param_names[i] ? strlen(proc->param_names[i]) : 0 };
            int idx = luby_find_global(L, name);
            f->param_existed[i] = idx;
            if (idx >= 0) f->param_saved[i] = L->global_values[idx];
            
            luby_value v = luby_nil();
            if (splat_idx >= 0 && (int)i == splat_idx) {
                // Collect remaining args into array for splat param
                luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
                if (arr) {
                    size_t splat_count = ((size_t)argc > regular_count) ? (size_t)argc - regular_count : 0;
                    arr->count = splat_count;
                    arr->capacity = splat_count > 0 ? splat_count : 1;
                    arr->frozen = 0;
                    arr->items = (luby_value *)luby_alloc_raw(L, NULL, arr->capacity * sizeof(luby_value));
                    for (size_t j = 0; j < splat_count; j++) {
                        arr->items[j] = argv[regular_count + j];
                    }
                    v.type = LUBY_T_ARRAY;
                    v.as.ptr = arr;
                }
            } else if (i < (size_t)argc) {
                v = argv[i];
            } else if (proc->default_chunks && proc->default_chunks[i].count > 0) {
                // Evaluate default value
                luby_value def_val = luby_nil();
                luby_execute_chunk(L, &proc->default_chunks[i], &def_val, "<default>");
                v = def_val;
            }
            luby_set_global(L, name, v);
        }
    }
    
    // Handle block param: &block
    if (proc && proc->has_block_param && proc->block_param_name) {
        luby_string_view name = { proc->block_param_name, strlen(proc->block_param_name) };
        luby_set_global(L, name, block);
    }

    // Save/clear local variables so they don't leak into the callee's scope
    f->local_count = (proc && proc->local_count > 0) ? proc->local_count : 0;
    f->local_existed = NULL;
    f->local_saved = NULL;
    if (f->local_count > 0) {
        f->local_existed = (int *)luby_alloc_raw(L, NULL, f->local_count * sizeof(int));
        f->local_saved = (luby_value *)luby_alloc_raw(L, NULL, f->local_count * sizeof(luby_value));
        if (f->local_existed && f->local_saved) {
            for (size_t i = 0; i < f->local_count; i++) {
                luby_string_view name = { proc->local_names[i], strlen(proc->local_names[i]) };
                int idx = luby_find_global(L, name);
                f->local_existed[i] = idx;
                if (idx >= 0) {
                    f->local_saved[i] = L->global_values[idx];
                    // Remove the local so it starts fresh in this function call
                    luby_remove_global(L, name);
                }
            }
        }
    }

    return 1;
}

static void luby_vm_pop_frame(luby_state *L, luby_vm *vm, luby_value ret, int push_ret) {
    if (!L || !vm || vm->frame_count <= 0) return;
    luby_vm_frame *f = &vm->frames[vm->frame_count - 1];

    if (f->proc && f->param_count > 0 && f->param_existed && f->param_saved) {
        for (size_t i = 0; i < f->param_count; i++) {
            luby_string_view name = { f->proc->param_names[i], f->proc->param_names[i] ? strlen(f->proc->param_names[i]) : 0 };
            if (f->param_existed[i] >= 0) {
                luby_set_global(L, name, f->param_saved[i]);
            } else {
                luby_remove_global(L, name);
            }
        }
    }
    
    // Clean up block param
    if (f->proc && f->proc->has_block_param && f->proc->block_param_name) {
        luby_string_view name = { f->proc->block_param_name, strlen(f->proc->block_param_name) };
        luby_remove_global(L, name);
    }

    // Restore local variables
    if (f->proc && f->local_count > 0 && f->local_existed && f->local_saved) {
        for (size_t i = 0; i < f->local_count; i++) {
            luby_string_view name = { f->proc->local_names[i], strlen(f->proc->local_names[i]) };
            // First remove whatever the function body left behind
            luby_remove_global(L, name);
            // Then restore if it existed before
            if (f->local_existed[i] >= 0) {
                luby_set_global(L, name, f->local_saved[i]);
            }
        }
    }

    if (f->param_existed) luby_alloc_raw(L, f->param_existed, 0);
    if (f->param_saved) luby_alloc_raw(L, f->param_saved, 0);
    f->param_existed = NULL;
    f->param_saved = NULL;
    f->param_count = 0;
    if (f->local_existed) luby_alloc_raw(L, f->local_existed, 0);
    if (f->local_saved) luby_alloc_raw(L, f->local_saved, 0);
    f->local_existed = NULL;
    f->local_saved = NULL;
    f->local_count = 0;

    if (f->set_self) {
        luby_string_view self_name = { "self", 4 };
        if (f->self_existed) {
            luby_set_global(L, self_name, f->self_saved);
        } else {
            luby_remove_global(L, self_name);
        }
    }

    L->current_block = f->saved_block;
    L->current_self = f->saved_self;
    L->current_method_class = f->saved_method_class;
    L->current_method_name = f->saved_method_name;

    // Check for return override (used by new/initialize)
    luby_value final_ret = f->has_return_override ? f->return_override : ret;

    vm->sp = f->stack_base;
    vm->frame_count--;

    if (push_ret) {
        if (luby_vm_ensure_stack(L, vm, 1)) {
            vm->stack[vm->sp++] = final_ret;
        }
    }
}

static int luby_vm_run(luby_state *L, luby_vm *vm, luby_value *out) {
    if (!L || !vm) return (int)LUBY_E_RUNTIME;
    if (vm->resume_pending) {
        if (!luby_vm_ensure_stack(L, vm, 1)) return (int)LUBY_E_OOM;
        vm->stack[vm->sp++] = vm->resume_value;
        vm->resume_pending = 0;
    }

    void *saved_vm = L->current_vm;
    L->current_vm = vm;

    luby_vm_frame *f = NULL;
    luby_chunk *chunk = NULL;

    while (vm->frame_count > 0) {
        f = &vm->frames[vm->frame_count - 1];
        chunk = f->chunk;
        int switched = 0;
        while (f->ip < chunk->count) {
    vm_continue: ;
            luby_inst inst = chunk->code[f->ip];
            int line = chunk->lines ? chunk->lines[f->ip] : 0;
            switch ((luby_op)inst.op) {
                case LUBY_OP_CONST:
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = chunk->consts[inst.c];
                    break;
                case LUBY_OP_POP:
                    if (vm->sp > f->stack_base) vm->sp--;
                    break;
                case LUBY_OP_SET_BLOCK:
                    L->current_block = chunk->consts[inst.c];
                    break;
                case LUBY_OP_GET_CLASS:
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = L->current_class;
                    break;
                case LUBY_OP_SET_CLASS:
                    if (vm->sp > f->stack_base) {
                        L->current_class = vm->stack[--vm->sp];
                    } else {
                        L->current_class = luby_nil();
                    }
                    break;
                case LUBY_OP_MAKE_CLASS: {
                    luby_value namev = chunk->consts[inst.c];
                    const char *name = namev.as.ptr ? (const char *)namev.as.ptr : "<class>";
                    luby_class_obj *super = NULL;
                    if (inst.b != 0xFFFF) {
                        luby_value superv = chunk->consts[inst.b];
                        const char *sname = superv.as.ptr ? (const char *)superv.as.ptr : NULL;
                        if (sname) {
                            luby_string_view sv = { sname, strlen(sname) };
                            luby_value gv = luby_get_global(L, sv);
                            if (gv.type == LUBY_T_CLASS) super = (luby_class_obj *)gv.as.ptr;
                        }
                    }
                    luby_class_obj *cls = luby_class_new(L, name, super);
                    if (!cls) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    if (super) {
                        luby_value superv; superv.type = LUBY_T_CLASS; superv.as.ptr = super;
                        luby_value cv; cv.type = LUBY_T_CLASS; cv.as.ptr = cls;
                        int hook_rc = luby_call_hook_if_exists(L, superv, "inherited", cv);
                        if (hook_rc == (int)LUBY_E_RUNTIME) { luby_set_error(L, LUBY_E_RUNTIME, "inherited hook failed", f->filename, line, 0); goto vm_error; }
                    }
                    luby_value v; v.type = LUBY_T_CLASS; v.as.ptr = cls;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = v;
                    break;
                }
                case LUBY_OP_MAKE_MODULE: {
                    luby_value namev = chunk->consts[inst.c];
                    const char *name = namev.as.ptr ? (const char *)namev.as.ptr : "<module>";
                    luby_class_obj *mod = luby_class_new(L, name, NULL);
                    if (!mod) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    luby_value v; v.type = LUBY_T_MODULE; v.as.ptr = mod;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = v;
                    break;
                }
                case LUBY_OP_DEF_METHOD: {
                    if (vm->sp <= f->stack_base) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value procv = vm->stack[--vm->sp];
                    luby_value namev = chunk->consts[inst.c];
                    const char *mname = namev.as.ptr ? (const char *)namev.as.ptr : "";
                    if (L->current_class.type == LUBY_T_CLASS || L->current_class.type == LUBY_T_MODULE) {
                        luby_class_obj *cls = (luby_class_obj *)L->current_class.as.ptr;
                        if (cls && cls->frozen) { luby_set_error(L, LUBY_E_RUNTIME, "frozen", f->filename, line, 0); goto vm_error; }
                        if (procv.type == LUBY_T_PROC) {
                            luby_class_set_method(L, cls, mname, (luby_proc *)procv.as.ptr);
                        }
                    }
                    break;
                }
                case LUBY_OP_DEF_SINGLETON: {
                    // Stack: [proc, receiver] -> []
                    if (vm->sp < f->stack_base + 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value recv = vm->stack[--vm->sp];
                    luby_value procv = vm->stack[--vm->sp];
                    luby_value namev = chunk->consts[inst.c];
                    const char *mname = namev.as.ptr ? (const char *)namev.as.ptr : "";
                    if (procv.type == LUBY_T_PROC) {
                        if (recv.type == LUBY_T_OBJECT) {
                            luby_object *obj = (luby_object *)recv.as.ptr;
                            if (obj && obj->frozen) { luby_set_error(L, LUBY_E_RUNTIME, "frozen", f->filename, line, 0); goto vm_error; }
                            luby_object_set_singleton_method(L, obj, mname, (luby_proc *)procv.as.ptr);
                        } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
                            luby_class_obj *cls = (luby_class_obj *)recv.as.ptr;
                            if (cls && cls->frozen) { luby_set_error(L, LUBY_E_RUNTIME, "frozen", f->filename, line, 0); goto vm_error; }
                            luby_class_set_singleton_method(L, cls, mname, (luby_proc *)procv.as.ptr);
                        } else {
                            luby_set_error(L, LUBY_E_TYPE, "cannot define singleton method on this type", f->filename, line, 0);
                            goto vm_error;
                        }
                    }
                    break;
                }
                case LUBY_OP_GET_GLOBAL: {
                    luby_value sym = chunk->consts[inst.c];
                    luby_string_view name = { (const char *)sym.as.ptr, sym.as.ptr ? strlen((const char *)sym.as.ptr) : 0 };
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = luby_get_global(L, name);
                    break;
                }
                case LUBY_OP_SET_GLOBAL: {
                    luby_value sym = chunk->consts[inst.c];
                    luby_string_view name = { (const char *)sym.as.ptr, sym.as.ptr ? strlen((const char *)sym.as.ptr) : 0 };
                    luby_value v = (vm->sp > f->stack_base) ? vm->stack[--vm->sp] : luby_nil();
                    luby_set_global(L, name, v);
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = v;
                    break;
                }
                case LUBY_OP_ADD:
                case LUBY_OP_SUB:
                case LUBY_OP_MUL:
                case LUBY_OP_DIV:
                case LUBY_OP_MOD: {
                    if (vm->sp - f->stack_base < 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value b = vm->stack[--vm->sp];
                    luby_value a = vm->stack[--vm->sp];
                    if (inst.op == LUBY_OP_ADD && a.type == LUBY_T_STRING && b.type == LUBY_T_STRING) {
                        const char *sa = a.as.ptr ? (const char *)a.as.ptr : "";
                        const char *sb = b.as.ptr ? (const char *)b.as.ptr : "";
                        size_t la = strlen(sa), lb = strlen(sb);
                        // Pause GC - a and b are popped, allocation could free them
                        int was_paused = L->gc_paused;
                        L->gc_paused = 1;
                        char *buf = luby_gc_alloc_string(L, NULL, la + lb);
                        if (!buf) { L->gc_paused = was_paused; luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                        memcpy(buf, sa, la);
                        memcpy(buf + la, sb, lb);
                        buf[la + lb] = '\0';
                        L->gc_paused = was_paused;
                        luby_value sv; sv.type = LUBY_T_STRING; sv.as.ptr = buf;
                        vm->stack[vm->sp++] = sv;
                    } else if (inst.op == LUBY_OP_ADD && a.type == LUBY_T_ARRAY && b.type == LUBY_T_ARRAY) {
                        luby_array *aa = (luby_array *)a.as.ptr;
                        luby_array *ba = (luby_array *)b.as.ptr;
                        size_t ac = aa ? aa->count : 0, bc = ba ? ba->count : 0;
                        // Pause GC - a and b are popped, allocation could free them
                        int was_paused = L->gc_paused;
                        L->gc_paused = 1;
                        luby_array *ra = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
                        if (!ra) { L->gc_paused = was_paused; luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                        ra->count = ac + bc; ra->capacity = ac + bc > 0 ? ac + bc : 1; ra->frozen = 0;
                        ra->items = (luby_value *)luby_alloc_raw(L, NULL, ra->capacity * sizeof(luby_value));
                        if (ac > 0) memcpy(ra->items, aa->items, ac * sizeof(luby_value));
                        if (bc > 0) memcpy(ra->items + ac, ba->items, bc * sizeof(luby_value));
                        L->gc_paused = was_paused;
                        luby_value rv; rv.type = LUBY_T_ARRAY; rv.as.ptr = ra;
                        vm->stack[vm->sp++] = rv;
                    } else if (a.type == LUBY_T_INT && b.type == LUBY_T_INT) {
                        int64_t r = 0;
                        if (inst.op == LUBY_OP_ADD) r = a.as.i + b.as.i;
                        else if (inst.op == LUBY_OP_SUB) r = a.as.i - b.as.i;
                        else if (inst.op == LUBY_OP_MUL) r = a.as.i * b.as.i;
                        else if (inst.op == LUBY_OP_DIV) r = b.as.i ? a.as.i / b.as.i : 0;
                        else r = b.as.i ? a.as.i % b.as.i : 0;
                        vm->stack[vm->sp++] = luby_int(r);
                    } else {
                        double af = (a.type == LUBY_T_FLOAT) ? a.as.f : (double)a.as.i;
                        double bf = (b.type == LUBY_T_FLOAT) ? b.as.f : (double)b.as.i;
                        double r = 0.0;
                        if (inst.op == LUBY_OP_ADD) r = af + bf;
                        else if (inst.op == LUBY_OP_SUB) r = af - bf;
                        else if (inst.op == LUBY_OP_MUL) r = af * bf;
                        else if (inst.op == LUBY_OP_DIV) r = bf != 0.0 ? af / bf : 0.0;
                        else r = bf != 0.0 ? fmod(af, bf) : 0.0;
                        vm->stack[vm->sp++] = luby_float(r);
                    }
                    break;
                }
                case LUBY_OP_AND:
                case LUBY_OP_OR: {
                    if (vm->sp - f->stack_base < 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value b = vm->stack[--vm->sp];
                    luby_value a = vm->stack[--vm->sp];
                    int av = luby_is_truthy(a);
                    int bv = luby_is_truthy(b);
                    int res = (inst.op == LUBY_OP_AND) ? (av && bv) : (av || bv);
                    vm->stack[vm->sp++] = luby_bool(res);
                    break;
                }
                case LUBY_OP_NOT: {
                    if (vm->sp - f->stack_base < 1) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value a = vm->stack[--vm->sp];
                    vm->stack[vm->sp++] = luby_bool(!luby_is_truthy(a));
                    break;
                }
                case LUBY_OP_NEG: {
                    if (vm->sp - f->stack_base < 1) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value a = vm->stack[--vm->sp];
                    if (a.type == LUBY_T_INT) vm->stack[vm->sp++] = luby_int(-a.as.i);
                    else if (a.type == LUBY_T_FLOAT) vm->stack[vm->sp++] = luby_float(-a.as.f);
                    else vm->stack[vm->sp++] = luby_nil();
                    break;
                }
                case LUBY_OP_EQ:
                case LUBY_OP_LT:
                case LUBY_OP_LTE:
                case LUBY_OP_GT:
                case LUBY_OP_GTE: {
                    if (vm->sp - f->stack_base < 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value b = vm->stack[--vm->sp];
                    luby_value a = vm->stack[--vm->sp];
                    int res = 0;
                    if (inst.op == LUBY_OP_EQ) {
                        res = luby_value_eq(a, b);
                    } else if (a.type == LUBY_T_INT && b.type == LUBY_T_INT) {
                        if (inst.op == LUBY_OP_LT) res = (a.as.i < b.as.i);
                        else if (inst.op == LUBY_OP_LTE) res = (a.as.i <= b.as.i);
                        else if (inst.op == LUBY_OP_GT) res = (a.as.i > b.as.i);
                        else res = (a.as.i >= b.as.i);
                    } else if ((a.type == LUBY_T_STRING || a.type == LUBY_T_SYMBOL) && a.type == b.type) {
                        const char *sa = a.as.ptr ? (const char *)a.as.ptr : "";
                        const char *sb = b.as.ptr ? (const char *)b.as.ptr : "";
                        int cmp = strcmp(sa, sb);
                        if (inst.op == LUBY_OP_LT) res = (cmp < 0);
                        else if (inst.op == LUBY_OP_LTE) res = (cmp <= 0);
                        else if (inst.op == LUBY_OP_GT) res = (cmp > 0);
                        else res = (cmp >= 0);
                    } else {
                        double af = (a.type == LUBY_T_FLOAT) ? a.as.f : (double)a.as.i;
                        double bf = (b.type == LUBY_T_FLOAT) ? b.as.f : (double)b.as.i;
                        if (inst.op == LUBY_OP_LT) res = (af < bf);
                        else if (inst.op == LUBY_OP_LTE) res = (af <= bf);
                        else if (inst.op == LUBY_OP_GT) res = (af > bf);
                        else res = (af >= bf);
                    }
                    vm->stack[vm->sp++] = luby_bool(res);
                    break;
                }
                case LUBY_OP_MAKE_ARRAY: {
                    uint8_t count = inst.a;
                    luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
                    if (!arr) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    arr->count = count;
                    arr->capacity = count;
                    arr->items = (luby_value *)luby_alloc_raw(L, NULL, count * sizeof(luby_value));
                    arr->frozen = 0;
                    if (!arr->items && count > 0) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    for (int i = (int)count - 1; i >= 0; i--) {
                        arr->items[i] = (vm->sp > f->stack_base) ? vm->stack[--vm->sp] : luby_nil();
                    }
                    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = arr;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = v;
                    break;
                }
                case LUBY_OP_MAKE_HASH: {
                    uint8_t count = inst.a;
                    luby_hash *h = (luby_hash *)luby_gc_alloc(L, sizeof(luby_hash), LUBY_GC_HASH);
                    if (!h) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    h->count = count;
                    h->capacity = count;
                    h->entries = (luby_hash_entry *)luby_alloc_raw(L, NULL, count * sizeof(luby_hash_entry));
                    h->frozen = 0;
                    if (!h->entries && count > 0) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    for (int i = (int)count - 1; i >= 0; i--) {
                        luby_value val = (vm->sp > f->stack_base) ? vm->stack[--vm->sp] : luby_nil();
                        luby_value key = (vm->sp > f->stack_base) ? vm->stack[--vm->sp] : luby_nil();
                        h->entries[i].key = key;
                        h->entries[i].value = val;
                    }
                    luby_value v; v.type = LUBY_T_HASH; v.as.ptr = h;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = v;
                    break;
                }
                case LUBY_OP_SAFE_INDEX:
                case LUBY_OP_GET_INDEX: {
                    if (vm->sp - f->stack_base < 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value index = vm->stack[--vm->sp];
                    luby_value target = vm->stack[--vm->sp];
                    luby_value r = luby_nil();
                    if (inst.op == LUBY_OP_SAFE_INDEX && target.type == LUBY_T_NIL) {
                        vm->stack[vm->sp++] = luby_nil();
                        break;
                    }
                    if (target.type == LUBY_T_ARRAY && target.as.ptr && index.type == LUBY_T_INT) {
                        luby_array *arr = (luby_array *)target.as.ptr;
                        int64_t idx = index.as.i;
                        if (idx >= 0 && (size_t)idx < arr->count) r = arr->items[idx];
                    } else if ((target.type == LUBY_T_STRING || target.type == LUBY_T_SYMBOL) && target.as.ptr && index.type == LUBY_T_INT) {
                        const char *str = (const char *)target.as.ptr;
                        size_t slen = strlen(str);
                        int64_t idx = index.as.i;
                        if (idx >= 0 && (size_t)idx < slen) {
                            r = luby_string(L, str + idx, 1);
                        }
                    } else if (target.type == LUBY_T_HASH && target.as.ptr) {
                        luby_hash *h = (luby_hash *)target.as.ptr;
                        for (size_t i = 0; i < h->count; i++) {
                            if (luby_value_eq(h->entries[i].key, index)) { r = h->entries[i].value; break; }
                        }
                    }
                    vm->stack[vm->sp++] = r;
                    break;
                }
                case LUBY_OP_SET_INDEX: {
                    if (vm->sp - f->stack_base < 3) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value value = vm->stack[--vm->sp];
                    luby_value index = vm->stack[--vm->sp];
                    luby_value target = vm->stack[--vm->sp];
                    if (target.type == LUBY_T_ARRAY && target.as.ptr && index.type == LUBY_T_INT) {
                        luby_array *arr = (luby_array *)target.as.ptr;
                        if (arr->frozen) { luby_set_error(L, LUBY_E_RUNTIME, "frozen", f->filename, line, 0); goto vm_error; }
                        int64_t idx = index.as.i;
                        if (idx >= 0) {
                            if ((size_t)idx >= arr->capacity) {
                                size_t new_cap = arr->capacity < 8 ? 8 : arr->capacity;
                                while ((size_t)idx >= new_cap) new_cap *= 2;
                                luby_value *ni = (luby_value *)luby_alloc_raw(L, arr->items, new_cap * sizeof(luby_value));
                                if (!ni) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                                for (size_t i = arr->capacity; i < new_cap; i++) ni[i] = luby_nil();
                                arr->items = ni;
                                arr->capacity = new_cap;
                            }
                            if ((size_t)idx >= arr->count) arr->count = (size_t)idx + 1;
                            arr->items[idx] = value;
                        }
                    } else if (target.type == LUBY_T_HASH && target.as.ptr) {
                        luby_hash *h = (luby_hash *)target.as.ptr;
                        if (h->frozen) { luby_set_error(L, LUBY_E_RUNTIME, "frozen", f->filename, line, 0); goto vm_error; }
                        for (size_t i = 0; i < h->count; i++) {
                            if (luby_value_eq(h->entries[i].key, index)) {
                                h->entries[i].value = value;
                                vm->stack[vm->sp++] = value;
                                goto set_index_done;
                            }
                        }
                        if (h->count + 1 > h->capacity) {
                            size_t new_cap = h->capacity < 8 ? 8 : h->capacity * 2;
                            luby_hash_entry *ne = (luby_hash_entry *)luby_alloc_raw(L, h->entries, new_cap * sizeof(luby_hash_entry));
                            if (!ne) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                            h->entries = ne;
                            h->capacity = new_cap;
                        }
                        h->entries[h->count].key = index;
                        h->entries[h->count].value = value;
                        h->count++;
                    }
                    vm->stack[vm->sp++] = value;
set_index_done:
                    break;
                }
                case LUBY_OP_SAFE_CALL:
                case LUBY_OP_CALL: {
                    int argc = inst.a;
                    if (vm->sp - f->stack_base < argc) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value sym = chunk->consts[inst.c];
                    const char *fname = (const char *)sym.as.ptr;
                    luby_cfunc fn = luby_find_cfunc(L, fname);
                    luby_value r = luby_nil();
                    luby_value args[16];
                    int use = argc > 16 ? 16 : argc;
                    for (int i = 0; i < use; i++) {
                        args[use - 1 - i] = vm->stack[--vm->sp];
                    }
                    if (argc > use) {
                        for (int i = use; i < argc; i++) (void)vm->stack[--vm->sp];
                    }

                    if (inst.op == LUBY_OP_SAFE_CALL) {
                        if (use > 0 && args[0].type == LUBY_T_NIL) {
                            L->current_block = luby_nil();
                            vm->stack[vm->sp++] = luby_nil();
                            break;
                        }
                    }

                    // super with implicit receiver
                    if (fname && strcmp(fname, "super") == 0 && L->current_method_class && L->current_method_class->super) {
                        const char *mname = L->current_method_name ? L->current_method_name : "";
                        luby_proc *sm = luby_class_get_method_from(L, L->current_method_class->super, mname);
                        if (!sm) {
                            luby_set_error(L, LUBY_E_NAME, "undefined super", f->filename, line, 0);
                            goto vm_error;
                        }
                        luby_value block = L->current_block;
                        L->current_block = luby_nil();
                        f->ip++;
                        if (!luby_vm_push_frame(L, vm, sm, &sm->chunk, "<super>", L->current_self, L->current_method_class->super, mname, use, args, block, 1)) {
                            luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                            goto vm_error;
                        }
                        goto vm_next_frame;
                    }

                    // Method call on receiver if present
                    if (use > 0) {
                        luby_value recv = args[0];
                        // Handle proc.call(args)
                        if (recv.type == LUBY_T_PROC && fname && strcmp(fname, "call") == 0) {
                            luby_proc *proc = (luby_proc *)recv.as.ptr;
                            if (!proc) { luby_set_error(L, LUBY_E_TYPE, "nil proc", f->filename, line, 0); goto vm_error; }
                            luby_value block = L->current_block;
                            L->current_block = luby_nil();
                            f->ip++;
                            if (!luby_vm_push_frame(L, vm, proc, &proc->chunk, "<proc>", luby_nil(), NULL, NULL, use - 1, args + 1, block, 0)) {
                                luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                goto vm_error;
                            }
                            goto vm_next_frame;
                        }
                        if ((recv.type == LUBY_T_OBJECT || recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) && fname) {
                            luby_class_obj *cls = NULL;
                            if (recv.type == LUBY_T_OBJECT) cls = ((luby_object *)recv.as.ptr)->klass;
                            else cls = (luby_class_obj *)recv.as.ptr;

                            if (cls && strcmp(fname, "new") == 0 && recv.type == LUBY_T_CLASS) {
                                luby_object *obj = luby_object_new(L, cls);
                                if (!obj) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                                r.type = LUBY_T_OBJECT; r.as.ptr = obj;
                                // Check for initialize method and call it
                                luby_proc *init = luby_class_get_method(L, cls, "initialize");
                                if (init) {
                                    luby_value block = L->current_block;
                                    L->current_block = luby_nil();
                                    f->ip++;
                                    // Push the new object as return value first - it will be replaced by frame pop
                                    // Actually: we need to return obj, not the result of initialize
                                    // So we push obj onto stack, then set up frame to call init
                                    // When init returns, it pops its return value, but we've already pushed obj
                                    // This is tricky - we need a different approach
                                    // Save obj in a temp, call init, then push obj
                                    // For now, push frame for initialize with obj as self
                                    if (!luby_vm_push_frame(L, vm, init, &init->chunk, "<initialize>", r, cls, "initialize", use - 1, args + 1, block, 1)) {
                                        luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                        goto vm_error;
                                    }
                                    // Mark that we should return obj, not init's return value
                                    vm->frames[vm->frame_count - 1].return_override = r;
                                    vm->frames[vm->frame_count - 1].has_return_override = 1;
                                    goto vm_next_frame;
                                }
                                L->current_block = luby_nil();
                                vm->stack[vm->sp++] = r;
                                break;
                            } else if (cls) {
                                luby_value method_val = luby_nil();
                                if (recv.type == LUBY_T_OBJECT && recv.as.ptr) {
                                    luby_proc *sp = luby_object_get_singleton_method((luby_object *)recv.as.ptr, fname);
                                    if (sp) { method_val.type = LUBY_T_PROC; method_val.as.ptr = sp; }
                                } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
                                    luby_proc *sp = luby_class_get_singleton_method(L, (luby_class_obj *)recv.as.ptr, fname);
                                    if (sp) { method_val.type = LUBY_T_PROC; method_val.as.ptr = sp; }
                                }
                                if (method_val.type == LUBY_T_NIL) method_val = luby_class_lookup_method(L, cls, fname);
                                if (method_val.type == LUBY_T_PROC) {
                                    luby_proc *m = (luby_proc *)method_val.as.ptr;
                                    luby_value block = L->current_block;
                                    L->current_block = luby_nil();
                                    f->ip++;
                                    if (!luby_vm_push_frame(L, vm, m, &m->chunk, "<method>", recv, cls, fname, use - 1, args + 1, block, 1)) {
                                        luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                        goto vm_error;
                                    }
                                    goto vm_next_frame;
                                } else if (method_val.type == LUBY_T_CMETHOD) {
                                    luby_cmethod *cm = (luby_cmethod *)method_val.as.ptr;
                                    L->current_block = luby_nil();
                                    if (cm->fn(L, use, args, &r) != 0) {
                                        if (L->last_error.code == LUBY_E_OK) {
                                            luby_set_error(L, LUBY_E_RUNTIME, "native method failed", f->filename, line, 0);
                                        }
                                        goto vm_error;
                                    }
                                    if (vm->native_yield) {
                                        vm->native_yield = 0;
                                        f->ip++;
                                        if (out) *out = vm->yield_value;
                                        L->current_vm = saved_vm;
                                        return (int)LUBY_E_OK;
                                    }
                                    vm->stack[vm->sp++] = r;
                                    break;
                                } else if (strcmp(fname, "super") == 0) {
                                    luby_class_obj *start = L->current_method_class ? L->current_method_class->super : NULL;
                                    const char *mname = L->current_method_name ? L->current_method_name : "";
                                    luby_proc *sm = luby_class_get_method_from(L, start, mname);
                                    if (sm) {
                                        luby_value block = L->current_block;
                                        L->current_block = luby_nil();
                                        f->ip++;
                                        if (!luby_vm_push_frame(L, vm, sm, &sm->chunk, "<super>", L->current_self, start, mname, use, args, block, 1)) {
                                            luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                            goto vm_error;
                                        }
                                        goto vm_next_frame;
                                    } else {
                                        luby_set_error(L, LUBY_E_NAME, "undefined super", f->filename, line, 0);
                                        goto vm_error;
                                    }
                                } else {
                                    // method_missing fallback
                                    luby_proc *mm = luby_class_get_method(L, cls, "method_missing");
                                    if (mm) {
                                        luby_value mm_args[16];
                                        int mcount = use + 1;
                                        if (mcount > 16) mcount = 16;
                                        mm_args[0] = recv;
                                        luby_value namev; namev.type = LUBY_T_SYMBOL; namev.as.ptr = (void *)fname;
                                        if (mcount > 1) mm_args[1] = namev;
                                        for (int i = 2; i < mcount; i++) mm_args[i] = args[i - 1];
                                        luby_value block = L->current_block;
                                        L->current_block = luby_nil();
                                        f->ip++;
                                        if (!luby_vm_push_frame(L, vm, mm, &mm->chunk, "<method_missing>", recv, cls, "method_missing", mcount - 1, mm_args + 1, block, 1)) {
                                            luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                            goto vm_error;
                                        }
                                        goto vm_next_frame;
                                    } else if (fn) {
                                        if (fn(L, use, args, &r) != 0) {
                                            if (L->last_error.code == LUBY_E_OK) {
                                                luby_set_error(L, LUBY_E_RUNTIME, "native call failed", f->filename, line, 0);
                                            }
                                            goto vm_error;
                                        }
                                        if (vm->native_yield) {
                                            vm->native_yield = 0;
                                            f->ip++;
                                            if (out) *out = vm->yield_value;
                                            L->current_vm = saved_vm;
                                            return (int)LUBY_E_OK;
                                        }
                                        L->current_block = luby_nil();
                                        vm->stack[vm->sp++] = r;
                                        break;
                                    } else {
                                        luby_set_error(L, LUBY_E_NAME, "undefined method", f->filename, line, 0);
                                        goto vm_error;
                                    }
                                }
                            }
                            L->current_block = luby_nil();
                            vm->stack[vm->sp++] = r;
                            break;
                        }
                    }

                    /* Check user-defined procs FIRST so they can shadow builtins */
                    {
                        luby_string_view sv = { fname, fname ? strlen(fname) : 0 };
                        luby_value gv = luby_get_global(L, sv);
                        if (gv.type == LUBY_T_PROC && gv.as.ptr) {
                            luby_proc *gp = (luby_proc *)gv.as.ptr;
                            luby_value block = L->current_block;
                            L->current_block = luby_nil();
                            f->ip++;
                            if (!luby_vm_push_frame(L, vm, gp, &gp->chunk, "<proc>", luby_nil(), NULL, NULL, use, args, block, 0)) {
                                luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                                goto vm_error;
                            }
                            goto vm_next_frame;
                        }
                    }
                    if (!fn) {
                        luby_set_error(L, LUBY_E_NAME, "undefined function", f->filename, line, 0);
                        goto vm_error;
                    } else {
                        if (fn(L, use, args, &r) != 0) {
                            if (L->last_error.code == LUBY_E_OK) {
                                luby_set_error(L, LUBY_E_RUNTIME, "native call failed", f->filename, line, 0);
                            }
                            goto vm_error;
                        }
                        if (vm->native_yield) {
                            vm->native_yield = 0;
                            f->ip++;
                            if (out) *out = vm->yield_value;
                            L->current_vm = saved_vm;
                            return (int)LUBY_E_OK;
                        }
                    }
                    L->current_block = luby_nil();
                    vm->stack[vm->sp++] = r;
                    break;
                }
                case LUBY_OP_YIELD: {
                    int argc = inst.a;
                    if (vm->sp - f->stack_base < argc) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value yargs[16];
                    int use = argc > 16 ? 16 : argc;
                    for (int i = use - 1; i >= 0; i--) {
                        yargs[i] = vm->stack[--vm->sp];
                    }
                    for (int i = use; i < argc; i++) (void)vm->stack[--vm->sp];

                    luby_value yv = luby_nil();
                    if (use == 1) {
                        yv = yargs[0];
                    } else if (use > 1) {
                        luby_value arrv = luby_array_new(L);
                        for (int i = 0; i < use; i++) {
                            luby_array_set(L, arrv, (size_t)i, yargs[i]);
                        }
                        yv = arrv;
                    }

                    if (L->current_coroutine) {
                        vm->yielded = 1;
                        vm->yield_value = yv;
                        f->ip++;
                        if (out) *out = yv;
                        L->current_vm = saved_vm;
                        return (int)LUBY_E_OK;
                    }

                    if (L->current_block.type != LUBY_T_PROC) {
                        if (out) *out = luby_nil();
                        luby_set_error(L, LUBY_E_RUNTIME, "no block given", f->filename, line, 0);
                        goto vm_error;
                    }
                    luby_proc *bp = (luby_proc *)L->current_block.as.ptr;
                    luby_value block = luby_nil();
                    L->current_block = luby_nil();
                    f->ip++;
                    if (!luby_vm_push_frame(L, vm, bp, &bp->chunk, "<block>", luby_nil(), NULL, NULL, use, yargs, block, 0)) {
                        luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0);
                        goto vm_error;
                    }
                    goto vm_next_frame;
                }
                case LUBY_OP_CONCAT: {
                    // Concatenate 'a' values from stack into a single string
                    int count = inst.a;
                    if (vm->sp - f->stack_base < count) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }

                    // Calculate total length
                    size_t total_len = 0;
                    char **parts = NULL;
                    size_t *lens = NULL;
                    if (count > 0) {
                        parts = (char **)luby_alloc_raw(L, NULL, count * sizeof(char *));
                        lens = (size_t *)luby_alloc_raw(L, NULL, count * sizeof(size_t));
                        if (!parts || !lens) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }

                        // Pop values and convert to strings (in reverse order since stack)
                        for (int i = count - 1; i >= 0; i--) {
                            luby_value v = vm->stack[--vm->sp];
                            parts[i] = luby_value_to_string(L, v);
                            if (!parts[i]) parts[i] = luby_dup_string(L, "", 0);
                            lens[i] = strlen(parts[i]);
                            total_len += lens[i];
                        }
                    }

                    // Allocate result string (GC-tracked)
                    char *result = luby_gc_alloc_string(L, NULL, total_len);
                    if (!result) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }

                    // Concatenate
                    char *p = result;
                    for (int i = 0; i < count; i++) {
                        memcpy(p, parts[i], lens[i]);
                        p += lens[i];
                        luby_alloc_raw(L, parts[i], 0); // free temporary part
                    }
                    *p = '\0';
                    if (parts) luby_alloc_raw(L, parts, 0);
                    if (lens) luby_alloc_raw(L, lens, 0);

                    // Push result
                    luby_value rv;
                    rv.type = LUBY_T_STRING;
                    rv.as.ptr = result;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = rv;
                    break;
                }
                case LUBY_OP_JUMP:
                    f->ip = inst.c;
                    continue;
                case LUBY_OP_JUMP_IF_FALSE: {
                    if (vm->sp - f->stack_base < 1) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value cond = vm->stack[--vm->sp];
                    if (!luby_is_truthy(cond)) {
                        f->ip = inst.c;
                        continue;
                    }
                    break;
                }
                case LUBY_OP_TRY:
                    if (f->hcount >= 16) { luby_set_error(L, LUBY_E_RUNTIME, "handler stack overflow", f->filename, line, 0); goto vm_error; }
                    f->handlers[f->hcount].rescue_ip = inst.c;
                    f->handlers[f->hcount].ensure_ip = LUBY_IP_NONE;
                    f->handlers[f->hcount].pending_error = 0;
                    f->handlers[f->hcount].phase = 0;
                    f->handlers[f->hcount].sp = vm->sp;
                    f->hcount++;
                    break;
                case LUBY_OP_SET_ENSURE:
                    if (f->hcount > 0) f->handlers[f->hcount - 1].ensure_ip = inst.c;
                    break;
                case LUBY_OP_ENTER_ENSURE:
                    if (f->hcount > 0) f->handlers[f->hcount - 1].phase = 2;
                    break;
                case LUBY_OP_END_TRY:
                    if (f->hcount > 0) {
                        luby_vm_handler h = f->handlers[--f->hcount];
                        if (h.pending_error) {
                            L->last_error = h.pending;
                            goto vm_error;
                        }
                    }
                    break;
                case LUBY_OP_THROW: {
                    if (vm->sp - f->stack_base < 1) { luby_set_error(L, LUBY_E_RUNTIME, "raise without value", f->filename, line, 0); goto vm_error; }
                    luby_value msgv = vm->stack[--vm->sp];
                    const char *msg = "raise";
                    if (msgv.type == LUBY_T_STRING || msgv.type == LUBY_T_SYMBOL) {
                        msg = (const char *)msgv.as.ptr;
                    } else if (msgv.type == LUBY_T_NIL) {
                        msg = "raise";
                    } else {
                        msg = "runtime error";
                    }
                    luby_set_error(L, LUBY_E_RUNTIME, msg, f->filename, line, 0);
                    goto vm_error;
                }
                case LUBY_OP_RET: {
                    luby_value result = (vm->sp > f->stack_base) ? vm->stack[--vm->sp] : luby_nil();
                    luby_vm_pop_frame(L, vm, result, 1);
                    goto vm_next_frame;
                }
                case LUBY_OP_GET_IVAR: {
                    // Get instance variable from self
                    luby_value self_val = L->current_self;
                    if (self_val.type != LUBY_T_OBJECT || !self_val.as.ptr) {
                        if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                        vm->stack[vm->sp++] = luby_nil();
                        break;
                    }
                    luby_object *obj = (luby_object *)self_val.as.ptr;
                    luby_value namev = chunk->consts[inst.c];
                    const char *name = (namev.type == LUBY_T_SYMBOL || namev.type == LUBY_T_STRING) ? (const char *)namev.as.ptr : "";
                    // Search for ivar
                    luby_value result = luby_nil();
                    for (size_t i = 0; i < obj->ivar_count; i++) {
                        if (strcmp(obj->ivar_names[i], name) == 0) {
                            result = obj->ivar_values[i];
                            break;
                        }
                    }
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = result;
                    break;
                }
                case LUBY_OP_SET_IVAR: {
                    // Set instance variable on self
                    luby_value self_val = L->current_self;
                    if (self_val.type != LUBY_T_OBJECT || !self_val.as.ptr) {
                        luby_set_error(L, LUBY_E_RUNTIME, "no self for ivar", f->filename, line, 0);
                        goto vm_error;
                    }
                    if (vm->sp - f->stack_base < 1) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value val = vm->stack[vm->sp - 1]; // keep on stack for result
                    luby_object *obj = (luby_object *)self_val.as.ptr;
                    luby_value namev = chunk->consts[inst.c];
                    const char *name = (namev.type == LUBY_T_SYMBOL || namev.type == LUBY_T_STRING) ? (const char *)namev.as.ptr : "";
                    // Search for existing ivar
                    int found = 0;
                    for (size_t i = 0; i < obj->ivar_count; i++) {
                        if (strcmp(obj->ivar_names[i], name) == 0) {
                            obj->ivar_values[i] = val;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        // Add new ivar
                        size_t n = obj->ivar_count;
                        obj->ivar_names = (char **)luby_alloc_raw(L, obj->ivar_names, (n + 1) * sizeof(char *));
                        obj->ivar_values = (luby_value *)luby_alloc_raw(L, obj->ivar_values, (n + 1) * sizeof(luby_value));
                        if (!obj->ivar_names || !obj->ivar_values) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                        obj->ivar_names[n] = luby_dup_string(L, name, strlen(name));
                        obj->ivar_values[n] = val;
                        obj->ivar_count = n + 1;
                    }
                    break;
                }
                case LUBY_OP_MAKE_RANGE: {
                    // Create a range object from stack [start, end]
                    if (vm->sp - f->stack_base < 2) { luby_set_error(L, LUBY_E_RUNTIME, "stack underflow", f->filename, line, 0); goto vm_error; }
                    luby_value end_val = vm->stack[--vm->sp];
                    luby_value start_val = vm->stack[--vm->sp];
                    int exclusive = inst.a;
                    // Create range object
                    luby_range *range = (luby_range *)luby_gc_alloc(L, sizeof(luby_range), LUBY_GC_RANGE);
                    if (!range) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    range->start = start_val;
                    range->end = end_val;
                    range->exclusive = exclusive;
                    luby_value rv;
                    rv.type = LUBY_T_RANGE;
                    rv.as.ptr = range;
                    if (!luby_vm_ensure_stack(L, vm, 1)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                    vm->stack[vm->sp++] = rv;
                    break;
                }
                case LUBY_OP_MULTI_UNPACK: {
                    // Unpack values for multiple assignment
                    // a = target_count, b = value_count
                    // Stack has value_count values, we need target_count values
                    size_t target_count = inst.a;
                    size_t value_count = inst.b;
                    // If single array value and multiple targets, destructure it
                    if (value_count == 1 && target_count > 1) {
                        luby_value single = vm->stack[vm->sp - 1];
                        if (single.type == LUBY_T_ARRAY && single.as.ptr) {
                            luby_array *arr = (luby_array *)single.as.ptr;
                            vm->sp--; // pop the array
                            // Push array elements in reverse order (so first element ends up first to pop)
                            if (!luby_vm_ensure_stack(L, vm, target_count)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                            for (size_t i = 0; i < target_count; i++) {
                                if (i < arr->count) {
                                    vm->stack[vm->sp++] = arr->items[i];
                                } else {
                                    vm->stack[vm->sp++] = luby_nil();
                                }
                            }
                            break;
                        }
                    }
                    // Otherwise, ensure we have enough values (pad with nil if needed)
                    if (value_count < target_count) {
                        if (!luby_vm_ensure_stack(L, vm, target_count - value_count)) { luby_set_error(L, LUBY_E_OOM, "oom", f->filename, line, 0); goto vm_error; }
                        for (size_t i = value_count; i < target_count; i++) {
                            vm->stack[vm->sp++] = luby_nil();
                        }
                    } else if (value_count > target_count) {
                        // Pop extra values
                        vm->sp -= (value_count - target_count);
                    }
                    break;
                }
                default:
                    break;
            }

            f->ip++;
            continue;

vm_next_frame:
            switched = 1;
            break;
        }

        if (vm->frame_count == 0) break;
        if (switched) continue;
        if (f->ip >= chunk->count) {
            luby_value result = (vm->sp > f->stack_base) ? vm->stack[vm->sp - 1] : luby_nil();
            luby_vm_pop_frame(L, vm, result, 1);
        }
    }

    L->current_vm = saved_vm;
    if (out) {
        if (vm->sp > 0) {
            *out = vm->stack[vm->sp - 1];
        } else {
            *out = luby_nil();
        }
    }
    return (int)L->last_error.code;

vm_error:
    if (luby_vm_handle_error(L, f->handlers, &f->hcount, &f->ip, &vm->sp)) {
        goto vm_continue;
    }
    L->current_vm = saved_vm;
    if (out) *out = luby_nil();
    return (int)L->last_error.code;
}

static luby_op luby_binary_op_from_token(luby_token_kind kind) {
    switch (kind) {
        case LUBY_TOK_PLUS: return LUBY_OP_ADD;
        case LUBY_TOK_MINUS: return LUBY_OP_SUB;
        case LUBY_TOK_STAR: return LUBY_OP_MUL;
        case LUBY_TOK_SLASH: return LUBY_OP_DIV;
        case LUBY_TOK_PERCENT: return LUBY_OP_MOD;
        case LUBY_TOK_AND:
        case LUBY_TOK_ANDAND: return LUBY_OP_AND;
        case LUBY_TOK_OR:
        case LUBY_TOK_OROR: return LUBY_OP_OR;
        case LUBY_TOK_EQEQ: return LUBY_OP_EQ;
        case LUBY_TOK_LT: return LUBY_OP_LT;
        case LUBY_TOK_LTE: return LUBY_OP_LTE;
        case LUBY_TOK_GT: return LUBY_OP_GT;
        case LUBY_TOK_GTE: return LUBY_OP_GTE;
        default: return LUBY_OP_NOOP;
    }
}

static int luby_compile_node(luby_compiler *C, luby_ast_node *node);

static int luby_compile_block(luby_compiler *C, luby_ast_node *block) {
    for (size_t i = 0; i < block->as.list.count; i++) {
        luby_ast_node *stmt = block->as.list.items[i];
        if (!luby_compile_node(C, stmt)) return 0;
        if (i + 1 < block->as.list.count) {
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, stmt->line);
        }
    }
    return 1;
}

static int luby_compile_literal(luby_compiler *C, luby_ast_node *node) {
    luby_value v = luby_nil();
    switch (node->kind) {
        case LUBY_AST_INT:
            {
                char *tmp = luby_dup_string(C->L, node->as.literal.data, node->as.literal.length);
                if (!tmp) return 0;
                v = luby_int(strtoll(tmp, NULL, 10));
                luby_alloc_raw(C->L, tmp, 0);
            }
            break;
        case LUBY_AST_FLOAT:
            {
                char *tmp = luby_dup_string(C->L, node->as.literal.data, node->as.literal.length);
                if (!tmp) return 0;
                v = luby_float(strtod(tmp, NULL));
                luby_alloc_raw(C->L, tmp, 0);
            }
            break;
        case LUBY_AST_BOOL:
            v = luby_bool(node->as.literal.length == 4); // "true" or "false"
            break;
        case LUBY_AST_NIL:
            v = luby_nil();
            break;
        case LUBY_AST_STRING:
        case LUBY_AST_SYMBOL: {
            size_t start = 0;
            size_t end = node->as.literal.length;
            const char *s = node->as.literal.data;
            if (node->kind == LUBY_AST_SYMBOL && s[0] == ':') {
                start = 1;
            }
            if (end > start && (s[start] == '"' || s[start] == '\'')) {
                start++;
                if (end > start && (s[end - 1] == '"' || s[end - 1] == '\'')) {
                    end--;
                }
            }
            size_t len = end > start ? end - start : 0;
            if (node->kind == LUBY_AST_SYMBOL) {
                v = luby_symbol(C->L, s + start, len);
                if (!v.as.ptr) return 0;
            } else {
                char *buf = luby_gc_alloc_string(C->L, s + start, len);
                if (!buf) return 0;
                v.type = LUBY_T_STRING;
                v.as.ptr = buf;
            }
            break;
        }
        default:
            v = luby_nil();
            break;
    }
    uint32_t idx = luby_chunk_add_const(C->L, C->chunk, v);
    luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONST, 0, 0, idx, node->line);
    return 1;
}


static luby_proc *luby_compile_block_proc(luby_compiler *C, luby_ast_node *lambda) {
    if (!lambda || lambda->kind != LUBY_AST_LAMBDA) return NULL;
    luby_proc *proc = (luby_proc *)luby_gc_alloc(C->L, sizeof(luby_proc), LUBY_GC_PROC);
    if (!proc) return NULL;
    proc->owned_by_chunk = 1;
    proc->splat_index = -1;

    if (lambda->as.lambda.param_count > 0) {
        proc->param_names = (char **)luby_alloc_raw(C->L, NULL, lambda->as.lambda.param_count * sizeof(char *));
        proc->default_chunks = (luby_chunk *)luby_alloc_raw(C->L, NULL, lambda->as.lambda.param_count * sizeof(luby_chunk));
        if (!proc->param_names || !proc->default_chunks) return NULL;
        proc->param_count = lambda->as.lambda.param_count;
        
        for (size_t i = 0; i < proc->param_count; i++) {
            luby_ast_node *param = lambda->as.lambda.params[i];
            luby_chunk_init(&proc->default_chunks[i]);
            
            if (param->kind == LUBY_AST_DEFAULT_PARAM) {
                proc->param_names[i] = luby_dup_string(C->L, param->as.assign.target->as.literal.data, param->as.assign.target->as.literal.length);
                // Compile default value into its own chunk
                luby_compiler dsub;
                dsub.L = C->L;
                dsub.chunk = &proc->default_chunks[i];
                dsub.class_depth = 0;
                dsub.loop_depth = 0;
                luby_compile_node(&dsub, param->as.assign.value);
            } else if (param->kind == LUBY_AST_SPLAT_PARAM) {
                proc->param_names[i] = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
                proc->splat_index = (int)i;
            } else if (param->kind == LUBY_AST_BLOCK_PARAM) {
                proc->block_param_name = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
                proc->has_block_param = 1;
                proc->param_count--; // Don't count block param in regular params
            } else {
                proc->param_names[i] = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
            }
        }
    }

    luby_chunk_init(&proc->chunk);
    luby_compiler sub;
    sub.L = C->L;
    sub.chunk = &proc->chunk;
    sub.class_depth = 0;
    sub.loop_depth = 0;
    if (!luby_compile_node(&sub, lambda->as.lambda.body)) return NULL;
    
    // Blocks and lambdas are always public
    proc->visibility = LUBY_VIS_PUBLIC;
    
    return proc;
}

/* Walk AST to collect variable names assigned in function body (excluding params). */
static void luby_collect_locals(luby_state *L, luby_ast_node *node,
                                char ***names, size_t *count,
                                char **param_names, size_t param_count) {
    if (!node) return;
    switch (node->kind) {
    case LUBY_AST_ASSIGN: {
        luby_ast_node *tgt = node->as.assign.target;
        if (tgt && tgt->kind == LUBY_AST_IDENT) {
            const char *nm = tgt->as.literal.data;
            size_t nl = tgt->as.literal.length;
            /* Skip if it's a param name */
            int is_param = 0;
            for (size_t i = 0; i < param_count; i++) {
                if (param_names[i] && strlen(param_names[i]) == nl &&
                    memcmp(param_names[i], nm, nl) == 0) {
                    is_param = 1; break;
                }
            }
            if (!is_param) {
                /* Skip if already collected */
                int dup = 0;
                for (size_t i = 0; i < *count; i++) {
                    if (strlen((*names)[i]) == nl &&
                        memcmp((*names)[i], nm, nl) == 0) {
                        dup = 1; break;
                    }
                }
                if (!dup) {
                    *names = (char **)luby_alloc_raw(L, *names, (*count + 1) * sizeof(char *));
                    (*names)[*count] = luby_dup_string(L, nm, nl);
                    (*count)++;
                }
            }
        }
        luby_collect_locals(L, node->as.assign.value, names, count, param_names, param_count);
        break;
    }
    case LUBY_AST_MULTI_ASSIGN: {
        for (size_t i = 0; i < node->as.multi_assign.target_count; i++) {
            luby_ast_node *tgt = node->as.multi_assign.targets[i];
            if (tgt && tgt->kind == LUBY_AST_IDENT) {
                const char *nm = tgt->as.literal.data;
                size_t nl = tgt->as.literal.length;
                int is_param = 0;
                for (size_t j = 0; j < param_count; j++) {
                    if (param_names[j] && strlen(param_names[j]) == nl &&
                        memcmp(param_names[j], nm, nl) == 0) {
                        is_param = 1; break;
                    }
                }
                if (!is_param) {
                    int dup = 0;
                    for (size_t j = 0; j < *count; j++) {
                        if (strlen((*names)[j]) == nl &&
                            memcmp((*names)[j], nm, nl) == 0) {
                            dup = 1; break;
                        }
                    }
                    if (!dup) {
                        *names = (char **)luby_alloc_raw(L, *names, (*count + 1) * sizeof(char *));
                        (*names)[*count] = luby_dup_string(L, nm, nl);
                        (*count)++;
                    }
                }
            }
        }
        for (size_t i = 0; i < node->as.multi_assign.value_count; i++)
            luby_collect_locals(L, node->as.multi_assign.values[i], names, count, param_names, param_count);
        break;
    }
    /* Don't descend into nested def/class/module — those have their own scope */
    case LUBY_AST_DEF:
    case LUBY_AST_CLASS:
    case LUBY_AST_MODULE:
        break;
    /* Recurse into statement-like nodes */
    case LUBY_AST_IF:
    case LUBY_AST_TERNARY:
        luby_collect_locals(L, node->as.if_stmt.cond, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.if_stmt.then_branch, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.if_stmt.else_branch, names, count, param_names, param_count);
        break;
    case LUBY_AST_WHILE:
        luby_collect_locals(L, node->as.while_stmt.cond, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.while_stmt.body, names, count, param_names, param_count);
        break;
    case LUBY_AST_RETURN:
        luby_collect_locals(L, node->as.ret.value, names, count, param_names, param_count);
        break;
    case LUBY_AST_BEGIN:
        luby_collect_locals(L, node->as.begin.body, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.begin.rescue_body, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.begin.ensure_body, names, count, param_names, param_count);
        break;
    case LUBY_AST_BINARY:
        luby_collect_locals(L, node->as.binary.left, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.binary.right, names, count, param_names, param_count);
        break;
    case LUBY_AST_UNARY:
        luby_collect_locals(L, node->as.unary.expr, names, count, param_names, param_count);
        break;
    case LUBY_AST_CALL:
        luby_collect_locals(L, node->as.call.recv, names, count, param_names, param_count);
        for (size_t i = 0; i < node->as.call.argc; i++)
            luby_collect_locals(L, node->as.call.args[i], names, count, param_names, param_count);
        luby_collect_locals(L, node->as.call.block, names, count, param_names, param_count);
        break;
    case LUBY_AST_INDEX:
        luby_collect_locals(L, node->as.index.target, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.index.index, names, count, param_names, param_count);
        break;
    case LUBY_AST_INDEX_ASSIGN:
        luby_collect_locals(L, node->as.index_assign.target, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.index_assign.index, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.index_assign.value, names, count, param_names, param_count);
        break;
    case LUBY_AST_BLOCK:
        for (size_t i = 0; i < node->as.list.count; i++)
            luby_collect_locals(L, node->as.list.items[i], names, count, param_names, param_count);
        break;
    case LUBY_AST_LAMBDA:
        /* Descend into lambda body so their assigns are captured as locals of the enclosing function */
        luby_collect_locals(L, node->as.lambda.body, names, count, param_names, param_count);
        break;
    case LUBY_AST_RANGE:
        luby_collect_locals(L, node->as.range.start, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.range.end, names, count, param_names, param_count);
        break;
    case LUBY_AST_PAIR:
        luby_collect_locals(L, node->as.pair.left, names, count, param_names, param_count);
        luby_collect_locals(L, node->as.pair.right, names, count, param_names, param_count);
        break;
    case LUBY_AST_INTERP_STRING:
    case LUBY_AST_ARRAY:
    case LUBY_AST_HASH:
        for (size_t i = 0; i < node->as.list.count; i++)
            luby_collect_locals(L, node->as.list.items[i], names, count, param_names, param_count);
        break;
    default:
        break;
    }
}

static luby_proc *luby_compile_def_proc(luby_compiler *C, luby_ast_node *defn) {
    if (!defn || defn->kind != LUBY_AST_DEF) return NULL;
    luby_proc *proc = (luby_proc *)luby_gc_alloc(C->L, sizeof(luby_proc), LUBY_GC_PROC);
    if (!proc) return NULL;
    proc->owned_by_chunk = 0;
    proc->splat_index = -1;

    if (defn->as.defn.param_count > 0) {
        proc->param_names = (char **)luby_alloc_raw(C->L, NULL, defn->as.defn.param_count * sizeof(char *));
        proc->default_chunks = (luby_chunk *)luby_alloc_raw(C->L, NULL, defn->as.defn.param_count * sizeof(luby_chunk));
        if (!proc->param_names || !proc->default_chunks) return NULL;
        proc->param_count = defn->as.defn.param_count;
        
        for (size_t i = 0; i < proc->param_count; i++) {
            luby_ast_node *param = defn->as.defn.params[i];
            luby_chunk_init(&proc->default_chunks[i]);
            
            if (param->kind == LUBY_AST_DEFAULT_PARAM) {
                proc->param_names[i] = luby_dup_string(C->L, param->as.assign.target->as.literal.data, param->as.assign.target->as.literal.length);
                // Compile default value into its own chunk
                luby_compiler dsub;
                dsub.L = C->L;
                dsub.chunk = &proc->default_chunks[i];
                dsub.class_depth = 0;
                dsub.loop_depth = 0;
                luby_compile_node(&dsub, param->as.assign.value);
            } else if (param->kind == LUBY_AST_SPLAT_PARAM) {
                proc->param_names[i] = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
                proc->splat_index = (int)i;
            } else if (param->kind == LUBY_AST_BLOCK_PARAM) {
                proc->block_param_name = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
                proc->has_block_param = 1;
                proc->param_count--; // Don't count block param in regular params
            } else {
                proc->param_names[i] = luby_dup_string(C->L, param->as.literal.data, param->as.literal.length);
            }
        }
    }

    luby_chunk_init(&proc->chunk);
    luby_compiler sub;
    sub.L = C->L;
    sub.chunk = &proc->chunk;
    sub.class_depth = 0;
    sub.loop_depth = 0;
    if (!luby_compile_node(&sub, defn->as.defn.body)) return NULL;
    
    // Collect local variable names from function body (excluding params)
    proc->local_names = NULL;
    proc->local_count = 0;
    luby_collect_locals(C->L, defn->as.defn.body,
                        &proc->local_names, &proc->local_count,
                        proc->param_names, proc->param_count);

    // Set visibility from current state
    proc->visibility = C->L->current_visibility;
    
    return proc;
}

static int luby_compile_call(luby_compiler *C, luby_ast_node *node) {
    if (!node->as.call.recv && node->as.call.method.length == 5 &&
        strncmp(node->as.call.method.data, "raise", 5) == 0) {
        if (node->as.call.argc > 0) {
            if (!luby_compile_node(C, node->as.call.args[0])) return 0;
        } else {
            if (!luby_emit_nil(C, node->line)) return 0;
        }
        luby_chunk_emit(C->L, C->chunk, LUBY_OP_THROW, 0, 0, 0, node->line);
        return 1;
    }
    if (!node->as.call.recv && node->as.call.method.length == 5 &&
        strncmp(node->as.call.method.data, "yield", 5) == 0) {
        int argc = 0;
        for (size_t i = 0; i < node->as.call.argc; i++) {
            if (!luby_compile_node(C, node->as.call.args[i])) return 0;
            argc++;
        }
        luby_chunk_emit(C->L, C->chunk, LUBY_OP_YIELD, (uint8_t)argc, 0, 0, node->line);
        return 1;
    }
    int argc = 0;
    if (node->as.call.block) {
        luby_proc *proc = luby_compile_block_proc(C, node->as.call.block);
        if (!proc) return 0;
        luby_value pv = luby_nil();
        pv.type = LUBY_T_PROC;
        pv.as.ptr = proc;
        uint32_t pidx = luby_chunk_add_const(C->L, C->chunk, pv);
        luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_BLOCK, 0, 0, pidx, node->line);
    } else {
        luby_value pv = luby_nil();
        uint32_t pidx = luby_chunk_add_const(C->L, C->chunk, pv);
        luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_BLOCK, 0, 0, pidx, node->line);
    }
    if (node->as.call.recv) {
        if (!luby_compile_node(C, node->as.call.recv)) return 0;
        argc++;
    }
    for (size_t i = 0; i < node->as.call.argc; i++) {
        if (!luby_compile_node(C, node->as.call.args[i])) return 0;
        argc++;
    }
    luby_value sym = luby_symbol(C->L, node->as.call.method.data, node->as.call.method.length);
    uint32_t midx = luby_chunk_add_const(C->L, C->chunk, sym);
    luby_chunk_emit(C->L, C->chunk, node->as.call.safe ? LUBY_OP_SAFE_CALL : LUBY_OP_CALL, (uint8_t)argc, 0, midx, node->line);
    return 1;
}

static int luby_compile_node(luby_compiler *C, luby_ast_node *node) {
    if (!node) return 0;
    switch (node->kind) {
        case LUBY_AST_BLOCK: return luby_compile_block(C, node);
        case LUBY_AST_LITERAL:
        case LUBY_AST_STRING:
        case LUBY_AST_SYMBOL:
        case LUBY_AST_INT:
        case LUBY_AST_FLOAT:
        case LUBY_AST_BOOL:
        case LUBY_AST_NIL:
            return luby_compile_literal(C, node);
        case LUBY_AST_INTERP_STRING: {
            // Compile each part (alternating string literals and expressions)
            for (size_t i = 0; i < node->as.list.count; i++) {
                if (!luby_compile_node(C, node->as.list.items[i])) return 0;
            }
            // CONCAT takes count of parts, concatenates them into one string
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONCAT, (uint8_t)node->as.list.count, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_IDENT: {
            luby_value sym = luby_symbol(C->L, node->as.literal.data, node->as.literal.length);
            uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_GLOBAL, 0, 0, idx, node->line);
            return 1;
        }
        case LUBY_AST_CONST: {
            luby_value sym = luby_symbol(C->L, node->as.literal.data, node->as.literal.length);
            uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_GLOBAL, 0, 0, idx, node->line);
            return 1;
        }
        case LUBY_AST_BINARY: {
            if (!luby_compile_node(C, node->as.binary.left)) return 0;
            if (!luby_compile_node(C, node->as.binary.right)) return 0;
            luby_op op = luby_binary_op_from_token(node->as.binary.op);
            if (node->as.binary.op == LUBY_TOK_NEQ) {
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_EQ, 0, 0, 0, node->line);
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_NOT, 0, 0, 0, node->line);
            } else {
                luby_chunk_emit(C->L, C->chunk, op, 0, 0, 0, node->line);
            }
            return 1;
        }
        case LUBY_AST_CALL:
            return luby_compile_call(C, node);
        case LUBY_AST_INDEX: {
            if (!luby_compile_node(C, node->as.index.target)) return 0;
            if (!luby_compile_node(C, node->as.index.index)) return 0;
            luby_chunk_emit(C->L, C->chunk, node->as.index.safe ? LUBY_OP_SAFE_INDEX : LUBY_OP_GET_INDEX, 0, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_ARRAY: {
            for (size_t i = 0; i < node->as.list.count; i++) {
                if (!luby_compile_node(C, node->as.list.items[i])) return 0;
            }
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MAKE_ARRAY, (uint8_t)node->as.list.count, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_HASH: {
            for (size_t i = 0; i < node->as.list.count; i++) {
                luby_ast_node *pair = node->as.list.items[i];
                if (!luby_compile_node(C, pair->as.pair.left)) return 0;
                if (!luby_compile_node(C, pair->as.pair.right)) return 0;
            }
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MAKE_HASH, (uint8_t)node->as.list.count, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_ASSIGN: {
            if (!luby_compile_node(C, node->as.assign.value)) return 0;
            luby_value sym = luby_symbol(C->L, node->as.assign.target->as.literal.data, node->as.assign.target->as.literal.length);
            uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, idx, node->line);
            return 1;
        }
        case LUBY_AST_MULTI_ASSIGN: {
            // Multiple assignment: a, b = 1, 2 or a, b = [1, 2]
            size_t tc = node->as.multi_assign.target_count;
            size_t vc = node->as.multi_assign.value_count;
            // If single value on RHS and it's an array, we need to destructure at runtime
            // First, evaluate all RHS values onto the stack
            for (size_t i = 0; i < vc; i++) {
                if (!luby_compile_node(C, node->as.multi_assign.values[i])) return 0;
            }
            // If we have fewer values than targets, we may need array destructuring
            // Emit special multi-assign opcode with target count and value count
            // For simplicity, emit MULTI_UNPACK which will handle both cases
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MULTI_UNPACK, (uint8_t)tc, (uint8_t)vc, 0, node->line);
            // Now assign each target from the stack (in reverse order since stack is LIFO)
            for (size_t i = tc; i > 0; i--) {
                luby_ast_node *target = node->as.multi_assign.targets[i - 1];
                if (target->kind == LUBY_AST_IDENT) {
                    luby_value sym = luby_symbol(C->L, target->as.literal.data, target->as.literal.length);
                    uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
                    luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, idx, node->line);
                    luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
                } else if (target->kind == LUBY_AST_IVAR) {
                    luby_value sym = luby_symbol(C->L, target->as.literal.data, target->as.literal.length);
                    uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
                    luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_IVAR, 0, 0, idx, node->line);
                    luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
                }
            }
            // Push nil as the result of multi-assign
            {
                uint32_t nil_idx = luby_chunk_add_const(C->L, C->chunk, luby_nil());
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONST, 0, 0, nil_idx, node->line);
            }
            return 1;
        }
        case LUBY_AST_IVAR: {
            // Get instance variable
            luby_value sym = luby_symbol(C->L, node->as.literal.data, node->as.literal.length);
            uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_IVAR, 0, 0, idx, node->line);
            return 1;
        }
        case LUBY_AST_IVAR_ASSIGN: {
            // Set instance variable
            if (!luby_compile_node(C, node->as.assign.value)) return 0;
            luby_value sym = luby_symbol(C->L, node->as.assign.target->as.literal.data, node->as.assign.target->as.literal.length);
            uint32_t idx = luby_chunk_add_const(C->L, C->chunk, sym);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_IVAR, 0, 0, idx, node->line);
            return 1;
        }
        case LUBY_AST_RANGE: {
            // Compile start and end expressions
            if (!luby_compile_node(C, node->as.range.start)) return 0;
            if (!luby_compile_node(C, node->as.range.end)) return 0;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MAKE_RANGE, (uint8_t)node->as.range.exclusive, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_TERNARY: {
            // cond ? then : else
            if (!luby_compile_node(C, node->as.if_stmt.cond)) return 0;
            size_t jmp_false = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP_IF_FALSE, node->line);
            if (!luby_compile_node(C, node->as.if_stmt.then_branch)) return 0;
            size_t jmp_end = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP, node->line);
            luby_chunk_patch_jump(C->chunk, jmp_false, C->chunk->count);
            if (!luby_compile_node(C, node->as.if_stmt.else_branch)) return 0;
            luby_chunk_patch_jump(C->chunk, jmp_end, C->chunk->count);
            return 1;
        }
        case LUBY_AST_INDEX_ASSIGN: {
            if (!luby_compile_node(C, node->as.index_assign.target)) return 0;
            if (!luby_compile_node(C, node->as.index_assign.index)) return 0;
            if (!luby_compile_node(C, node->as.index_assign.value)) return 0;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_INDEX, 0, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_IF: {
            if (!luby_compile_node(C, node->as.if_stmt.cond)) return 0;
            size_t jmp_false = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP_IF_FALSE, node->line);
            if (!luby_compile_node(C, node->as.if_stmt.then_branch)) return 0;
            size_t jmp_end = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP, node->line);
            luby_chunk_patch_jump(C->chunk, jmp_false, C->chunk->count);
            if (node->as.if_stmt.else_branch) {
                if (!luby_compile_node(C, node->as.if_stmt.else_branch)) return 0;
            }
            luby_chunk_patch_jump(C->chunk, jmp_end, C->chunk->count);
            return 1;
        }
        case LUBY_AST_WHILE: {
            size_t loop_start = C->chunk->count;
            if (C->loop_depth >= 16) return 0;
            C->loops[C->loop_depth].start = loop_start;
            C->loops[C->loop_depth].breaks = NULL;
            C->loops[C->loop_depth].break_count = 0;
            C->loops[C->loop_depth].break_capacity = 0;
            C->loop_depth++;
            if (!luby_compile_node(C, node->as.while_stmt.cond)) goto loop_fail;
            size_t jmp_exit = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP_IF_FALSE, node->line);
            C->loops[C->loop_depth - 1].body_start = C->chunk->count;
            if (!luby_compile_node(C, node->as.while_stmt.body)) goto loop_fail;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_JUMP, 0, 0, (uint32_t)loop_start, node->line);
            luby_chunk_patch_jump(C->chunk, jmp_exit, C->chunk->count);
            if (C->loop_depth > 0) {
                int idx = C->loop_depth - 1;
                for (size_t i = 0; i < C->loops[idx].break_count; i++) {
                    luby_chunk_patch_jump(C->chunk, C->loops[idx].breaks[i], C->chunk->count);
                }
                luby_alloc_raw(C->L, C->loops[idx].breaks, 0);
                C->loop_depth--;
            }
            return 1;
        loop_fail:
            if (C->loop_depth > 0) {
                int idx = C->loop_depth - 1;
                luby_alloc_raw(C->L, C->loops[idx].breaks, 0);
                C->loop_depth--;
            }
            return 0;
        }
        case LUBY_AST_DEF: {
            luby_proc *proc = luby_compile_def_proc(C, node);
            if (!proc) return 0;
            luby_value pv = luby_nil();
            pv.type = LUBY_T_PROC;
            pv.as.ptr = proc;
            uint32_t pidx = luby_chunk_add_const(C->L, C->chunk, pv);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONST, 0, 0, pidx, node->line);
            luby_value sym = luby_symbol(C->L, node->as.defn.name.data, node->as.defn.name.length);
            uint32_t sidx = luby_chunk_add_const(C->L, C->chunk, sym);
            
            if (node->as.defn.receiver) {
                // Singleton method: def receiver.method_name
                if (!luby_compile_node(C, node->as.defn.receiver)) return 0;
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_DEF_SINGLETON, 0, 0, sidx, node->line);
            } else if (C->class_depth > 0) {
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_DEF_METHOD, 0, 0, sidx, node->line);
            } else {
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, sidx, node->line);
            }
            return 1;
        }
        case LUBY_AST_CLASS: {
            luby_value namev = luby_symbol(C->L, node->as.class_decl.name.data, node->as.class_decl.name.length);
            uint32_t nidx = luby_chunk_add_const(C->L, C->chunk, namev);
            uint16_t sidx = (uint16_t)0xFFFF;
            if (node->as.class_decl.super_name.data && node->as.class_decl.super_name.length > 0) {
                luby_value sv = luby_symbol(C->L, node->as.class_decl.super_name.data, node->as.class_decl.super_name.length);
                sidx = (uint16_t)luby_chunk_add_const(C->L, C->chunk, sv);
            }
            // Add "self" symbol for setting self inside class body
            luby_value selfsym = luby_symbol(C->L, "self", 4);
            uint32_t selfidx = luby_chunk_add_const(C->L, C->chunk, selfsym);
            
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_CLASS, 0, 0, 0, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MAKE_CLASS, 0, sidx, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_GLOBAL, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_CLASS, 0, 0, 0, node->line);
            // Set self to the class inside the body
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_GLOBAL, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, selfidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
            C->class_depth++;
            if (node->as.class_decl.body && !luby_compile_node(C, node->as.class_decl.body)) return 0;
            C->class_depth--;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_CLASS, 0, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_MODULE: {
            luby_value namev = luby_symbol(C->L, node->as.module_decl.name.data, node->as.module_decl.name.length);
            uint32_t nidx = luby_chunk_add_const(C->L, C->chunk, namev);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_CLASS, 0, 0, 0, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_MAKE_MODULE, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_GLOBAL, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_GET_GLOBAL, 0, 0, nidx, node->line);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_CLASS, 0, 0, 0, node->line);
            C->class_depth++;
            if (node->as.module_decl.body && !luby_compile_node(C, node->as.module_decl.body)) return 0;
            C->class_depth--;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_CLASS, 0, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_RETURN:
            if (node->as.ret.value && !luby_compile_node(C, node->as.ret.value)) return 0;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_RET, 0, 0, 0, node->line);
            return 1;
        case LUBY_AST_BREAK: {
            if (C->loop_depth <= 0) return 0;
            if (node->as.ret.value) {
                if (!luby_compile_node(C, node->as.ret.value)) return 0;
            } else {
                if (!luby_emit_nil(C, node->line)) return 0;
            }
            size_t jmp = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP, node->line);
            return luby_loop_add_break(C, jmp);
        }
        case LUBY_AST_NEXT: {
            if (C->loop_depth <= 0) return 0;
            if (node->as.ret.value) {
                if (!luby_compile_node(C, node->as.ret.value)) return 0;
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_POP, 0, 0, 0, node->line);
            }
            size_t target = C->loops[C->loop_depth - 1].start;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_JUMP, 0, 0, (uint32_t)target, node->line);
            return 1;
        }
        case LUBY_AST_REDO: {
            if (C->loop_depth <= 0) return 0;
            size_t target = C->loops[C->loop_depth - 1].body_start;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_JUMP, 0, 0, (uint32_t)target, node->line);
            return 1;
        }
        case LUBY_AST_BEGIN: {
            if (!node->as.begin.body) return 0;
            if (!node->as.begin.rescue_body && !node->as.begin.ensure_body) {
                return luby_compile_node(C, node->as.begin.body);
            }

            size_t try_at = C->chunk->count;
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_TRY, 0, 0, LUBY_IP_NONE, node->line);
            size_t ensure_at = 0;
            if (node->as.begin.ensure_body) {
                ensure_at = C->chunk->count;
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_SET_ENSURE, 0, 0, LUBY_IP_NONE, node->line);
            }

            if (!luby_compile_node(C, node->as.begin.body)) return 0;

            size_t jump_after_body = 0;
            if (node->as.begin.rescue_body || node->as.begin.ensure_body) {
                jump_after_body = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP, node->line);
            }

            uint32_t rescue_ip = LUBY_IP_NONE;
            size_t jump_after_rescue = 0;
            if (node->as.begin.rescue_body) {
                rescue_ip = (uint32_t)C->chunk->count;
                if (!luby_compile_node(C, node->as.begin.rescue_body)) return 0;
                if (node->as.begin.ensure_body) {
                    jump_after_rescue = luby_chunk_emit_jump(C->L, C->chunk, LUBY_OP_JUMP, node->line);
                }
            }

            uint32_t ensure_ip = LUBY_IP_NONE;
            if (node->as.begin.ensure_body) {
                ensure_ip = (uint32_t)C->chunk->count;
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_ENTER_ENSURE, 0, 0, 0, node->line);
                if (!luby_compile_node(C, node->as.begin.ensure_body)) return 0;
            }

            if (jump_after_body) {
                luby_chunk_patch_jump(C->chunk, jump_after_body, node->as.begin.ensure_body ? ensure_ip : C->chunk->count);
            }
            if (jump_after_rescue) {
                luby_chunk_patch_jump(C->chunk, jump_after_rescue, ensure_ip);
            }
            luby_chunk_patch_jump(C->chunk, try_at, rescue_ip);
            if (node->as.begin.ensure_body) {
                luby_chunk_patch_jump(C->chunk, ensure_at, ensure_ip);
            }
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_END_TRY, 0, 0, 0, node->line);
            return 1;
        }
        case LUBY_AST_UNARY: {
            if (!luby_compile_node(C, node->as.unary.expr)) return 0;
            if (node->as.unary.op == LUBY_TOK_BANG || node->as.unary.op == LUBY_TOK_NOT) {
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_NOT, 0, 0, 0, node->line);
            } else if (node->as.unary.op == LUBY_TOK_MINUS) {
                luby_chunk_emit(C->L, C->chunk, LUBY_OP_NEG, 0, 0, 0, node->line);
            }
            return 1;
        }
        case LUBY_AST_LAMBDA: {
            // Stabby lambda or block as standalone expression
            luby_proc *proc = luby_compile_block_proc(C, node);
            if (!proc) return 0;
            luby_value pv = luby_nil();
            pv.type = LUBY_T_PROC;
            pv.as.ptr = proc;
            uint32_t pidx = luby_chunk_add_const(C->L, C->chunk, pv);
            luby_chunk_emit(C->L, C->chunk, LUBY_OP_CONST, 0, 0, pidx, node->line);
            return 1;
        }
        default:
            return 1;
    }
}

// ------------------------------ API Impl ----------------------------------

LUBY_API luby_state *luby_new(const luby_config *cfg) {
    luby_state *L = (luby_state *)luby_default_alloc(NULL, NULL, sizeof(luby_state));
    if (!L) return NULL;
    memset(L, 0, sizeof(*L));
    if (cfg) L->cfg = *cfg;
    L->last_error.code = LUBY_E_OK;
    L->current_block = luby_nil();
    L->current_class = luby_nil();
    L->current_self = luby_nil();
    L->current_method_class = NULL;
    L->current_method_name = NULL;
    L->current_visibility = LUBY_VIS_PUBLIC;  // default visibility is public
    L->method_epoch = 1;
    L->current_coroutine = NULL;
    L->current_vm = NULL;
    // Initialize RNG with a default seed
    L->rng_state[0] = 0x853c49e6748fea9bULL;
    L->rng_state[1] = 0xda3e39cb94b95bdbULL;
    // Initialize GC
    L->gc_threshold = LUBY_GC_INITIAL_THRESHOLD;
    return L;
}

LUBY_API void luby_free(luby_state *L) {
    if (!L) return;
    // Free all GC-tracked objects (mark nothing, sweep everything)
    L->gc_paused = 1;
    luby_gc_obj *obj = L->gc_objects;
    while (obj) {
        luby_gc_obj *next = obj->gc_next;
        luby_gc_free_obj(L, obj);
        obj = next;
    }
    L->gc_objects = NULL;
    L->gc_total = 0;
    // Free bookkeeping arrays
    for (size_t i = 0; i < L->global_count; i++) {
        luby_alloc_raw(L, (void *)L->global_names[i].data, 0);
    }
    for (size_t i = 0; i < L->search_path_count; i++) {
        luby_alloc_raw(L, L->search_paths[i], 0);
    }
    for (size_t i = 0; i < L->loaded_count; i++) {
        luby_alloc_raw(L, L->loaded_paths[i], 0);
    }
    for (size_t i = 0; i < L->symbol_count; i++) {
        luby_alloc_raw(L, L->symbol_names[i], 0);
    }
    luby_alloc_raw(L, L->global_names, 0);
    luby_alloc_raw(L, L->global_values, 0);
    luby_alloc_raw(L, L->search_paths, 0);
    luby_alloc_raw(L, L->loaded_paths, 0);
    luby_alloc_raw(L, L->cfuncs.names, 0);
    luby_alloc_raw(L, L->cfuncs.funcs, 0);
    luby_alloc_raw(L, L->symbol_names, 0);
    luby_default_alloc(NULL, L, 0);
}

static int luby_execute_chunk(luby_state *L, luby_chunk *chunk, luby_value *out, const char *filename) {
    luby_vm vm;
    luby_vm_init(&vm);
    if (!luby_vm_ensure_stack(L, &vm, 1)) return (int)LUBY_E_OOM;
    if (!luby_vm_push_frame(L, &vm, NULL, chunk, filename, luby_nil(), NULL, NULL, 0, NULL, luby_nil(), 0)) {
        luby_vm_free(L, &vm);
        return (int)LUBY_E_OOM;
    }
    int rc = luby_vm_run(L, &vm, out);
    luby_vm_free(L, &vm);
    return rc;
}

static int luby_call_block(luby_state *L, luby_proc *proc, int argc, const luby_value *argv, luby_value *out) {
    if (!proc) return (int)LUBY_E_TYPE;
    luby_value saved_block = L->current_block;
    L->current_block = luby_nil();

    luby_vm vm;
    luby_vm_init(&vm);
    if (!luby_vm_ensure_stack(L, &vm, 1)) {
        L->current_block = saved_block;
        return (int)LUBY_E_OOM;
    }
    if (!luby_vm_push_frame(L, &vm, proc, &proc->chunk, "<block>", luby_nil(), NULL, NULL, argc, argv, luby_nil(), 0)) {
        luby_vm_free(L, &vm);
        L->current_block = saved_block;
        return (int)LUBY_E_OOM;
    }
    int rc = luby_vm_run(L, &vm, out);
    luby_vm_free(L, &vm);
    L->current_block = saved_block;
    return rc;
}

static int luby_call_proc_with_self(luby_state *L, luby_proc *proc, luby_value recv, int argc, const luby_value *argv, luby_value *out) {
    if (!proc) return (int)LUBY_E_TYPE;
    luby_value saved_block = L->current_block;
    L->current_block = luby_nil();

    luby_vm vm;
    luby_vm_init(&vm);
    if (!luby_vm_ensure_stack(L, &vm, 1)) {
        L->current_block = saved_block;
        return (int)LUBY_E_OOM;
    }
    if (!luby_vm_push_frame(L, &vm, proc, &proc->chunk, "<method>", recv, L->current_method_class, L->current_method_name, argc, argv, luby_nil(), 1)) {
        luby_vm_free(L, &vm);
        L->current_block = saved_block;
        return (int)LUBY_E_OOM;
    }
    int rc = luby_vm_run(L, &vm, out);
    luby_vm_free(L, &vm);
    L->current_block = saved_block;
    return rc;
}

static int luby_call_method(luby_state *L, luby_class_obj *cls, const char *name, luby_proc *proc, luby_value recv, int argc, const luby_value *argv, luby_value *out) {
    luby_class_obj *saved_class = L->current_method_class;
    const char *saved_name = L->current_method_name;
    L->current_method_class = cls;
    L->current_method_name = name;
    int rc = luby_call_proc_with_self(L, proc, recv, argc, argv, out);
    L->current_method_class = saved_class;
    L->current_method_name = saved_name;
    return rc;
}

static int luby_eval_with_context(luby_state *L, luby_value new_class, luby_value new_self, const char *code, size_t len, const char *filename, luby_value *out) {
    luby_value saved_class = L->current_class;
    luby_value saved_self = L->current_self;
    L->current_class = new_class;
    L->current_self = new_self;
    int rc = luby_eval(L, code, len, filename, out);
    L->current_class = saved_class;
    L->current_self = saved_self;
    return rc;
}

LUBY_API int luby_eval(luby_state *L, const char *code, size_t len, const char *filename, luby_value *out) {
    luby_clear_error(L);
    luby_error err = {0};
    luby_ast_node *ast = luby_parse(L, code, len, filename, &err);
    if (err.code != LUBY_E_OK || !ast) {
        luby_free_ast(L, ast);
        luby_set_error(L, err.code, err.message ? err.message : "parse error", err.file, err.line, err.column);
        return (int)err.code;
    }
    luby_chunk chunk;
    luby_chunk_init(&chunk);
    luby_compiler C;
    C.L = L;
    C.chunk = &chunk;
    C.class_depth = (L->current_class.type == LUBY_T_CLASS || L->current_class.type == LUBY_T_MODULE) ? 1 : 0;
    C.loop_depth = 0;
    if (!luby_compile_node(&C, ast)) {
        luby_free_ast(L, ast);
        luby_chunk_free(L, &chunk);
        luby_set_error(L, LUBY_E_PARSE, "compile error", filename, 0, 0);
        return (int)LUBY_E_PARSE;
    }
    luby_free_ast(L, ast);
    luby_value result = luby_nil();
    int rc = luby_execute_chunk(L, &chunk, &result, filename);

    // Result strings are GC-tracked, so they remain valid after chunk_free
    if (out) {
        *out = result;
    }

    luby_chunk_free(L, &chunk);
    if (L->last_error.code != LUBY_E_OK) return (int)L->last_error.code;
    return rc;
}

LUBY_API int luby_require(luby_state *L, const char *path, luby_value *out) {
    if (!L || !path) return (int)LUBY_E_RUNTIME;
    if (!L->cfg.vfs.read || !L->cfg.vfs.exists) {
        luby_set_error(L, LUBY_E_IO, "vfs not configured", NULL, 0, 0);
        return (int)LUBY_E_IO;
    }

    const char *candidate = path;
    size_t path_len = strlen(path);
    char *with_ext = NULL;
    if (path_len < 3 || strcmp(path + path_len - 3, ".rb") != 0) {
        with_ext = (char *)luby_alloc_raw(L, NULL, path_len + 4);
        if (with_ext) {
            memcpy(with_ext, path, path_len);
            memcpy(with_ext + path_len, ".rb", 4);
        }
    }

    char fullpath[1024];
    const char *resolved = NULL;

    if (L->cfg.vfs.exists(L->cfg.vfs.user, candidate)) {
        resolved = candidate;
    } else if (with_ext && L->cfg.vfs.exists(L->cfg.vfs.user, with_ext)) {
        resolved = with_ext;
    } else {
        for (size_t i = 0; i < L->search_path_count && !resolved; i++) {
            const char *base = L->search_paths[i];
            if (!base) continue;
            size_t blen = strlen(base);
            snprintf(fullpath, sizeof(fullpath), "%s%s%s", base, (blen && base[blen - 1] == '/') ? "" : "/", candidate);
            if (L->cfg.vfs.exists(L->cfg.vfs.user, fullpath)) { resolved = fullpath; break; }
            if (with_ext) {
                snprintf(fullpath, sizeof(fullpath), "%s%s%s", base, (blen && base[blen - 1] == '/') ? "" : "/", with_ext);
                if (L->cfg.vfs.exists(L->cfg.vfs.user, fullpath)) { resolved = fullpath; break; }
            }
        }
    }

    if (!resolved) {
        luby_set_error(L, LUBY_E_IO, "module not found", path, 0, 0);
        luby_alloc_raw(L, with_ext, 0);
        return (int)LUBY_E_IO;
    }

    if (luby_string_list_contains(L->loaded_paths, L->loaded_count, resolved)) {
        luby_alloc_raw(L, with_ext, 0);
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }

    size_t size = 0;
    char *code = L->cfg.vfs.read(L->cfg.vfs.user, resolved, &size);
    if (!code) {
        luby_set_error(L, LUBY_E_IO, "read failed", resolved, 0, 0);
        luby_alloc_raw(L, with_ext, 0);
        return (int)LUBY_E_IO;
    }

    luby_string_list_add(L, &L->loaded_paths, &L->loaded_count, &L->loaded_capacity, resolved);
    luby_value res;
    int rc = luby_eval(L, code, size, resolved, &res);
    L->cfg.alloc ? L->cfg.alloc(L->cfg.alloc_user, code, 0) : luby_default_alloc(NULL, code, 0);
    luby_alloc_raw(L, with_ext, 0);
    if (out) *out = luby_bool(rc == 0);
    return rc;
}

LUBY_API int luby_load(luby_state *L, const char *path, luby_value *out) {
    if (!L || !path) return (int)LUBY_E_RUNTIME;
    if (!L->cfg.vfs.read || !L->cfg.vfs.exists) {
        luby_set_error(L, LUBY_E_IO, "vfs not configured", NULL, 0, 0);
        return (int)LUBY_E_IO;
    }

    const char *candidate = path;
    size_t path_len = strlen(path);
    char *with_ext = NULL;
    if (path_len < 3 || strcmp(path + path_len - 3, ".rb") != 0) {
        with_ext = (char *)luby_alloc_raw(L, NULL, path_len + 4);
        if (with_ext) {
            memcpy(with_ext, path, path_len);
            memcpy(with_ext + path_len, ".rb", 4);
        }
    }

    char fullpath[1024];
    const char *resolved = NULL;

    if (L->cfg.vfs.exists(L->cfg.vfs.user, candidate)) {
        resolved = candidate;
    } else if (with_ext && L->cfg.vfs.exists(L->cfg.vfs.user, with_ext)) {
        resolved = with_ext;
    } else {
        for (size_t i = 0; i < L->search_path_count && !resolved; i++) {
            const char *base = L->search_paths[i];
            if (!base) continue;
            size_t blen = strlen(base);
            snprintf(fullpath, sizeof(fullpath), "%s%s%s", base, (blen && base[blen - 1] == '/') ? "" : "/", candidate);
            if (L->cfg.vfs.exists(L->cfg.vfs.user, fullpath)) { resolved = fullpath; break; }
            if (with_ext) {
                snprintf(fullpath, sizeof(fullpath), "%s%s%s", base, (blen && base[blen - 1] == '/') ? "" : "/", with_ext);
                if (L->cfg.vfs.exists(L->cfg.vfs.user, fullpath)) { resolved = fullpath; break; }
            }
        }
    }

    if (!resolved) {
        luby_set_error(L, LUBY_E_IO, "module not found", path, 0, 0);
        luby_alloc_raw(L, with_ext, 0);
        return (int)LUBY_E_IO;
    }

    size_t size = 0;
    char *code = L->cfg.vfs.read(L->cfg.vfs.user, resolved, &size);
    if (!code) {
        luby_set_error(L, LUBY_E_IO, "read failed", resolved, 0, 0);
        luby_alloc_raw(L, with_ext, 0);
        return (int)LUBY_E_IO;
    }

    luby_value res;
    int rc = luby_eval(L, code, size, resolved, &res);
    L->cfg.alloc ? L->cfg.alloc(L->cfg.alloc_user, code, 0) : luby_default_alloc(NULL, code, 0);
    luby_alloc_raw(L, with_ext, 0);
    if (out) *out = luby_bool(rc == 0);
    return rc;
}

LUBY_API luby_error luby_last_error(luby_state *L) {
    luby_error err = {0};
    if (!L) return err;
    return L->last_error;
}

LUBY_API void luby_clear_error(luby_state *L) {
    if (!L) return;
    L->last_error.code = LUBY_E_OK;
    L->last_error.message = NULL;
    L->last_error.file = NULL;
    L->last_error.line = 0;
    L->last_error.column = 0;
}

LUBY_API const char *luby_error_code_string(luby_error_code code) {
    switch (code) {
        case LUBY_E_OK: return "ok";
        case LUBY_E_PARSE: return "parse";
        case LUBY_E_RUNTIME: return "runtime";
        case LUBY_E_OOM: return "oom";
        case LUBY_E_IO: return "io";
        case LUBY_E_TYPE: return "type";
        case LUBY_E_NAME: return "name";
        default: return "unknown";
    }
}

LUBY_API size_t luby_format_error(luby_state *L, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return 0;
    if (!L || L->last_error.code == LUBY_E_OK) {
        buffer[0] = '\0';
        return 0;
    }
    const char *code = luby_error_code_string(L->last_error.code);
    const char *msg = L->last_error.message ? L->last_error.message : "error";
    const char *file = L->last_error.file ? L->last_error.file : "<unknown>";
    int line = L->last_error.line;
    int col = L->last_error.column;
    int written = 0;
    if (line > 0) {
        if (col > 0) {
            written = snprintf(buffer, buffer_size, "%s:%d:%d: %s: %s", file, line, col, code, msg);
        } else {
            written = snprintf(buffer, buffer_size, "%s:%d: %s: %s", file, line, code, msg);
        }
    } else {
        written = snprintf(buffer, buffer_size, "%s: %s: %s", file, code, msg);
    }
    if (written < 0) {
        buffer[0] = '\0';
        return 0;
    }
    return (size_t)written;
}

LUBY_API luby_value luby_nil(void) { luby_value v; v.type = LUBY_T_NIL; v.as.ptr = NULL; return v; }
LUBY_API luby_value luby_bool(int b) { luby_value v; v.type = LUBY_T_BOOL; v.as.b = !!b; return v; }
LUBY_API luby_value luby_int(int64_t v) { luby_value r; r.type = LUBY_T_INT; r.as.i = v; return r; }
LUBY_API luby_value luby_float(double v) { luby_value r; r.type = LUBY_T_FLOAT; r.as.f = v; return r; }
LUBY_API luby_value luby_string(luby_state *L, const char *s, size_t len) {
    luby_value v; v.type = LUBY_T_STRING;
    if (!s) { v.as.ptr = NULL; return v; }
    if (len == 0) len = strlen(s);
    v.as.ptr = luby_gc_alloc_string(L, s, len);
    return v;
}
LUBY_API luby_value luby_symbol(luby_state *L, const char *s, size_t len) {
    luby_value v; v.type = LUBY_T_SYMBOL;
    if (!s) { v.as.ptr = NULL; return v; }
    v.as.ptr = (void *)luby_intern_symbol(L, s, len);
    return v;
}

LUBY_API void luby_set_global_value(luby_state *L, const char *name, luby_value v) {
    if (!L || !name) return;
    luby_string_view sv = { name, strlen(name) };
    luby_set_global(L, sv, v);
}

LUBY_API luby_value luby_get_global_value(luby_state *L, const char *name) {
    if (!L || !name) return luby_nil();
    luby_string_view sv = { name, strlen(name) };
    return luby_get_global(L, sv);
}

LUBY_API luby_value luby_array_new(luby_state *L) {
    luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!arr) return luby_nil();
    arr->count = 0;
    arr->capacity = 0;
    arr->items = NULL;
    arr->frozen = 0;
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = arr;
    return v;
}

LUBY_API size_t luby_array_len(luby_value arr) {
    if (arr.type != LUBY_T_ARRAY || !arr.as.ptr) return 0;
    return ((luby_array *)arr.as.ptr)->count;
}

LUBY_API int luby_array_get(luby_value arr, size_t index, luby_value *out) {
    if (arr.type != LUBY_T_ARRAY || !arr.as.ptr) return (int)LUBY_E_TYPE;
    luby_array *a = (luby_array *)arr.as.ptr;
    if (index >= a->count) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }
    if (out) *out = a->items[index];
    return (int)LUBY_E_OK;
}

LUBY_API int luby_array_set(luby_state *L, luby_value arr, size_t index, luby_value v) {
    if (arr.type != LUBY_T_ARRAY || !arr.as.ptr) return (int)LUBY_E_TYPE;
    luby_array *a = (luby_array *)arr.as.ptr;
    if (a->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    if (index >= a->capacity) {
        size_t new_cap = a->capacity < 8 ? 8 : a->capacity;
        while (index >= new_cap) new_cap *= 2;
        luby_value *ni = (luby_value *)luby_alloc_raw(L, a->items, new_cap * sizeof(luby_value));
        if (!ni) return (int)LUBY_E_OOM;
        for (size_t i = a->capacity; i < new_cap; i++) ni[i] = luby_nil();
        a->items = ni;
        a->capacity = new_cap;
    }
    if (index >= a->count) a->count = index + 1;
    a->items[index] = v;
    return (int)LUBY_E_OK;
}

LUBY_API int luby_array_push_value(luby_state *L, luby_value arr, luby_value v) {
    return luby_array_set(L, arr, luby_array_len(arr), v);
}

LUBY_API luby_value luby_hash_new(luby_state *L) {
    luby_hash *h = (luby_hash *)luby_gc_alloc(L, sizeof(luby_hash), LUBY_GC_HASH);
    if (!h) return luby_nil();
    h->count = 0;
    h->capacity = 0;
    h->entries = NULL;
    h->frozen = 0;
    luby_value v; v.type = LUBY_T_HASH; v.as.ptr = h;
    return v;
}

LUBY_API size_t luby_hash_len(luby_value h) {
    if (h.type != LUBY_T_HASH || !h.as.ptr) return 0;
    return ((luby_hash *)h.as.ptr)->count;
}

LUBY_API int luby_hash_get_value(luby_value h, luby_value key, luby_value *out) {
    if (h.type != LUBY_T_HASH || !h.as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *hh = (luby_hash *)h.as.ptr;
    for (size_t i = 0; i < hh->count; i++) {
        if (luby_value_eq(hh->entries[i].key, key)) {
            if (out) *out = hh->entries[i].value;
            return (int)LUBY_E_OK;
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

LUBY_API int luby_hash_set_value(luby_state *L, luby_value h, luby_value key, luby_value value) {
    if (h.type != LUBY_T_HASH || !h.as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *hh = (luby_hash *)h.as.ptr;
    if (hh->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    for (size_t i = 0; i < hh->count; i++) {
        if (luby_value_eq(hh->entries[i].key, key)) {
            hh->entries[i].value = value;
            return (int)LUBY_E_OK;
        }
    }
    if (hh->count + 1 > hh->capacity) {
        size_t new_cap = hh->capacity < 8 ? 8 : hh->capacity * 2;
        luby_hash_entry *ne = (luby_hash_entry *)luby_alloc_raw(L, hh->entries, new_cap * sizeof(luby_hash_entry));
        if (!ne) return (int)LUBY_E_OOM;
        hh->entries = ne;
        hh->capacity = new_cap;
    }
    hh->entries[hh->count].key = key;
    hh->entries[hh->count].value = value;
    hh->count++;
    return (int)LUBY_E_OK;
}

LUBY_API int luby_invoke_global(luby_state *L, const char *name, int argc, const luby_value *argv, luby_value *out) {
    if (!L || !name) return (int)LUBY_E_RUNTIME;
    
    // First check for a Luby-defined proc in globals
    luby_string_view sv = { name, strlen(name) };
    luby_value gv = luby_get_global(L, sv);
    if (gv.type == LUBY_T_PROC && gv.as.ptr) {
        luby_proc *proc = (luby_proc *)gv.as.ptr;
        luby_vm vm;
        luby_vm_init(&vm);
        luby_vm *saved_vm = L->current_vm;
        L->current_vm = &vm;
        
        if (!luby_vm_push_frame(L, &vm, proc, &proc->chunk, name, luby_nil(), NULL, name, argc, argv, luby_nil(), 0)) {
            L->current_vm = saved_vm;
            luby_vm_free(L, &vm);
            return (int)LUBY_E_OOM;
        }
        
        int rc = luby_vm_run(L, &vm, out);
        L->current_vm = saved_vm;
        luby_vm_free(L, &vm);
        return rc;
    }
    
    // Fall back to C function
    luby_cfunc fn = luby_find_cfunc(L, name);
    if (!fn) {
        luby_set_error(L, LUBY_E_NAME, "undefined function", NULL, 0, 0);
        return (int)LUBY_E_NAME;
    }
    luby_value result = luby_nil();
    int rc = fn(L, argc, argv, &result);
    if (out) *out = result;
    return rc;
}

LUBY_API int luby_invoke_method(luby_state *L, luby_value recv, const char *method, int argc, const luby_value *argv, luby_value *out) {
    if (!L || !method) return (int)LUBY_E_RUNTIME;
    
    luby_class_obj *cls = NULL;
    if (recv.type == LUBY_T_OBJECT && recv.as.ptr) {
        cls = ((luby_object *)recv.as.ptr)->klass;
    } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
        cls = (luby_class_obj *)recv.as.ptr;
    }
    
    if (cls) {
        // Check for singleton method first
        luby_proc *m = NULL;
        if (recv.type == LUBY_T_OBJECT) {
            m = luby_object_get_singleton_method((luby_object *)recv.as.ptr, method);
        } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
            m = luby_class_get_singleton_method(L, cls, method);
        }
        if (!m) m = luby_class_get_method(L, cls, method);
        
        if (m) {
            luby_vm vm;
            luby_vm_init(&vm);
            luby_vm *saved_vm = L->current_vm;
            L->current_vm = &vm;
            
            if (!luby_vm_push_frame(L, &vm, m, &m->chunk, method, recv, cls, method, argc, argv, luby_nil(), 1)) {
                L->current_vm = saved_vm;
                luby_vm_free(L, &vm);
                return (int)LUBY_E_OOM;
            }
            
            int rc = luby_vm_run(L, &vm, out);
            L->current_vm = saved_vm;
            luby_vm_free(L, &vm);
            return rc;
        }
        
        // Check for native method (CMETHOD)
        luby_value method_val = luby_class_lookup_method(L, cls, method);
        if (method_val.type == LUBY_T_CMETHOD && method_val.as.ptr) {
            luby_cmethod *cm = (luby_cmethod *)method_val.as.ptr;
            // Build args with receiver at front
            luby_value *full_argv = (luby_value *)luby_alloc_raw(L, NULL, (argc + 1) * sizeof(luby_value));
            if (!full_argv) return (int)LUBY_E_OOM;
            full_argv[0] = recv;
            for (int i = 0; i < argc; i++) full_argv[i + 1] = argv[i];
            luby_value result = luby_nil();
            int rc = cm->fn(L, argc + 1, full_argv, &result);
            luby_alloc_raw(L, full_argv, 0);
            if (out) *out = result;
            return rc;
        }
    }
    
    luby_set_error(L, LUBY_E_NAME, "undefined method", NULL, 0, 0);
    return (int)LUBY_E_NAME;
}

LUBY_API int luby_call(luby_state *L, luby_value recv, const char *method, int argc, const luby_value *argv, luby_value *out) {
    // If recv is nil, call as global function
    if (recv.type == LUBY_T_NIL) {
        return luby_invoke_global(L, method, argc, argv, out);
    }
    return luby_invoke_method(L, recv, method, argc, argv, out);
}

LUBY_API int luby_register_function(luby_state *L, const char *name, luby_cfunc fn) {
    if (!L || !name || !fn) return (int)LUBY_E_RUNTIME;
    if (L->cfuncs.count + 1 > L->cfuncs.capacity) {
        size_t new_cap = L->cfuncs.capacity < 8 ? 8 : L->cfuncs.capacity * 2;
        const char **nn = (const char **)luby_alloc_raw(L, L->cfuncs.names, new_cap * sizeof(char *));
        luby_cfunc *nf = (luby_cfunc *)luby_alloc_raw(L, L->cfuncs.funcs, new_cap * sizeof(luby_cfunc));
        if (!nn || !nf) return (int)LUBY_E_OOM;
        L->cfuncs.names = nn;
        L->cfuncs.funcs = nf;
        L->cfuncs.capacity = new_cap;
    }
    L->cfuncs.names[L->cfuncs.count] = name;
    L->cfuncs.funcs[L->cfuncs.count] = fn;
    L->cfuncs.count++;
    return (int)LUBY_E_OK;
}

LUBY_API int luby_register_module(luby_state *L, const char *name, luby_cfunc loader) {
    return luby_register_function(L, name, loader);
}

// --------------------------- Base Stdlib -----------------------------------

static int luby_base_print(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    for (int i = 0; i < argc; i++) {
        if (i > 0) printf(" ");
        luby_print_value(argv[i]);
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_puts(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc == 0) {
        printf("\n");
    } else {
        for (int i = 0; i < argc; i++) {
            luby_print_value(argv[i]);
            printf("\n");
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_type(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (out) {
        *out = luby_string(L, luby_type_name(argv[0]), 0);
    }
    return (int)LUBY_E_OK;
}

static int luby_base_to_i(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value v = argv[0];
    if (v.type == LUBY_T_INT) { if (out) *out = v; return (int)LUBY_E_OK; }
    if (v.type == LUBY_T_FLOAT) { if (out) *out = luby_int((int64_t)v.as.f); return (int)LUBY_E_OK; }
    if (v.type == LUBY_T_STRING && v.as.ptr) { if (out) *out = luby_int(strtoll((const char *)v.as.ptr, NULL, 10)); return (int)LUBY_E_OK; }
    if (out) *out = luby_int(0);
    return (int)LUBY_E_OK;
}

static int luby_base_to_f(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value v = argv[0];
    if (v.type == LUBY_T_FLOAT) { if (out) *out = v; return (int)LUBY_E_OK; }
    if (v.type == LUBY_T_INT) { if (out) *out = luby_float((double)v.as.i); return (int)LUBY_E_OK; }
    if (v.type == LUBY_T_STRING && v.as.ptr) { if (out) *out = luby_float(strtod((const char *)v.as.ptr, NULL)); return (int)LUBY_E_OK; }
    if (out) *out = luby_float(0.0);
    return (int)LUBY_E_OK;
}

static int luby_base_len(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value v = argv[0];
    if (v.type == LUBY_T_STRING && v.as.ptr) {
        if (out) *out = luby_int((int64_t)strlen((const char *)v.as.ptr));
        return (int)LUBY_E_OK;
    }
    if (v.type == LUBY_T_ARRAY && v.as.ptr) {
        luby_array *arr = (luby_array *)v.as.ptr;
        if (out) *out = luby_int((int64_t)arr->count);
        return (int)LUBY_E_OK;
    }
    if (v.type == LUBY_T_HASH && v.as.ptr) {
        luby_hash *h = (luby_hash *)v.as.ptr;
        if (out) *out = luby_int((int64_t)h->count);
        return (int)LUBY_E_OK;
    }
    if (out) *out = luby_int(0);
    return (int)LUBY_E_OK;
}

static int luby_array_push(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (arr->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    luby_value v = argv[1];
    if (arr->count + 1 > arr->capacity) {
        size_t new_cap = arr->capacity < 8 ? 8 : arr->capacity * 2;
        luby_value *ni = (luby_value *)luby_alloc_raw(L, arr->items, new_cap * sizeof(luby_value));
        if (!ni) return (int)LUBY_E_OOM;
        arr->items = ni;
        arr->capacity = new_cap;
    }
    arr->items[arr->count++] = v;
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_array_pop(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (arr->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    if (arr->count == 0) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }
    luby_value v = arr->items[--arr->count];
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

enum {
    LUBY_ENUM_ARRAY = 0,
    LUBY_ENUM_ARRAY_WITH_INDEX = 1,
    LUBY_ENUM_HASH = 2
};

static luby_value luby_enum_new(luby_state *L, luby_value target, int kind) {
    luby_string_view name = { "Enumerator", 10 };
    luby_value cv = luby_get_global(L, name);
    luby_class_obj *cls = NULL;
    if (cv.type == LUBY_T_CLASS && cv.as.ptr) {
        cls = (luby_class_obj *)cv.as.ptr;
    } else {
        cls = luby_class_new(L, "Enumerator", NULL);
        if (!cls) return luby_nil();
        luby_value v; v.type = LUBY_T_CLASS; v.as.ptr = cls;
        luby_set_global(L, name, v);
    }
    luby_object *obj = luby_object_new(L, cls);
    if (!obj) return luby_nil();
    luby_value ov; ov.type = LUBY_T_OBJECT; ov.as.ptr = obj;

    luby_value key_target = luby_symbol(L, "_enum_target", 0);
    luby_value key_index = luby_symbol(L, "_enum_index", 0);
    luby_value key_kind = luby_symbol(L, "_enum_kind", 0);
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_target, target);
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_index, luby_int(0));
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_kind, luby_int(kind));
    return ov;
}

static int luby_enum_get_field(luby_state *L, luby_object *obj, const char *name, luby_value *out) {
    if (!obj || !obj->ivars) return (int)LUBY_E_TYPE;
    luby_value key = luby_symbol(L, name, 0);
    return luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key, out);
}

static int luby_enum_set_field(luby_state *L, luby_object *obj, const char *name, luby_value val) {
    if (!obj || !obj->ivars) return (int)LUBY_E_TYPE;
    luby_value key = luby_symbol(L, name, 0);
    return luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key, val);
}

static luby_value luby_make_pair_array(luby_state *L, luby_value a, luby_value b) {
    luby_value arrv = luby_array_new(L);
    if (arrv.type != LUBY_T_ARRAY || !arrv.as.ptr) return luby_nil();
    luby_array_set(L, arrv, 0, a);
    luby_array_set(L, arrv, 1, b);
    return arrv;
}

static int luby_enum_next(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_OBJECT || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_object *obj = (luby_object *)argv[0].as.ptr;
    luby_value target = luby_nil();
    luby_value indexv = luby_nil();
    luby_value kindv = luby_nil();
    if (luby_enum_get_field(L, obj, "_enum_target", &target) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;
    if (luby_enum_get_field(L, obj, "_enum_index", &indexv) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;
    if (luby_enum_get_field(L, obj, "_enum_kind", &kindv) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;

    if (indexv.type != LUBY_T_INT || kindv.type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    int64_t idx = indexv.as.i;
    int kind = (int)kindv.as.i;

    if (kind == LUBY_ENUM_ARRAY || kind == LUBY_ENUM_ARRAY_WITH_INDEX) {
        if (target.type != LUBY_T_ARRAY || !target.as.ptr) return (int)LUBY_E_TYPE;
        luby_array *arr = (luby_array *)target.as.ptr;
        if (idx < 0 || (size_t)idx >= arr->count) {
            luby_set_error(L, LUBY_E_RUNTIME, "stop iteration", NULL, 0, 0);
            return (int)LUBY_E_RUNTIME;
        }
        luby_value v = arr->items[idx];
        idx++;
        luby_enum_set_field(L, obj, "_enum_index", luby_int(idx));
        if (kind == LUBY_ENUM_ARRAY_WITH_INDEX) {
            if (out) *out = luby_make_pair_array(L, v, luby_int(idx - 1));
        } else {
            if (out) *out = v;
        }
        return (int)LUBY_E_OK;
    }

    if (kind == LUBY_ENUM_HASH) {
        if (target.type != LUBY_T_HASH || !target.as.ptr) return (int)LUBY_E_TYPE;
        luby_hash *h = (luby_hash *)target.as.ptr;
        if (idx < 0 || (size_t)idx >= h->count) {
            luby_set_error(L, LUBY_E_RUNTIME, "stop iteration", NULL, 0, 0);
            return (int)LUBY_E_RUNTIME;
        }
        luby_hash_entry *e = &h->entries[idx];
        idx++;
        luby_enum_set_field(L, obj, "_enum_index", luby_int(idx));
        if (out) *out = luby_make_pair_array(L, e->key, e->value);
        return (int)LUBY_E_OK;
    }

    return (int)LUBY_E_TYPE;
}

static int luby_enum_rewind(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_OBJECT || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_object *obj = (luby_object *)argv[0].as.ptr;
    luby_enum_set_field(L, obj, "_enum_index", luby_int(0));
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_enum_each(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_OBJECT || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) { if (out) *out = argv[0]; return (int)LUBY_E_OK; }

    luby_object *obj = (luby_object *)argv[0].as.ptr;
    luby_value target = luby_nil();
    luby_value indexv = luby_nil();
    luby_value kindv = luby_nil();
    if (luby_enum_get_field(L, obj, "_enum_target", &target) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;
    if (luby_enum_get_field(L, obj, "_enum_index", &indexv) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;
    if (luby_enum_get_field(L, obj, "_enum_kind", &kindv) != (int)LUBY_E_OK) return (int)LUBY_E_TYPE;

    if (indexv.type != LUBY_T_INT || kindv.type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    int64_t idx = indexv.as.i;
    int kind = (int)kindv.as.i;

    if (kind == LUBY_ENUM_ARRAY || kind == LUBY_ENUM_ARRAY_WITH_INDEX) {
        if (target.type != LUBY_T_ARRAY || !target.as.ptr) return (int)LUBY_E_TYPE;
        luby_array *arr = (luby_array *)target.as.ptr;
        for (; idx < (int64_t)arr->count; idx++) {
            luby_value res = luby_nil();
            if (kind == LUBY_ENUM_ARRAY_WITH_INDEX) {
                luby_value args[2];
                args[0] = arr->items[idx];
                args[1] = luby_int(idx);
                if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
            } else {
                if (luby_call_block(L, block, 1, &arr->items[idx], &res) != 0) return (int)LUBY_E_RUNTIME;
            }
        }
        luby_enum_set_field(L, obj, "_enum_index", luby_int(idx));
        if (out) *out = argv[0];
        return (int)LUBY_E_OK;
    }

    if (kind == LUBY_ENUM_HASH) {
        if (target.type != LUBY_T_HASH || !target.as.ptr) return (int)LUBY_E_TYPE;
        luby_hash *h = (luby_hash *)target.as.ptr;
        for (; idx < (int64_t)h->count; idx++) {
            luby_value args[2];
            args[0] = h->entries[idx].key;
            args[1] = h->entries[idx].value;
            luby_value res = luby_nil();
            if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        }
        luby_enum_set_field(L, obj, "_enum_index", luby_int(idx));
        if (out) *out = argv[0];
        return (int)LUBY_E_OK;
    }

    return (int)LUBY_E_TYPE;
}

static int luby_coroutine_new_cfunc(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    luby_proc *proc = NULL;
    if (argc >= 1 && argv[0].type == LUBY_T_PROC) proc = (luby_proc *)argv[0].as.ptr;
    else if (L->current_block.type == LUBY_T_PROC) proc = (luby_proc *)L->current_block.as.ptr;
    if (!proc) return (int)LUBY_E_TYPE;

    luby_coroutine *co = luby_coroutine_new(L, (luby_value){ .type = LUBY_T_PROC, .as.ptr = proc });
    if (!co) return (int)LUBY_E_OOM;

    luby_string_view name = { "Coroutine", 9 };
    luby_value cv = luby_get_global(L, name);
    luby_class_obj *cls = NULL;
    if (cv.type == LUBY_T_CLASS && cv.as.ptr) {
        cls = (luby_class_obj *)cv.as.ptr;
    } else {
        cls = luby_class_new(L, "Coroutine", NULL);
        if (!cls) return (int)LUBY_E_OOM;
        luby_value v; v.type = LUBY_T_CLASS; v.as.ptr = cls;
        luby_set_global(L, name, v);
    }
    luby_object *obj = luby_object_new(L, cls);
    if (!obj) return (int)LUBY_E_OOM;
    luby_value ov; ov.type = LUBY_T_OBJECT; ov.as.ptr = obj;

    luby_value key_ptr = luby_symbol(L, "_co_ptr", 0);
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_ptr, luby_int((int64_t)(intptr_t)co));
    if (out) *out = ov;
    return (int)LUBY_E_OK;
}

static int luby_coroutine_resume_cfunc(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_OBJECT || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_object *obj = (luby_object *)argv[0].as.ptr;
    luby_value key_ptr = luby_symbol(L, "_co_ptr", 0);
    luby_value pv = luby_nil();
    luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_ptr, &pv);
    if (pv.type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_coroutine *co = (luby_coroutine *)(intptr_t)pv.as.i;
    if (!co) return (int)LUBY_E_TYPE;

    luby_value rv = luby_nil();
    int yielded = 0;
    int rc = luby_coroutine_resume(L, co, argc - 1, argv + 1, &rv, &yielded);
    if (out) *out = rv;
    return rc;
}

static int luby_coroutine_alive_cfunc(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_OBJECT || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_object *obj = (luby_object *)argv[0].as.ptr;
    luby_value key_ptr = luby_symbol(L, "_co_ptr", 0);
    luby_value pv = luby_nil();
    luby_hash_get_value((luby_value){ .type = LUBY_T_HASH, .as.ptr = obj->ivars }, key_ptr, &pv);
    luby_coroutine *co = (pv.type == LUBY_T_INT) ? (luby_coroutine *)(intptr_t)pv.as.i : NULL;
    int alive = (co && !co->done);
    if (out) *out = luby_bool(alive);
    return (int)LUBY_E_OK;
}

static int luby_array_map(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    const char *fname = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) fname = (const char *)argv[1].as.ptr;
    luby_cfunc fn = fname ? luby_find_cfunc(L, fname) : NULL;
    if (!block && !fn) return (int)LUBY_E_TYPE;

    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0;
    dst->capacity = src->count;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    if (!dst->items && dst->capacity > 0) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (block) {
            if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        } else {
            if (fn(L, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        }
        dst->items[dst->count++] = res;
    }
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_array_select(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    const char *fname = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) fname = (const char *)argv[1].as.ptr;
    luby_cfunc fn = fname ? luby_find_cfunc(L, fname) : NULL;
    if (!block && !fn) return (int)LUBY_E_TYPE;

    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0;
    dst->capacity = src->count;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    if (!dst->items && dst->capacity > 0) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (block) {
            if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        } else {
            if (fn(L, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        }
        if (luby_is_truthy(res)) dst->items[dst->count++] = src->items[i];
    }
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_array_reject(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    const char *fname = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) fname = (const char *)argv[1].as.ptr;
    luby_cfunc fn = fname ? luby_find_cfunc(L, fname) : NULL;
    if (!block && !fn) return (int)LUBY_E_TYPE;

    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0;
    dst->capacity = src->count;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    dst->frozen = 0;
    if (!dst->items && dst->capacity > 0) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (block) {
            if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        } else {
            if (fn(L, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        }
        if (!luby_is_truthy(res)) dst->items[dst->count++] = src->items[i];
    }
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_range_each(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_RANGE || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) {
        if (out) *out = argv[0];
        return (int)LUBY_E_OK;
    }

    luby_range *range = (luby_range *)argv[0].as.ptr;
    if (range->start.type != LUBY_T_INT || range->end.type != LUBY_T_INT) {
        return (int)LUBY_E_TYPE;
    }
    
    int64_t start = range->start.as.i;
    int64_t end = range->end.as.i;
    if (range->exclusive) end--;
    
    for (int64_t i = start; i <= end; i++) {
        luby_value iv = luby_int(i);
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &iv, &res) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_generic_each(luby_state *L, int argc, const luby_value *argv, luby_value *out);

static int luby_array_each(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    const char *fname = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) fname = (const char *)argv[1].as.ptr;
    luby_cfunc fn = fname ? luby_find_cfunc(L, fname) : NULL;
    if (!block && !fn) {
        if (out) *out = luby_enum_new(L, argv[0], LUBY_ENUM_ARRAY);
        return (int)LUBY_E_OK;
    }

    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (block) {
            if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        } else {
            if (fn(L, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        }
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_array_each_with_index(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    const char *fname = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) fname = (const char *)argv[1].as.ptr;
    luby_cfunc fn = fname ? luby_find_cfunc(L, fname) : NULL;
    if (!block && !fn) {
        if (out) *out = luby_enum_new(L, argv[0], LUBY_ENUM_ARRAY_WITH_INDEX);
        return (int)LUBY_E_OK;
    }

    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value args[2];
        args[0] = src->items[i];
        args[1] = luby_int((int64_t)i);
        luby_value res = luby_nil();
        if (block) {
            if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        } else {
            if (fn(L, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        }
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_array_compact(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0;
    dst->capacity = src->count;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    dst->frozen = 0;
    if (!dst->items && dst->capacity > 0) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < src->count; i++) {
        if (src->items[i].type != LUBY_T_NIL) {
            dst->items[dst->count++] = src->items[i];
        }
    }
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_array_compact_bang(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (arr->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    size_t write = 0;
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->items[i].type != LUBY_T_NIL) {
            arr->items[write++] = arr->items[i];
        }
    }
    arr->count = write;
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_array_reduce(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;

    luby_array *src = (luby_array *)argv[0].as.ptr;
    if (src->count == 0) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }

    size_t i = 0;
    luby_value acc = luby_nil();
    if (argc >= 2) {
        acc = argv[1];
    } else {
        acc = src->items[0];
        i = 1;
    }

    for (; i < src->count; i++) {
        luby_value args[2];
        args[0] = acc;
        args[1] = src->items[i];
        if (luby_call_block(L, block, 2, args, &acc) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = acc;
    return (int)LUBY_E_OK;
}

static int luby_array_any(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = luby_bool(1); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(0);
    return (int)LUBY_E_OK;
}

static int luby_array_all(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        if (!luby_is_truthy(res)) { if (out) *out = luby_bool(0); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(1);
    return (int)LUBY_E_OK;
}

static int luby_array_none(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = luby_bool(0); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(1);
    return (int)LUBY_E_OK;
}

static int luby_array_find(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < src->count; i++) {
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &src->items[i], &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = src->items[i]; return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_hash_get(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        if (luby_value_eq(h->entries[i].key, argv[1])) {
            if (out) *out = h->entries[i].value;
            return (int)LUBY_E_OK;
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_hash_set(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 3 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    if (h->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    for (size_t i = 0; i < h->count; i++) {
        if (luby_value_eq(h->entries[i].key, argv[1])) {
            h->entries[i].value = argv[2];
            if (out) *out = argv[0];
            return (int)LUBY_E_OK;
        }
    }
    if (h->count + 1 > h->capacity) {
        size_t new_cap = h->capacity < 8 ? 8 : h->capacity * 2;
        luby_hash_entry *ne = (luby_hash_entry *)luby_alloc_raw(L, h->entries, new_cap * sizeof(luby_hash_entry));
        if (!ne) return (int)LUBY_E_OOM;
        h->entries = ne;
        h->capacity = new_cap;
    }
    h->entries[h->count].key = argv[1];
    h->entries[h->count].value = argv[2];
    h->count++;
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_hash_merge(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    if (argv[1].type != LUBY_T_HASH || !argv[1].as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *a = (luby_hash *)argv[0].as.ptr;
    luby_hash *b = (luby_hash *)argv[1].as.ptr;
    luby_hash *dst = luby_hash_new_heap(L);
    if (!dst) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < a->count; i++) {
        luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = dst }, a->entries[i].key, a->entries[i].value);
    }
    for (size_t i = 0; i < b->count; i++) {
        luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = dst }, b->entries[i].key, b->entries[i].value);
    }
    luby_value v; v.type = LUBY_T_HASH; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_hash_each(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) {
        if (out) *out = luby_enum_new(L, argv[0], LUBY_ENUM_HASH);
        return (int)LUBY_E_OK;
    }
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_generic_each(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    switch (argv[0].type) {
        case LUBY_T_ARRAY: return luby_array_each(L, argc, argv, out);
        case LUBY_T_HASH: return luby_hash_each(L, argc, argv, out);
        case LUBY_T_RANGE: return luby_range_each(L, argc, argv, out);
        default: return (int)LUBY_E_TYPE;
    }
}

static int luby_hash_map(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0;
    dst->capacity = h->count;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    if (!dst->items && dst->capacity > 0) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        dst->items[dst->count++] = res;
    }
    luby_value v; v.type = LUBY_T_ARRAY; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_hash_select(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    luby_hash *dst = luby_hash_new_heap(L);
    if (!dst) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) {
            luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = dst }, h->entries[i].key, h->entries[i].value);
        }
    }
    luby_value v; v.type = LUBY_T_HASH; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_hash_reject(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    luby_hash *dst = luby_hash_new_heap(L);
    if (!dst) return (int)LUBY_E_OOM;

    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (!luby_is_truthy(res)) {
            luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = dst }, h->entries[i].key, h->entries[i].value);
        }
    }
    luby_value v; v.type = LUBY_T_HASH; v.as.ptr = dst;
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_hash_any(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = luby_bool(1); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(0);
    return (int)LUBY_E_OK;
}

static int luby_hash_all(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (!luby_is_truthy(res)) { if (out) *out = luby_bool(0); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(1);
    return (int)LUBY_E_OK;
}

static int luby_hash_none(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = luby_bool(0); return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_bool(1);
    return (int)LUBY_E_OK;
}

static int luby_hash_find(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    for (size_t i = 0; i < h->count; i++) {
        luby_value args[2];
        args[0] = h->entries[i].key;
        args[1] = h->entries[i].value;
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 2, args, &res) != 0) return (int)LUBY_E_RUNTIME;
        if (luby_is_truthy(res)) { if (out) *out = h->entries[i].key; return (int)LUBY_E_OK; }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_hash_reduce(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    if (h->count == 0) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }

    size_t i = 0;
    luby_value acc = luby_nil();
    if (argc >= 2) {
        acc = argv[1];
    } else {
        acc = h->entries[0].value;
        i = 1;
    }

    for (; i < h->count; i++) {
        luby_value args[3];
        args[0] = acc;
        args[1] = h->entries[i].key;
        args[2] = h->entries[i].value;
        if (luby_call_block(L, block, 3, args, &acc) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = acc;
    return (int)LUBY_E_OK;
}

static int luby_base_dig(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }
    luby_value cur = argv[0];
    for (int i = 1; i < argc; i++) {
        if (cur.type == LUBY_T_NIL) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }
        luby_value key = argv[i];
        if (cur.type == LUBY_T_ARRAY && cur.as.ptr && key.type == LUBY_T_INT) {
            luby_array *arr = (luby_array *)cur.as.ptr;
            int64_t idx = key.as.i;
            if (idx < 0 || (size_t)idx >= arr->count) { if (out) *out = luby_nil(); return (int)LUBY_E_OK; }
            cur = arr->items[idx];
            continue;
        }
        if (cur.type == LUBY_T_HASH && cur.as.ptr) {
            luby_value v = luby_nil();
            int found = 0;
            if (luby_hash_get_value_found(cur, key, &v, &found) == (int)LUBY_E_OK && found) {
                cur = v;
                continue;
            }
            if (out) *out = luby_nil();
            return (int)LUBY_E_OK;
        }
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    if (out) *out = cur;
    return (int)LUBY_E_OK;
}

static int luby_base_respond_to(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2) return (int)LUBY_E_TYPE;
    luby_value recv = argv[0];
    const char *name = NULL;
    if (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL) {
        name = (const char *)argv[1].as.ptr;
    }
    int ok = 0;
    if (name) {
        if (recv.type == LUBY_T_OBJECT && recv.as.ptr) {
            luby_class_obj *cls = ((luby_object *)recv.as.ptr)->klass;
            ok = luby_class_has_method(L, cls, name);
        } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
            ok = luby_class_has_method(L, (luby_class_obj *)recv.as.ptr, name);
        } else {
            ok = luby_find_cfunc(L, name) != NULL;
            if (!ok) {
                luby_string_view sv = { name, strlen(name) };
                luby_value gv = luby_get_global(L, sv);
                ok = (gv.type == LUBY_T_PROC);
            }
        }
    }
    if (!ok && name && (recv.type == LUBY_T_OBJECT || recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE)) {
        luby_class_obj *cls = (recv.type == LUBY_T_OBJECT && recv.as.ptr) ? ((luby_object *)recv.as.ptr)->klass : (luby_class_obj *)recv.as.ptr;
        if (cls) {
            luby_proc *m = luby_class_get_method(L, cls, "respond_to_missing?");
            if (m) {
                luby_value arg; arg.type = LUBY_T_SYMBOL; arg.as.ptr = (void *)name;
                luby_value res = luby_nil();
                if (luby_call_method(L, cls, "respond_to_missing?", m, recv, 1, &arg, &res) == 0) {
                    ok = luby_is_truthy(res);
                }
            }
        }
    }
    if (out) *out = luby_bool(ok);
    return (int)LUBY_E_OK;
}

// is_a? and kind_of? check if object is instance of class or its ancestors
static int luby_base_is_a(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    luby_value obj = argv[0];
    luby_value klass = argv[1];
    
    if (klass.type != LUBY_T_CLASS && klass.type != LUBY_T_MODULE) {
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }
    
    luby_class_obj *target_class = (luby_class_obj *)klass.as.ptr;
    luby_class_obj *obj_class = NULL;
    
    // Get the class of the object
    if (obj.type == LUBY_T_OBJECT && obj.as.ptr) {
        obj_class = ((luby_object *)obj.as.ptr)->klass;
    } else if (obj.type == LUBY_T_CLASS || obj.type == LUBY_T_MODULE) {
        obj_class = (luby_class_obj *)obj.as.ptr;
    }
    
    // Check if obj_class is target_class or includes it
    int is_match = 0;
    if (obj_class && target_class) {
        // Check direct match or superclass chain
        luby_class_obj *current = obj_class;
        while (current) {
            if (current == target_class) {
                is_match = 1;
                break;
            }
            // Check included modules
            for (size_t i = 0; i < current->included_count; i++) {
                if (current->included_modules[i] == target_class) {
                    is_match = 1;
                    break;
                }
            }
            if (is_match) break;
            current = current->super;
        }
    }
    
    if (out) *out = luby_bool(is_match);
    return (int)LUBY_E_OK;
}

// instance_of? checks if object is exactly an instance of class (no inheritance)
static int luby_base_instance_of(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    luby_value obj = argv[0];
    luby_value klass = argv[1];
    
    if (klass.type != LUBY_T_CLASS) {
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }
    
    luby_class_obj *target_class = (luby_class_obj *)klass.as.ptr;
    int is_match = 0;
    
    if (obj.type == LUBY_T_OBJECT && obj.as.ptr) {
        luby_object *o = (luby_object *)obj.as.ptr;
        is_match = (o->klass == target_class);
    }
    
    if (out) *out = luby_bool(is_match);
    return (int)LUBY_E_OK;
}

// defined? checks if a variable, method, or constant is defined
static int luby_base_defined(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    
    // The argument is typically a symbol or string representing what to check
    const char *name = NULL;
    if (argv[0].type == LUBY_T_STRING || argv[0].type == LUBY_T_SYMBOL) {
        name = (const char *)argv[0].as.ptr;
    }
    
    if (!name) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    
    // Check if it's a global variable
    luby_string_view sv = { name, strlen(name) };
    luby_value gv = luby_get_global(L, sv);
    if (gv.type != LUBY_T_NIL) {
        if (out) *out = luby_string(L, "global-variable", 0);
        return (int)LUBY_E_OK;
    }
    
    // Check if it's a method/function
    if (luby_find_cfunc(L, name) != NULL) {
        if (out) *out = luby_string(L, "method", 0);
        return (int)LUBY_E_OK;
    }
    
    // Check if it's a constant (starts with uppercase)
    if (name[0] >= 'A' && name[0] <= 'Z') {
        if (gv.type != LUBY_T_NIL) {
            if (out) *out = luby_string(L, "constant", 0);
            return (int)LUBY_E_OK;
        }
    }
    
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_freeze(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value v = argv[0];
    switch (v.type) {
        case LUBY_T_ARRAY:
            if (v.as.ptr) ((luby_array *)v.as.ptr)->frozen = 1;
            break;
        case LUBY_T_HASH:
            if (v.as.ptr) ((luby_hash *)v.as.ptr)->frozen = 1;
            break;
        case LUBY_T_OBJECT:
            if (v.as.ptr) ((luby_object *)v.as.ptr)->frozen = 1;
            break;
        case LUBY_T_CLASS:
        case LUBY_T_MODULE:
            if (v.as.ptr) ((luby_class_obj *)v.as.ptr)->frozen = 1;
            break;
        default:
            break;
    }
    if (out) *out = v;
    return (int)LUBY_E_OK;
}

static int luby_base_frozen(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    int frz = luby_value_is_frozen(argv[0]);
    if (out) *out = luby_bool(frz);
    return (int)LUBY_E_OK;
}

static int luby_base_require(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || (argv[0].type != LUBY_T_STRING && argv[0].type != LUBY_T_SYMBOL)) return (int)LUBY_E_TYPE;
    const char *path = (const char *)argv[0].as.ptr;
    if (!path) return (int)LUBY_E_TYPE;
    return luby_require(L, path, out);
}

static int luby_base_load(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || (argv[0].type != LUBY_T_STRING && argv[0].type != LUBY_T_SYMBOL)) return (int)LUBY_E_TYPE;
    const char *path = (const char *)argv[0].as.ptr;
    if (!path) return (int)LUBY_E_TYPE;
    return luby_load(L, path, out);
}

static int luby_base_yield(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    return luby_yield(L, argc, argv, out);
}

static int luby_base_send(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2) return (int)LUBY_E_TYPE;
    luby_value recv = argv[0];
    const char *name = NULL;
    if (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL) {
        name = (const char *)argv[1].as.ptr;
    }
    if (!name) return (int)LUBY_E_TYPE;
    return luby_call_method_by_name(L, recv, name, argc - 2, argv + 2, out);
}

static int luby_base_public_send(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    return luby_base_send(L, argc, argv, out);
}

static int luby_base_define_method(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    const char *name = NULL;
    if (argv[0].type == LUBY_T_STRING || argv[0].type == LUBY_T_SYMBOL) {
        name = (const char *)argv[0].as.ptr;
    }
    if (!name) return (int)LUBY_E_TYPE;
    if (!(L->current_class.type == LUBY_T_CLASS || L->current_class.type == LUBY_T_MODULE)) return (int)LUBY_E_TYPE;
    luby_class_obj *cls = (luby_class_obj *)L->current_class.as.ptr;
    if (!cls) return (int)LUBY_E_TYPE;
    if (cls->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }

    luby_proc *proc = NULL;
    if (argc >= 2 && argv[1].type == LUBY_T_PROC) proc = (luby_proc *)argv[1].as.ptr;
    else if (L->current_block.type == LUBY_T_PROC) proc = (luby_proc *)L->current_block.as.ptr;
    if (!proc) return (int)LUBY_E_TYPE;

    luby_class_set_method(L, cls, name, proc);
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_define_singleton_method(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value target = L->current_self;
    const char *name = NULL;

    if (argc >= 2 && (argv[0].type == LUBY_T_OBJECT || argv[0].type == LUBY_T_CLASS || argv[0].type == LUBY_T_MODULE)) {
        target = argv[0];
        if (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL) {
            name = (const char *)argv[1].as.ptr;
        }
    } else {
        if (argv[0].type == LUBY_T_STRING || argv[0].type == LUBY_T_SYMBOL) {
            name = (const char *)argv[0].as.ptr;
        }
    }

    if (!name) return (int)LUBY_E_TYPE;
    if (!(target.type == LUBY_T_OBJECT || target.type == LUBY_T_CLASS || target.type == LUBY_T_MODULE)) return (int)LUBY_E_TYPE;
    if (target.type == LUBY_T_OBJECT && target.as.ptr && ((luby_object *)target.as.ptr)->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }
    if ((target.type == LUBY_T_CLASS || target.type == LUBY_T_MODULE) && target.as.ptr && ((luby_class_obj *)target.as.ptr)->frozen) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }

    luby_proc *proc = NULL;
    if (argc >= 3 && argv[2].type == LUBY_T_PROC) proc = (luby_proc *)argv[2].as.ptr;
    else if (argc >= 2 && argv[1].type == LUBY_T_PROC) proc = (luby_proc *)argv[1].as.ptr;
    else if (L->current_block.type == LUBY_T_PROC) proc = (luby_proc *)L->current_block.as.ptr;
    if (!proc) return (int)LUBY_E_TYPE;

    if (target.type == LUBY_T_OBJECT && target.as.ptr) {
        luby_object_set_singleton_method(L, (luby_object *)target.as.ptr, name, proc);
    } else {
        luby_class_set_singleton_method(L, (luby_class_obj *)target.as.ptr, name, proc);
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_class_eval(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value target = argv[0];
    if (!(target.type == LUBY_T_CLASS || target.type == LUBY_T_MODULE)) return (int)LUBY_E_TYPE;

    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) {
        const char *code = (const char *)argv[1].as.ptr;
        if (!code) return (int)LUBY_E_TYPE;
        return luby_eval_with_context(L, target, target, code, 0, "<class_eval>", out);
    }

    if (L->current_block.type == LUBY_T_PROC) {
        luby_value saved_class = L->current_class;
        L->current_class = target;
        int rc = luby_call_proc_with_self(L, (luby_proc *)L->current_block.as.ptr, target, 0, NULL, out);
        L->current_class = saved_class;
        return rc;
    }

    return (int)LUBY_E_TYPE;
}

static int luby_base_instance_eval(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value target = argv[0];
    if (!(target.type == LUBY_T_OBJECT || target.type == LUBY_T_CLASS || target.type == LUBY_T_MODULE)) return (int)LUBY_E_TYPE;

    luby_value target_class = luby_nil();
    if (target.type == LUBY_T_OBJECT && target.as.ptr) {
        target_class.type = LUBY_T_CLASS;
        target_class.as.ptr = ((luby_object *)target.as.ptr)->klass;
    } else if (target.type == LUBY_T_CLASS || target.type == LUBY_T_MODULE) {
        target_class = target;
    }

    if (argc >= 2 && (argv[1].type == LUBY_T_STRING || argv[1].type == LUBY_T_SYMBOL)) {
        const char *code = (const char *)argv[1].as.ptr;
        if (!code) return (int)LUBY_E_TYPE;
        return luby_eval_with_context(L, target_class, target, code, 0, "<instance_eval>", out);
    }

    if (L->current_block.type == LUBY_T_PROC) {
        luby_value saved_class = L->current_class;
        L->current_class = target_class;
        int rc = luby_call_proc_with_self(L, (luby_proc *)L->current_block.as.ptr, target, 0, NULL, out);
        L->current_class = saved_class;
        return rc;
    }

    return (int)LUBY_E_TYPE;
}

static int luby_base_include(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value modv = argv[0];
    if (modv.type != LUBY_T_MODULE && modv.type != LUBY_T_CLASS) return (int)LUBY_E_TYPE;
    luby_class_obj *mod = (luby_class_obj *)modv.as.ptr;
    luby_class_obj *target = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_CLASS || argv[1].type == LUBY_T_MODULE)) {
        target = (luby_class_obj *)argv[1].as.ptr;
    } else if (L->current_class.type == LUBY_T_CLASS || L->current_class.type == LUBY_T_MODULE) {
        target = (luby_class_obj *)L->current_class.as.ptr;
    }
    if (!target) return (int)LUBY_E_TYPE;
    if (target->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    if (!luby_class_add_include(L, target, mod)) return (int)LUBY_E_RUNTIME;
    {
        luby_value recv; recv.type = modv.type; recv.as.ptr = mod;
        luby_value arg; arg.type = (target == mod) ? modv.type : LUBY_T_CLASS; arg.as.ptr = target;
        int hook_rc = luby_call_hook_if_exists(L, recv, "included", arg);
        if (hook_rc == (int)LUBY_E_RUNTIME) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_prepend(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    luby_value modv = argv[0];
    if (modv.type != LUBY_T_MODULE && modv.type != LUBY_T_CLASS) return (int)LUBY_E_TYPE;
    luby_class_obj *mod = (luby_class_obj *)modv.as.ptr;
    luby_class_obj *target = NULL;
    if (argc >= 2 && (argv[1].type == LUBY_T_CLASS || argv[1].type == LUBY_T_MODULE)) {
        target = (luby_class_obj *)argv[1].as.ptr;
    } else if (L->current_class.type == LUBY_T_CLASS || L->current_class.type == LUBY_T_MODULE) {
        target = (luby_class_obj *)L->current_class.as.ptr;
    }
    if (!target) return (int)LUBY_E_TYPE;
    if (target->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    if (!luby_class_add_prepend(L, target, mod)) return (int)LUBY_E_RUNTIME;
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_extend(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    luby_value recv = luby_nil();
    luby_value modv = luby_nil();
    if (argc == 1) {
        recv = L->current_self;
        modv = argv[0];
    } else if (argc >= 2) {
        recv = argv[0];
        modv = argv[1];
    } else {
        return (int)LUBY_E_TYPE;
    }
    if (modv.type != LUBY_T_MODULE && modv.type != LUBY_T_CLASS) return (int)LUBY_E_TYPE;
    luby_class_obj *mod = (luby_class_obj *)modv.as.ptr;
    luby_class_obj *target = NULL;
    if (recv.type == LUBY_T_OBJECT && recv.as.ptr) {
        target = ((luby_object *)recv.as.ptr)->klass;
    } else if (recv.type == LUBY_T_CLASS || recv.type == LUBY_T_MODULE) {
        target = (luby_class_obj *)recv.as.ptr;
    }
    if (!target) return (int)LUBY_E_TYPE;
    if (target->frozen) { if (L) luby_set_error(L, LUBY_E_RUNTIME, "frozen", NULL, 0, 0); return (int)LUBY_E_RUNTIME; }
    if (!luby_class_merge_methods(L, target, mod)) return (int)LUBY_E_RUNTIME;
    if (out) *out = recv;
    return (int)LUBY_E_OK;
}

// attr_reader: defines a getter method for instance variable
// attr_reader(:name) generates: def name; @name; end
static int luby_base_attr_reader(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
        return (int)LUBY_E_RUNTIME;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i].type != LUBY_T_SYMBOL || !argv[i].as.ptr) continue;
        const char *name = (const char *)argv[i].as.ptr;
        // Generate: def name; @name; end
        char code[128];
        snprintf(code, sizeof(code), "def %s; @%s; end", name, name);
        luby_value dummy;
        luby_eval(L, code, 0, "<attr_reader>", &dummy);
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// attr_writer: defines a setter method for instance variable
// attr_writer(:name) generates: def name=(v); @name = v; end
static int luby_base_attr_writer(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
        return (int)LUBY_E_RUNTIME;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i].type != LUBY_T_SYMBOL || !argv[i].as.ptr) continue;
        const char *name = (const char *)argv[i].as.ptr;
        // Generate: def name=(v); @name = v; end
        char code[128];
        snprintf(code, sizeof(code), "def %s=(v); @%s = v; end", name, name);
        luby_value dummy;
        luby_eval(L, code, 0, "<attr_writer>", &dummy);
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// attr_accessor: defines both getter and setter
static int luby_base_attr_accessor(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    luby_base_attr_reader(L, argc, argv, out);
    luby_base_attr_writer(L, argc, argv, out);
    return (int)LUBY_E_OK;
}

// private: sets visibility of subsequent methods to private
static int luby_base_private(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc == 0) {
        // No arguments: change visibility mode for subsequent methods
        L->current_visibility = LUBY_VIS_PRIVATE;
    } else {
        // With arguments: change visibility of specific methods
        if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
            return (int)LUBY_E_RUNTIME;
        }
        luby_class_obj *cls = (luby_class_obj *)L->current_class.as.ptr;
        for (int i = 0; i < argc; i++) {
            if (argv[i].type != LUBY_T_SYMBOL || !argv[i].as.ptr) continue;
            const char *name = (const char *)argv[i].as.ptr;
            luby_proc *proc = luby_class_get_method(L, cls, name);
            if (proc) {
                proc->visibility = LUBY_VIS_PRIVATE;
                L->method_epoch++;  // Invalidate method cache
            }
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// public: sets visibility of subsequent methods to public
static int luby_base_public(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc == 0) {
        // No arguments: change visibility mode for subsequent methods
        L->current_visibility = LUBY_VIS_PUBLIC;
    } else {
        // With arguments: change visibility of specific methods
        if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
            return (int)LUBY_E_RUNTIME;
        }
        luby_class_obj *cls = (luby_class_obj *)L->current_class.as.ptr;
        for (int i = 0; i < argc; i++) {
            if (argv[i].type != LUBY_T_SYMBOL || !argv[i].as.ptr) continue;
            const char *name = (const char *)argv[i].as.ptr;
            luby_proc *proc = luby_class_get_method(L, cls, name);
            if (proc) {
                proc->visibility = LUBY_VIS_PUBLIC;
                L->method_epoch++;  // Invalidate method cache
            }
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// protected: sets visibility of subsequent methods to protected
static int luby_base_protected(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc == 0) {
        // No arguments: change visibility mode for subsequent methods
        L->current_visibility = LUBY_VIS_PROTECTED;
    } else {
        // With arguments: change visibility of specific methods
        if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
            return (int)LUBY_E_RUNTIME;
        }
        luby_class_obj *cls = (luby_class_obj *)L->current_class.as.ptr;
        for (int i = 0; i < argc; i++) {
            if (argv[i].type != LUBY_T_SYMBOL || !argv[i].as.ptr) continue;
            const char *name = (const char *)argv[i].as.ptr;
            luby_proc *proc = luby_class_get_method(L, cls, name);
            if (proc) {
                proc->visibility = LUBY_VIS_PROTECTED;
                L->method_epoch++;  // Invalidate method cache
            }
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// alias: creates an alias for a method
// Usage: alias new_name old_name
static int luby_base_alias(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    const char *new_name = NULL;
    const char *old_name = NULL;
    luby_class_obj *cls;
    luby_proc *old_proc;
    char msg[256];
    
    if (argc < 2) {
        luby_set_error(L, LUBY_E_TYPE, "alias requires 2 arguments", NULL, 0, 0);
        return (int)LUBY_E_TYPE;
    }
    if (L->current_class.type != LUBY_T_CLASS && L->current_class.type != LUBY_T_MODULE) {
        luby_set_error(L, LUBY_E_RUNTIME, "alias must be called in class/module context", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }
    
    // Convert arguments to symbols/strings
    if (argv[0].type == LUBY_T_SYMBOL || argv[0].type == LUBY_T_STRING) {
        new_name = (const char *)argv[0].as.ptr;
    }
    
    if (argv[1].type == LUBY_T_SYMBOL || argv[1].type == LUBY_T_STRING) {
        old_name = (const char *)argv[1].as.ptr;
    }
    
    if (!new_name || !old_name) {
        luby_set_error(L, LUBY_E_TYPE, "alias arguments must be symbols or strings", NULL, 0, 0);
        return (int)LUBY_E_TYPE;
    }
    
    cls = (luby_class_obj *)L->current_class.as.ptr;
    old_proc = luby_class_get_method(L, cls, old_name);
    
    if (!old_proc) {
        snprintf(msg, sizeof(msg), "undefined method '%s' for class", old_name);
        luby_set_error(L, LUBY_E_NAME, msg, NULL, 0, 0);
        return (int)LUBY_E_NAME;
    }
    
    // Copy the method to the new name
    luby_class_set_method(L, cls, new_name, old_proc);
    
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

// ---------------------- Additional stdlib functions -------------------------

static int luby_base_to_s(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) return (int)LUBY_E_TYPE;
    char *str = luby_value_to_string(L, argv[0]);
    if (out) *out = luby_string(L, str, 0);
    if (str) luby_alloc_raw(L, str, 0);
    return (int)LUBY_E_OK;
}

static int luby_base_is_nil(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) { if (out) *out = luby_bool(1); return (int)LUBY_E_OK; }
    if (out) *out = luby_bool(argv[0].type == LUBY_T_NIL);
    return (int)LUBY_E_OK;
}

static int luby_str_upcase(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    const char *src = (const char *)argv[0].as.ptr;
    size_t len = strlen(src);
    // Pause GC - src points to argv which may not be rooted during alloc
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    char *dst = luby_gc_alloc_string(L, src, len);
    if (!dst) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    for (size_t i = 0; i < len; i++) {
        dst[i] = (char)((src[i] >= 'a' && src[i] <= 'z') ? src[i] - 32 : src[i]);
    }
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_STRING; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_str_downcase(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    const char *src = (const char *)argv[0].as.ptr;
    size_t len = strlen(src);
    // Pause GC - src points to argv which may not be rooted during alloc
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    char *dst = luby_gc_alloc_string(L, src, len);
    if (!dst) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    for (size_t i = 0; i < len; i++) {
        dst[i] = (char)((src[i] >= 'A' && src[i] <= 'Z') ? src[i] + 32 : src[i]);
    }
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_STRING; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_str_split(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    const char *src = (const char *)argv[0].as.ptr;
    const char *delim = (argv[1].type == LUBY_T_STRING && argv[1].as.ptr) ? (const char *)argv[1].as.ptr : " ";
    size_t delim_len = strlen(delim);
    
    // Pause GC - src and delim point to argv which may not be rooted
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    
    luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!arr) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    arr->count = 0; arr->capacity = 4; arr->frozen = 0;
    arr->items = (luby_value *)luby_alloc_raw(L, NULL, arr->capacity * sizeof(luby_value));
    
    const char *p = src;
    while (*p) {
        const char *found = delim_len > 0 ? strstr(p, delim) : NULL;
        size_t part_len = found ? (size_t)(found - p) : strlen(p);
        char *part = luby_gc_alloc_string(L, p, part_len);
        
        if (arr->count >= arr->capacity) {
            arr->capacity *= 2;
            arr->items = (luby_value *)luby_alloc_raw(L, arr->items, arr->capacity * sizeof(luby_value));
        }
        arr->items[arr->count].type = LUBY_T_STRING;
        arr->items[arr->count].as.ptr = part;
        arr->count++;
        
        if (!found) break;
        p = found + delim_len;
    }
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = arr; }
    return (int)LUBY_E_OK;
}

static int luby_str_join(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    const char *sep = (argc >= 2 && argv[1].type == LUBY_T_STRING && argv[1].as.ptr) ? (const char *)argv[1].as.ptr : "";
    size_t sep_len = strlen(sep);
    
    size_t total = 0;
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->items[i].type == LUBY_T_STRING && arr->items[i].as.ptr)
            total += strlen((const char *)arr->items[i].as.ptr);
        if (i > 0) total += sep_len;
    }
    
    // Pause GC - arr and sep point to argv which may not be rooted
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    
    char *result = luby_gc_alloc_string(L, NULL, total);
    if (!result) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    char *p = result;
    for (size_t i = 0; i < arr->count; i++) {
        if (i > 0) { memcpy(p, sep, sep_len); p += sep_len; }
        if (arr->items[i].type == LUBY_T_STRING && arr->items[i].as.ptr) {
            size_t len = strlen((const char *)arr->items[i].as.ptr);
            memcpy(p, arr->items[i].as.ptr, len);
            p += len;
        }
    }
    *p = '\0';
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_STRING; out->as.ptr = result; }
    return (int)LUBY_E_OK;
}

static int luby_array_reverse(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    // Pause GC - src/argv may not be rooted during allocation
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    /* String reverse */
    if (argv[0].type == LUBY_T_STRING || argv[0].type == LUBY_T_SYMBOL) {
        const char *src = (const char *)argv[0].as.ptr;
        size_t slen = strlen(src);
        char *dst = luby_gc_alloc_string(L, NULL, slen);
        if (!dst) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
        for (size_t i = 0; i < slen; i++) {
            dst[i] = src[slen - 1 - i];
        }
        L->gc_paused = was_paused;
        if (out) { out->type = LUBY_T_STRING; out->as.ptr = dst; }
        return (int)LUBY_E_OK;
    }
    /* Array reverse */
    if (argv[0].type != LUBY_T_ARRAY) { L->gc_paused = was_paused; return (int)LUBY_E_TYPE; }
    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    dst->count = src->count; dst->capacity = src->count; dst->frozen = 0;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    for (size_t i = 0; i < src->count; i++) {
        dst->items[i] = src->items[src->count - 1 - i];
    }
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_array_first(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (out) *out = (arr->count > 0) ? arr->items[0] : luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_array_last(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (out) *out = (arr->count > 0) ? arr->items[arr->count - 1] : luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_array_flatten(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0; dst->capacity = src->count * 2; dst->frozen = 0;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    
    for (size_t i = 0; i < src->count; i++) {
        if (src->items[i].type == LUBY_T_ARRAY && src->items[i].as.ptr) {
            luby_array *inner = (luby_array *)src->items[i].as.ptr;
            for (size_t j = 0; j < inner->count; j++) {
                if (dst->count >= dst->capacity) {
                    dst->capacity *= 2;
                    dst->items = (luby_value *)luby_alloc_raw(L, dst->items, dst->capacity * sizeof(luby_value));
                }
                dst->items[dst->count++] = inner->items[j];
            }
        } else {
            if (dst->count >= dst->capacity) {
                dst->capacity *= 2;
                dst->items = (luby_value *)luby_alloc_raw(L, dst->items, dst->capacity * sizeof(luby_value));
            }
            dst->items[dst->count++] = src->items[i];
        }
    }
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_array_uniq(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = 0; dst->capacity = src->count; dst->frozen = 0;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    
    for (size_t i = 0; i < src->count; i++) {
        int found = 0;
        for (size_t j = 0; j < dst->count; j++) {
            if (luby_value_eq(src->items[i], dst->items[j])) { found = 1; break; }
        }
        if (!found) dst->items[dst->count++] = src->items[i];
    }
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_array_sort(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *src = (luby_array *)argv[0].as.ptr;
    luby_array *dst = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!dst) return (int)LUBY_E_OOM;
    dst->count = src->count; dst->capacity = src->count; dst->frozen = 0;
    dst->items = (luby_value *)luby_alloc_raw(L, NULL, dst->capacity * sizeof(luby_value));
    memcpy(dst->items, src->items, src->count * sizeof(luby_value));
    
    // Simple bubble sort for now
    for (size_t i = 0; i < dst->count; i++) {
        for (size_t j = i + 1; j < dst->count; j++) {
            int swap = 0;
            if (dst->items[i].type == LUBY_T_INT && dst->items[j].type == LUBY_T_INT) {
                swap = dst->items[i].as.i > dst->items[j].as.i;
            } else if (dst->items[i].type == LUBY_T_FLOAT && dst->items[j].type == LUBY_T_FLOAT) {
                swap = dst->items[i].as.f > dst->items[j].as.f;
            } else if (dst->items[i].type == LUBY_T_STRING && dst->items[j].type == LUBY_T_STRING) {
                swap = strcmp((const char *)dst->items[i].as.ptr, (const char *)dst->items[j].as.ptr) > 0;
            }
            if (swap) {
                luby_value tmp = dst->items[i];
                dst->items[i] = dst->items[j];
                dst->items[j] = tmp;
            }
        }
    }
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_hash_keys(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!arr) return (int)LUBY_E_OOM;
    arr->count = h->count; arr->capacity = h->count; arr->frozen = 0;
    arr->items = (luby_value *)luby_alloc_raw(L, NULL, arr->capacity * sizeof(luby_value));
    for (size_t i = 0; i < h->count; i++) {
        arr->items[i] = h->entries[i].key;
    }
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = arr; }
    return (int)LUBY_E_OK;
}

static int luby_hash_values(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_HASH || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_hash *h = (luby_hash *)argv[0].as.ptr;
    luby_array *arr = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!arr) return (int)LUBY_E_OOM;
    arr->count = h->count; arr->capacity = h->count; arr->frozen = 0;
    arr->items = (luby_value *)luby_alloc_raw(L, NULL, arr->capacity * sizeof(luby_value));
    for (size_t i = 0; i < h->count; i++) {
        arr->items[i] = h->entries[i].value;
    }
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = arr; }
    return (int)LUBY_E_OK;
}

static int luby_base_times(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) { if (out) *out = argv[0]; return (int)LUBY_E_OK; }
    
    int64_t n = argv[0].as.i;
    for (int64_t i = 0; i < n; i++) {
        luby_value iv = luby_int(i);
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &iv, &res) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_base_upto(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_INT || argv[1].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) { if (out) *out = argv[0]; return (int)LUBY_E_OK; }
    
    int64_t from = argv[0].as.i;
    int64_t to = argv[1].as.i;
    for (int64_t i = from; i <= to; i++) {
        luby_value iv = luby_int(i);
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &iv, &res) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

static int luby_base_downto(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_INT || argv[1].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_proc *block = (L && L->current_block.type == LUBY_T_PROC) ? (luby_proc *)L->current_block.as.ptr : NULL;
    if (!block) { if (out) *out = argv[0]; return (int)LUBY_E_OK; }
    
    int64_t from = argv[0].as.i;
    int64_t to = argv[1].as.i;
    for (int64_t i = from; i >= to; i--) {
        luby_value iv = luby_int(i);
        luby_value res = luby_nil();
        if (luby_call_block(L, block, 1, &iv, &res) != 0) return (int)LUBY_E_RUNTIME;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

// ---------------------- More stdlib functions -------------------------

static int luby_base_abs(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_INT) {
        int64_t v = argv[0].as.i;
        if (out) *out = luby_int(v < 0 ? -v : v);
    } else if (argv[0].type == LUBY_T_FLOAT) {
        double v = argv[0].as.f;
        if (out) *out = luby_float(v < 0 ? -v : v);
    } else return (int)LUBY_E_TYPE;
    return (int)LUBY_E_OK;
}

static int luby_base_floor(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_FLOAT) {
        if (out) *out = luby_int((int64_t)argv[0].as.f);
    } else if (argv[0].type == LUBY_T_INT) {
        if (out) *out = argv[0];
    } else return (int)LUBY_E_TYPE;
    return (int)LUBY_E_OK;
}

static int luby_base_ceil(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_FLOAT) {
        double v = argv[0].as.f;
        int64_t iv = (int64_t)v;
        if (v > (double)iv) iv++;
        if (out) *out = luby_int(iv);
    } else if (argv[0].type == LUBY_T_INT) {
        if (out) *out = argv[0];
    } else return (int)LUBY_E_TYPE;
    return (int)LUBY_E_OK;
}

static int luby_base_round(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_FLOAT) {
        double v = argv[0].as.f;
        if (out) *out = luby_int((int64_t)(v + (v >= 0 ? 0.5 : -0.5)));
    } else if (argv[0].type == LUBY_T_INT) {
        if (out) *out = argv[0];
    } else return (int)LUBY_E_TYPE;
    return (int)LUBY_E_OK;
}

static int luby_base_even(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1 || argv[0].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    if (out) *out = luby_bool((argv[0].as.i % 2) == 0);
    return (int)LUBY_E_OK;
}

static int luby_base_odd(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1 || argv[0].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    if (out) *out = luby_bool((argv[0].as.i % 2) != 0);
    return (int)LUBY_E_OK;
}

static int luby_numeric_zero(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_INT) {
        if (out) *out = luby_bool(argv[0].as.i == 0);
        return (int)LUBY_E_OK;
    }
    if (argv[0].type == LUBY_T_FLOAT) {
        if (out) *out = luby_bool(argv[0].as.f == 0.0);
        return (int)LUBY_E_OK;
    }
    return (int)LUBY_E_TYPE;
}

static int luby_numeric_positive(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_INT) {
        if (out) *out = luby_bool(argv[0].as.i > 0);
        return (int)LUBY_E_OK;
    }
    if (argv[0].type == LUBY_T_FLOAT) {
        if (out) *out = luby_bool(argv[0].as.f > 0.0);
        return (int)LUBY_E_OK;
    }
    return (int)LUBY_E_TYPE;
}

static int luby_numeric_negative(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_INT) {
        if (out) *out = luby_bool(argv[0].as.i < 0);
        return (int)LUBY_E_OK;
    }
    if (argv[0].type == LUBY_T_FLOAT) {
        if (out) *out = luby_bool(argv[0].as.f < 0.0);
        return (int)LUBY_E_OK;
    }
    return (int)LUBY_E_TYPE;
}

// ---------------------- Game math helpers -------------------------

static double luby_to_double(luby_value v) {
    if (v.type == LUBY_T_INT) return (double)v.as.i;
    if (v.type == LUBY_T_FLOAT) return v.as.f;
    return 0.0;
}

static int luby_math_lerp(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 3) return (int)LUBY_E_TYPE;
    double a = luby_to_double(argv[0]);
    double b = luby_to_double(argv[1]);
    double t = luby_to_double(argv[2]);
    if (out) *out = luby_float(a + (b - a) * t);
    return (int)LUBY_E_OK;
}

static int luby_math_inverse_lerp(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 3) return (int)LUBY_E_TYPE;
    double a = luby_to_double(argv[0]);
    double b = luby_to_double(argv[1]);
    double v = luby_to_double(argv[2]);
    if (b == a) {
        if (out) *out = luby_float(0.0);
    } else {
        if (out) *out = luby_float((v - a) / (b - a));
    }
    return (int)LUBY_E_OK;
}

static int luby_math_smoothstep(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 3) return (int)LUBY_E_TYPE;
    double edge0 = luby_to_double(argv[0]);
    double edge1 = luby_to_double(argv[1]);
    double x = luby_to_double(argv[2]);
    double t = (x - edge0) / (edge1 - edge0);
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    if (out) *out = luby_float(t * t * (3.0 - 2.0 * t));
    return (int)LUBY_E_OK;
}

static int luby_math_clamp(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 3) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    double lo = luby_to_double(argv[1]);
    double hi = luby_to_double(argv[2]);
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    // Return int if all inputs were int
    if (argv[0].type == LUBY_T_INT && argv[1].type == LUBY_T_INT && argv[2].type == LUBY_T_INT) {
        if (out) *out = luby_int((int64_t)v);
    } else {
        if (out) *out = luby_float(v);
    }
    return (int)LUBY_E_OK;
}

static int luby_math_wrap(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 3) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    double lo = luby_to_double(argv[1]);
    double hi = luby_to_double(argv[2]);
    double range = hi - lo;
    if (range <= 0) {
        if (out) *out = luby_float(lo);
        return (int)LUBY_E_OK;
    }
    double result = v - range * floor((v - lo) / range);
    if (argv[0].type == LUBY_T_INT && argv[1].type == LUBY_T_INT && argv[2].type == LUBY_T_INT) {
        if (out) *out = luby_int((int64_t)result);
    } else {
        if (out) *out = luby_float(result);
    }
    return (int)LUBY_E_OK;
}

static int luby_math_sign(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    int64_t s = (v > 0) ? 1 : ((v < 0) ? -1 : 0);
    if (out) *out = luby_int(s);
    return (int)LUBY_E_OK;
}

static int luby_math_min(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double result = luby_to_double(argv[0]);
    int all_int = (argv[0].type == LUBY_T_INT);
    for (int i = 1; i < argc; i++) {
        double v = luby_to_double(argv[i]);
        if (v < result) result = v;
        if (argv[i].type != LUBY_T_INT) all_int = 0;
    }
    if (all_int) {
        if (out) *out = luby_int((int64_t)result);
    } else {
        if (out) *out = luby_float(result);
    }
    return (int)LUBY_E_OK;
}

static int luby_math_max(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double result = luby_to_double(argv[0]);
    int all_int = (argv[0].type == LUBY_T_INT);
    for (int i = 1; i < argc; i++) {
        double v = luby_to_double(argv[i]);
        if (v > result) result = v;
        if (argv[i].type != LUBY_T_INT) all_int = 0;
    }
    if (all_int) {
        if (out) *out = luby_int((int64_t)result);
    } else {
        if (out) *out = luby_float(result);
    }
    return (int)LUBY_E_OK;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int luby_math_deg_to_rad(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double deg = luby_to_double(argv[0]);
    if (out) *out = luby_float(deg * M_PI / 180.0);
    return (int)LUBY_E_OK;
}

static int luby_math_rad_to_deg(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double rad = luby_to_double(argv[0]);
    if (out) *out = luby_float(rad * 180.0 / M_PI);
    return (int)LUBY_E_OK;
}

static int luby_math_sin(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(sin(v));
    return (int)LUBY_E_OK;
}

static int luby_math_cos(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(cos(v));
    return (int)LUBY_E_OK;
}

static int luby_math_tan(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(tan(v));
    return (int)LUBY_E_OK;
}

static int luby_math_asin(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(asin(v));
    return (int)LUBY_E_OK;
}

static int luby_math_acos(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(acos(v));
    return (int)LUBY_E_OK;
}

static int luby_math_atan(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(atan(v));
    return (int)LUBY_E_OK;
}

static int luby_math_atan2(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    double y = luby_to_double(argv[0]);
    double x = luby_to_double(argv[1]);
    if (out) *out = luby_float(atan2(y, x));
    return (int)LUBY_E_OK;
}

static int luby_math_sqrt(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(sqrt(v));
    return (int)LUBY_E_OK;
}

static int luby_math_pow(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    double base = luby_to_double(argv[0]);
    double exp = luby_to_double(argv[1]);
    if (out) *out = luby_float(pow(base, exp));
    return (int)LUBY_E_OK;
}

static int luby_math_log(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(log(v));
    return (int)LUBY_E_OK;
}

static int luby_math_exp(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 1) return (int)LUBY_E_TYPE;
    double v = luby_to_double(argv[0]);
    if (out) *out = luby_float(exp(v));
    return (int)LUBY_E_OK;
}

static int luby_math_distance(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 4) return (int)LUBY_E_TYPE;
    double x1 = luby_to_double(argv[0]);
    double y1 = luby_to_double(argv[1]);
    double x2 = luby_to_double(argv[2]);
    double y2 = luby_to_double(argv[3]);
    double dx = x2 - x1;
    double dy = y2 - y1;
    if (out) *out = luby_float(sqrt(dx * dx + dy * dy));
    return (int)LUBY_E_OK;
}

static int luby_math_distance_squared(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 4) return (int)LUBY_E_TYPE;
    double x1 = luby_to_double(argv[0]);
    double y1 = luby_to_double(argv[1]);
    double x2 = luby_to_double(argv[2]);
    double y2 = luby_to_double(argv[3]);
    double dx = x2 - x1;
    double dy = y2 - y1;
    if (out) *out = luby_float(dx * dx + dy * dy);
    return (int)LUBY_E_OK;
}

static int luby_math_normalize(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2) return (int)LUBY_E_TYPE;
    double x = luby_to_double(argv[0]);
    double y = luby_to_double(argv[1]);
    double len = sqrt(x * x + y * y);
    luby_value arrv = luby_array_new(L);
    if (arrv.type == LUBY_T_NIL) return (int)LUBY_E_OOM;
    if (len > 0) {
        luby_array_push_value(L, arrv, luby_float(x / len));
        luby_array_push_value(L, arrv, luby_float(y / len));
    } else {
        luby_array_push_value(L, arrv, luby_float(0.0));
        luby_array_push_value(L, arrv, luby_float(0.0));
    }
    if (out) *out = arrv;
    return (int)LUBY_E_OK;
}

static int luby_math_dot(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 4) return (int)LUBY_E_TYPE;
    double x1 = luby_to_double(argv[0]);
    double y1 = luby_to_double(argv[1]);
    double x2 = luby_to_double(argv[2]);
    double y2 = luby_to_double(argv[3]);
    if (out) *out = luby_float(x1 * x2 + y1 * y2);
    return (int)LUBY_E_OK;
}

static int luby_math_cross(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 4) return (int)LUBY_E_TYPE;
    double x1 = luby_to_double(argv[0]);
    double y1 = luby_to_double(argv[1]);
    double x2 = luby_to_double(argv[2]);
    double y2 = luby_to_double(argv[3]);
    if (out) *out = luby_float(x1 * y2 - y1 * x2);
    return (int)LUBY_E_OK;
}

static int luby_math_angle(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    double x = luby_to_double(argv[0]);
    double y = luby_to_double(argv[1]);
    if (out) *out = luby_float(atan2(y, x));
    return (int)LUBY_E_OK;
}

static int luby_math_fmod(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    double a = luby_to_double(argv[0]);
    double b = luby_to_double(argv[1]);
    if (b == 0) {
        if (out) *out = luby_float(0.0);
    } else {
        if (out) *out = luby_float(fmod(a, b));
    }
    return (int)LUBY_E_OK;
}

// ---------------------- Seeded RNG (xoroshiro128+) -------------------------

static uint64_t luby_rng_rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t luby_rng_next(luby_state *L) {
    uint64_t s0 = L->rng_state[0];
    uint64_t s1 = L->rng_state[1];
    uint64_t result = s0 + s1;
    s1 ^= s0;
    L->rng_state[0] = luby_rng_rotl(s0, 24) ^ s1 ^ (s1 << 16);
    L->rng_state[1] = luby_rng_rotl(s1, 37);
    return result;
}

static double luby_rng_double(luby_state *L) {
    uint64_t r = luby_rng_next(L);
    return (double)(r >> 11) * (1.0 / (double)(1ULL << 53));
}

static int luby_rng_srand(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    uint64_t seed = 0;
    if (argc >= 1) {
        if (argv[0].type == LUBY_T_INT) seed = (uint64_t)argv[0].as.i;
        else if (argv[0].type == LUBY_T_FLOAT) seed = (uint64_t)argv[0].as.f;
    }
    // SplitMix64 to initialize state from seed
    seed += 0x9e3779b97f4a7c15ULL;
    uint64_t z = seed;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    L->rng_state[0] = z ^ (z >> 31);
    seed += 0x9e3779b97f4a7c15ULL;
    z = seed;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    L->rng_state[1] = z ^ (z >> 31);
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_rng_rand(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc == 0) {
        // rand() -> float in [0, 1)
        if (out) *out = luby_float(luby_rng_double(L));
    } else if (argc == 1) {
        if (argv[0].type == LUBY_T_INT) {
            // rand(n) -> int in [0, n)
            int64_t n = argv[0].as.i;
            if (n <= 0) {
                if (out) *out = luby_int(0);
            } else {
                uint64_t r = luby_rng_next(L);
                if (out) *out = luby_int((int64_t)(r % (uint64_t)n));
            }
        } else if (argv[0].type == LUBY_T_RANGE && argv[0].as.ptr) {
            // rand(a..b) -> int in [a, b] (inclusive for .. ranges)
            luby_range *rng = (luby_range *)argv[0].as.ptr;
            int64_t lo = (rng->start.type == LUBY_T_INT) ? rng->start.as.i : 0;
            int64_t hi = (rng->end.type == LUBY_T_INT) ? rng->end.as.i : 0;
            if (rng->exclusive) hi--;
            if (hi < lo) {
                if (out) *out = luby_int(lo);
            } else {
                uint64_t span = (uint64_t)(hi - lo + 1);
                uint64_t r = luby_rng_next(L);
                if (out) *out = luby_int(lo + (int64_t)(r % span));
            }
        } else {
            if (out) *out = luby_float(luby_rng_double(L));
        }
    } else if (argc >= 2) {
        // rand(a, b) -> int in [a, b]
        int64_t lo = (argv[0].type == LUBY_T_INT) ? argv[0].as.i : 0;
        int64_t hi = (argv[1].type == LUBY_T_INT) ? argv[1].as.i : 0;
        if (hi < lo) {
            if (out) *out = luby_int(lo);
        } else {
            uint64_t span = (uint64_t)(hi - lo + 1);
            uint64_t r = luby_rng_next(L);
            if (out) *out = luby_int(lo + (int64_t)(r % span));
        }
    }
    return (int)LUBY_E_OK;
}

static int luby_rng_rand_float(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2) {
        if (out) *out = luby_float(luby_rng_double(L));
    } else {
        double lo = luby_to_double(argv[0]);
        double hi = luby_to_double(argv[1]);
        double t = luby_rng_double(L);
        if (out) *out = luby_float(lo + (hi - lo) * t);
    }
    return (int)LUBY_E_OK;
}

static int luby_rng_sample(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    if (arr->count == 0) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    uint64_t r = luby_rng_next(L);
    size_t idx = (size_t)(r % arr->count);
    if (out) *out = arr->items[idx];
    return (int)LUBY_E_OK;
}

static int luby_rng_shuffle(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    luby_array *src = (luby_array *)argv[0].as.ptr;
    // Create a copy
    luby_value arrv = luby_array_new(L);
    if (arrv.type == LUBY_T_NIL) return (int)LUBY_E_OOM;
    for (size_t i = 0; i < src->count; i++) {
        luby_array_push_value(L, arrv, src->items[i]);
    }
    luby_array *arr = (luby_array *)arrv.as.ptr;
    // Fisher-Yates shuffle
    for (size_t i = arr->count; i > 1; i--) {
        uint64_t r = luby_rng_next(L);
        size_t j = (size_t)(r % i);
        luby_value tmp = arr->items[i - 1];
        arr->items[i - 1] = arr->items[j];
        arr->items[j] = tmp;
    }
    if (out) *out = arrv;
    return (int)LUBY_E_OK;
}

static int luby_rng_shuffle_bang(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    // Fisher-Yates shuffle in-place
    for (size_t i = arr->count; i > 1; i--) {
        uint64_t r = luby_rng_next(L);
        size_t j = (size_t)(r % i);
        luby_value tmp = arr->items[i - 1];
        arr->items[i - 1] = arr->items[j];
        arr->items[j] = tmp;
    }
    if (out) *out = argv[0];
    return (int)LUBY_E_OK;
}

// ---------------------- Probability helpers -------------------------

static int luby_rng_chance(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) {
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }
    double percent = luby_to_double(argv[0]);
    double roll = luby_rng_double(L) * 100.0;
    if (out) *out = luby_bool(roll < percent);
    return (int)LUBY_E_OK;
}

static int luby_rng_dice(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    int64_t count = 1;
    int64_t sides = 6;
    if (argc >= 1 && argv[0].type == LUBY_T_INT) count = argv[0].as.i;
    if (argc >= 2 && argv[1].type == LUBY_T_INT) sides = argv[1].as.i;
    if (count < 0) count = 0;
    if (sides < 1) sides = 1;
    int64_t total = 0;
    for (int64_t i = 0; i < count; i++) {
        uint64_t r = luby_rng_next(L);
        total += 1 + (int64_t)(r % (uint64_t)sides);
    }
    if (out) *out = luby_int(total);
    return (int)LUBY_E_OK;
}

static int luby_rng_weighted_choice(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    // Accept either hash {item => weight} or array [[item, weight], ...]
    if (argv[0].type == LUBY_T_HASH && argv[0].as.ptr) {
        luby_hash *h = (luby_hash *)argv[0].as.ptr;
        if (h->count == 0) {
            if (out) *out = luby_nil();
            return (int)LUBY_E_OK;
        }
        // Sum weights
        double total_weight = 0.0;
        for (size_t i = 0; i < h->count; i++) {
            total_weight += luby_to_double(h->entries[i].value);
        }
        if (total_weight <= 0.0) {
            if (out) *out = luby_nil();
            return (int)LUBY_E_OK;
        }
        // Pick
        double roll = luby_rng_double(L) * total_weight;
        double cumulative = 0.0;
        for (size_t i = 0; i < h->count; i++) {
            cumulative += luby_to_double(h->entries[i].value);
            if (roll < cumulative) {
                if (out) *out = h->entries[i].key;
                return (int)LUBY_E_OK;
            }
        }
        // Fallback to last
        if (out) *out = h->entries[h->count - 1].key;
    } else if (argv[0].type == LUBY_T_ARRAY && argv[0].as.ptr) {
        luby_array *arr = (luby_array *)argv[0].as.ptr;
        if (arr->count == 0) {
            if (out) *out = luby_nil();
            return (int)LUBY_E_OK;
        }
        // Sum weights from [[item, weight], ...]
        double total_weight = 0.0;
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].type == LUBY_T_ARRAY && arr->items[i].as.ptr) {
                luby_array *pair = (luby_array *)arr->items[i].as.ptr;
                if (pair->count >= 2) {
                    total_weight += luby_to_double(pair->items[1]);
                }
            }
        }
        if (total_weight <= 0.0) {
            if (out) *out = luby_nil();
            return (int)LUBY_E_OK;
        }
        // Pick
        double roll = luby_rng_double(L) * total_weight;
        double cumulative = 0.0;
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].type == LUBY_T_ARRAY && arr->items[i].as.ptr) {
                luby_array *pair = (luby_array *)arr->items[i].as.ptr;
                if (pair->count >= 2) {
                    cumulative += luby_to_double(pair->items[1]);
                    if (roll < cumulative) {
                        if (out) *out = pair->items[0];
                        return (int)LUBY_E_OK;
                    }
                }
            }
        }
        // Fallback
        luby_array *last = (luby_array *)arr->items[arr->count - 1].as.ptr;
        if (last && last->count >= 1) {
            if (out) *out = last->items[0];
        } else {
            if (out) *out = luby_nil();
        }
    } else {
        if (out) *out = luby_nil();
    }
    return (int)LUBY_E_OK;
}

static int luby_rng_roll(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    // roll("3d6") or roll("2d20+5") style parsing
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) {
        if (out) *out = luby_int(0);
        return (int)LUBY_E_OK;
    }
    const char *str = (const char *)argv[0].as.ptr;
    int64_t count = 0, sides = 0, modifier = 0;
    int sign = 1;
    // Parse: [count]d<sides>[+/-modifier]
    const char *p = str;
    // Count (optional, defaults to 1)
    while (*p >= '0' && *p <= '9') {
        count = count * 10 + (*p - '0');
        p++;
    }
    if (count == 0) count = 1;
    // 'd' or 'D'
    if (*p == 'd' || *p == 'D') p++;
    // Sides
    while (*p >= '0' && *p <= '9') {
        sides = sides * 10 + (*p - '0');
        p++;
    }
    if (sides == 0) sides = 6;
    // Modifier
    if (*p == '+') { sign = 1; p++; }
    else if (*p == '-') { sign = -1; p++; }
    while (*p >= '0' && *p <= '9') {
        modifier = modifier * 10 + (*p - '0');
        p++;
    }
    modifier *= sign;
    // Roll
    int64_t total = 0;
    for (int64_t i = 0; i < count; i++) {
        uint64_t r = luby_rng_next(L);
        total += 1 + (int64_t)(r % (uint64_t)sides);
    }
    total += modifier;
    if (out) *out = luby_int(total);
    return (int)LUBY_E_OK;
}

// ---------------------- VFS Data Loading -------------------------

static int luby_resolve_vfs_path(luby_state *L, const char *path, char *resolved, size_t resolved_size) {
    if (!L->cfg.vfs.exists) return 0;
    
    // Try direct path first
    if (L->cfg.vfs.exists(L->cfg.vfs.user, path)) {
        snprintf(resolved, resolved_size, "%s", path);
        return 1;
    }
    
    // Search in search paths
    for (size_t i = 0; i < L->search_path_count; i++) {
        const char *base = L->search_paths[i];
        if (!base) continue;
        size_t blen = strlen(base);
        snprintf(resolved, resolved_size, "%s%s%s", base, (blen && base[blen - 1] == '/') ? "" : "/", path);
        if (L->cfg.vfs.exists(L->cfg.vfs.user, resolved)) {
            return 1;
        }
    }
    return 0;
}

static int luby_base_load_text(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    
    if (!L->cfg.vfs.read || !L->cfg.vfs.exists) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    
    const char *path = (const char *)argv[0].as.ptr;
    char resolved[1024];
    
    if (!luby_resolve_vfs_path(L, path, resolved, sizeof(resolved))) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    
    size_t size = 0;
    char *content = L->cfg.vfs.read(L->cfg.vfs.user, resolved, &size);
    if (!content) {
        if (out) *out = luby_nil();
        return (int)LUBY_E_OK;
    }
    
    luby_value result = luby_string(L, content, size);
    // Free the VFS-allocated buffer
    if (L->cfg.alloc) {
        L->cfg.alloc(L->cfg.alloc_user, content, 0);
    } else {
        luby_default_alloc(NULL, content, 0);
    }
    
    if (out) *out = result;
    return (int)LUBY_E_OK;
}

static int luby_base_file_exists(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) {
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }
    
    if (!L->cfg.vfs.exists) {
        if (out) *out = luby_bool(0);
        return (int)LUBY_E_OK;
    }
    
    const char *path = (const char *)argv[0].as.ptr;
    char resolved[1024];
    
    if (out) *out = luby_bool(luby_resolve_vfs_path(L, path, resolved, sizeof(resolved)));
    return (int)LUBY_E_OK;
}

static int luby_base_includes(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2) return (int)LUBY_E_TYPE;
    if (argv[0].type == LUBY_T_STRING && argv[0].as.ptr && argv[1].type == LUBY_T_STRING && argv[1].as.ptr) {
        const char *haystack = (const char *)argv[0].as.ptr;
        const char *needle = (const char *)argv[1].as.ptr;
        if (out) *out = luby_bool(strstr(haystack, needle) != NULL);
    } else if (argv[0].type == LUBY_T_ARRAY && argv[0].as.ptr) {
        luby_array *arr = (luby_array *)argv[0].as.ptr;
        int found = 0;
        for (size_t i = 0; i < arr->count; i++) {
            if (luby_value_eq(arr->items[i], argv[1])) { found = 1; break; }
        }
        if (out) *out = luby_bool(found);
    } else return (int)LUBY_E_TYPE;
    return (int)LUBY_E_OK;
}

static int luby_base_index(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc < 2 || argv[0].type != LUBY_T_ARRAY || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    for (size_t i = 0; i < arr->count; i++) {
        if (luby_value_eq(arr->items[i], argv[1])) {
            if (out) *out = luby_int((int64_t)i);
            return (int)LUBY_E_OK;
        }
    }
    if (out) *out = luby_nil();
    return (int)LUBY_E_OK;
}

static int luby_base_concat(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_ARRAY || argv[1].type != LUBY_T_ARRAY) return (int)LUBY_E_TYPE;
    luby_array *a = (luby_array *)argv[0].as.ptr;
    luby_array *b = (luby_array *)argv[1].as.ptr;
    luby_array *result = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!result) return (int)LUBY_E_OOM;
    result->count = a->count + b->count;
    result->capacity = result->count;
    result->frozen = 0;
    result->items = (luby_value *)luby_alloc_raw(L, NULL, result->capacity * sizeof(luby_value));
    memcpy(result->items, a->items, a->count * sizeof(luby_value));
    memcpy(result->items + a->count, b->items, b->count * sizeof(luby_value));
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = result; }
    return (int)LUBY_E_OK;
}

static int luby_base_take(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_ARRAY || argv[1].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    size_t n = (size_t)argv[1].as.i;
    if (n > arr->count) n = arr->count;
    luby_array *result = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!result) return (int)LUBY_E_OOM;
    result->count = n;
    result->capacity = n > 0 ? n : 1;
    result->frozen = 0;
    result->items = (luby_value *)luby_alloc_raw(L, NULL, result->capacity * sizeof(luby_value));
    memcpy(result->items, arr->items, n * sizeof(luby_value));
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = result; }
    return (int)LUBY_E_OK;
}

static int luby_base_drop(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 2 || argv[0].type != LUBY_T_ARRAY || argv[1].type != LUBY_T_INT) return (int)LUBY_E_TYPE;
    luby_array *arr = (luby_array *)argv[0].as.ptr;
    size_t n = (size_t)argv[1].as.i;
    if (n > arr->count) n = arr->count;
    size_t rem = arr->count - n;
    luby_array *result = (luby_array *)luby_gc_alloc(L, sizeof(luby_array), LUBY_GC_ARRAY);
    if (!result) return (int)LUBY_E_OOM;
    result->count = rem;
    result->capacity = rem > 0 ? rem : 1;
    result->frozen = 0;
    result->items = (luby_value *)luby_alloc_raw(L, NULL, result->capacity * sizeof(luby_value));
    memcpy(result->items, arr->items + n, rem * sizeof(luby_value));
    if (out) { out->type = LUBY_T_ARRAY; out->as.ptr = result; }
    return (int)LUBY_E_OK;
}

static int luby_str_capitalize(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    const char *src = (const char *)argv[0].as.ptr;
    size_t len = strlen(src);
    // Pause GC - src points to argv which may not be rooted during alloc
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    char *dst = luby_gc_alloc_string(L, src, len);
    if (!dst) { L->gc_paused = was_paused; return (int)LUBY_E_OOM; }
    for (size_t i = 0; i < len; i++) {
        if (i == 0 && src[i] >= 'a' && src[i] <= 'z') dst[i] = (char)(src[i] - 32);
        else if (i > 0 && src[i] >= 'A' && src[i] <= 'Z') dst[i] = (char)(src[i] + 32);
        else dst[i] = src[i];
    }
    L->gc_paused = was_paused;
    if (out) { out->type = LUBY_T_STRING; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

static int luby_str_strip(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (argc < 1 || argv[0].type != LUBY_T_STRING || !argv[0].as.ptr) return (int)LUBY_E_TYPE;
    const char *src = (const char *)argv[0].as.ptr;
    size_t len = strlen(src);
    size_t start = 0, end = len;
    while (start < len && (src[start] == ' ' || src[start] == '\t' || src[start] == '\n' || src[start] == '\r')) start++;
    while (end > start && (src[end-1] == ' ' || src[end-1] == '\t' || src[end-1] == '\n' || src[end-1] == '\r')) end--;
    size_t new_len = end - start;
    // Pause GC - src points to argv which may not be rooted during alloc
    int was_paused = L->gc_paused;
    L->gc_paused = 1;
    char *dst = luby_gc_alloc_string(L, src + start, new_len);
    L->gc_paused = was_paused;
    if (!dst) return (int)LUBY_E_OOM;
    if (out) { out->type = LUBY_T_STRING; out->as.ptr = dst; }
    return (int)LUBY_E_OK;
}

LUBY_API void luby_open_base(luby_state *L) {
    if (!L) return;
    luby_register_function(L, "print", luby_base_print);
    luby_register_function(L, "puts", luby_base_puts);
    luby_register_function(L, "type", luby_base_type);
    luby_register_function(L, "to_i", luby_base_to_i);
    luby_register_function(L, "to_f", luby_base_to_f);
    luby_register_function(L, "len", luby_base_len);
    luby_register_function(L, "freeze", luby_base_freeze);
    luby_register_function(L, "frozen?", luby_base_frozen);
    luby_register_function(L, "dig", luby_base_dig);
    luby_register_function(L, "respond_to", luby_base_respond_to);
    luby_register_function(L, "is_a?", luby_base_is_a);
    luby_register_function(L, "kind_of?", luby_base_is_a);  // same as is_a?
    luby_register_function(L, "instance_of?", luby_base_instance_of);
    luby_register_function(L, "defined?", luby_base_defined);
    luby_register_function(L, "require", luby_base_require);
    luby_register_function(L, "load", luby_base_load);
    luby_register_function(L, "yield", luby_base_yield);
    luby_register_function(L, "enum_next", luby_enum_next);
    luby_register_function(L, "enum_rewind", luby_enum_rewind);
    luby_register_function(L, "enum_each", luby_enum_each);
    luby_register_function(L, "coroutine_new", luby_coroutine_new_cfunc);
    luby_register_function(L, "coroutine_resume", luby_coroutine_resume_cfunc);
    luby_register_function(L, "coroutine_alive", luby_coroutine_alive_cfunc);
    luby_register_function(L, "send", luby_base_send);
    luby_register_function(L, "public_send", luby_base_public_send);
    luby_register_function(L, "define_method", luby_base_define_method);
    luby_register_function(L, "define_singleton_method", luby_base_define_singleton_method);
    luby_register_function(L, "class_eval", luby_base_class_eval);
    luby_register_function(L, "instance_eval", luby_base_instance_eval);
    luby_register_function(L, "include", luby_base_include);
    luby_register_function(L, "prepend", luby_base_prepend);
    luby_register_function(L, "extend", luby_base_extend);
    luby_register_function(L, "attr_reader", luby_base_attr_reader);
    luby_register_function(L, "attr_writer", luby_base_attr_writer);
    luby_register_function(L, "attr_accessor", luby_base_attr_accessor);
    luby_register_function(L, "private", luby_base_private);
    luby_register_function(L, "public", luby_base_public);
    luby_register_function(L, "protected", luby_base_protected);
    luby_register_function(L, "alias", luby_base_alias);
    luby_register_function(L, "array_push", luby_array_push);
    luby_register_function(L, "array_pop", luby_array_pop);
    luby_register_function(L, "array_map", luby_array_map);
    luby_register_function(L, "array_select", luby_array_select);
    luby_register_function(L, "array_reject", luby_array_reject);
    luby_register_function(L, "array_each", luby_array_each);
    luby_register_function(L, "array_each_with_index", luby_array_each_with_index);
    luby_register_function(L, "array_compact", luby_array_compact);
    luby_register_function(L, "array_compact!", luby_array_compact_bang);
    luby_register_function(L, "array_reduce", luby_array_reduce);
    luby_register_function(L, "array_any", luby_array_any);
    luby_register_function(L, "array_all", luby_array_all);
    luby_register_function(L, "array_none", luby_array_none);
    luby_register_function(L, "array_find", luby_array_find);
    luby_register_function(L, "map", luby_array_map);
    luby_register_function(L, "select", luby_array_select);
    luby_register_function(L, "reject", luby_array_reject);
    luby_register_function(L, "each", luby_generic_each);
    luby_register_function(L, "each_with_index", luby_array_each_with_index);
    luby_register_function(L, "range_each", luby_range_each);
    luby_register_function(L, "compact", luby_array_compact);
    luby_register_function(L, "compact!", luby_array_compact_bang);
    luby_register_function(L, "reduce", luby_array_reduce);
    luby_register_function(L, "inject", luby_array_reduce);
    luby_register_function(L, "any?", luby_array_any);
    luby_register_function(L, "all?", luby_array_all);
    luby_register_function(L, "none?", luby_array_none);
    luby_register_function(L, "find", luby_array_find);
    luby_register_function(L, "hash_get", luby_hash_get);
    luby_register_function(L, "hash_set", luby_hash_set);
    luby_register_function(L, "hash_each", luby_hash_each);
    luby_register_function(L, "hash_map", luby_hash_map);
    luby_register_function(L, "hash_select", luby_hash_select);
    luby_register_function(L, "hash_reject", luby_hash_reject);
    luby_register_function(L, "hash_merge", luby_hash_merge);
    luby_register_function(L, "merge", luby_hash_merge);
    luby_register_function(L, "hash_any", luby_hash_any);
    luby_register_function(L, "hash_all", luby_hash_all);
    luby_register_function(L, "hash_none", luby_hash_none);
    luby_register_function(L, "hash_find", luby_hash_find);
    luby_register_function(L, "hash_reduce", luby_hash_reduce);
    luby_register_function(L, "each_hash", luby_hash_each);
    luby_register_function(L, "map_hash", luby_hash_map);
    luby_register_function(L, "select_hash", luby_hash_select);
    luby_register_function(L, "reject_hash", luby_hash_reject);
    luby_register_function(L, "merge_hash", luby_hash_merge);
    luby_register_function(L, "any_hash", luby_hash_any);
    luby_register_function(L, "all_hash", luby_hash_all);
    luby_register_function(L, "none_hash", luby_hash_none);
    luby_register_function(L, "find_hash", luby_hash_find);
    luby_register_function(L, "reduce_hash", luby_hash_reduce);
    // Additional stdlib functions
    luby_register_function(L, "to_s", luby_base_to_s);
    luby_register_function(L, "is_nil", luby_base_is_nil);
    luby_register_function(L, "nil?", luby_base_is_nil);
    luby_register_function(L, "upcase", luby_str_upcase);
    luby_register_function(L, "downcase", luby_str_downcase);
    luby_register_function(L, "split", luby_str_split);
    luby_register_function(L, "join", luby_str_join);
    luby_register_function(L, "reverse", luby_array_reverse);
    luby_register_function(L, "first", luby_array_first);
    luby_register_function(L, "last", luby_array_last);
    luby_register_function(L, "flatten", luby_array_flatten);
    luby_register_function(L, "uniq", luby_array_uniq);
    luby_register_function(L, "sort", luby_array_sort);
    luby_register_function(L, "keys", luby_hash_keys);
    luby_register_function(L, "values", luby_hash_values);
    luby_register_function(L, "times", luby_base_times);
    luby_register_function(L, "upto", luby_base_upto);
    luby_register_function(L, "downto", luby_base_downto);
    // More stdlib functions
    luby_register_function(L, "abs", luby_base_abs);
    luby_register_function(L, "floor", luby_base_floor);
    luby_register_function(L, "ceil", luby_base_ceil);
    luby_register_function(L, "round", luby_base_round);
    luby_register_function(L, "even?", luby_base_even);
    luby_register_function(L, "odd?", luby_base_odd);
    luby_register_function(L, "zero?", luby_numeric_zero);
    luby_register_function(L, "positive?", luby_numeric_positive);
    luby_register_function(L, "negative?", luby_numeric_negative);
    luby_register_function(L, "include?", luby_base_includes);
    luby_register_function(L, "index", luby_base_index);
    luby_register_function(L, "concat", luby_base_concat);
    luby_register_function(L, "take", luby_base_take);
    luby_register_function(L, "drop", luby_base_drop);
    luby_register_function(L, "capitalize", luby_str_capitalize);
    luby_register_function(L, "strip", luby_str_strip);
    // Game math helpers
    luby_register_function(L, "lerp", luby_math_lerp);
    luby_register_function(L, "inverse_lerp", luby_math_inverse_lerp);
    luby_register_function(L, "smoothstep", luby_math_smoothstep);
    luby_register_function(L, "clamp", luby_math_clamp);
    luby_register_function(L, "wrap", luby_math_wrap);
    luby_register_function(L, "sign", luby_math_sign);
    luby_register_function(L, "min", luby_math_min);
    luby_register_function(L, "max", luby_math_max);
    luby_register_function(L, "deg_to_rad", luby_math_deg_to_rad);
    luby_register_function(L, "rad_to_deg", luby_math_rad_to_deg);
    luby_register_function(L, "sin", luby_math_sin);
    luby_register_function(L, "cos", luby_math_cos);
    luby_register_function(L, "tan", luby_math_tan);
    luby_register_function(L, "asin", luby_math_asin);
    luby_register_function(L, "acos", luby_math_acos);
    luby_register_function(L, "atan", luby_math_atan);
    luby_register_function(L, "atan2", luby_math_atan2);
    luby_register_function(L, "sqrt", luby_math_sqrt);
    luby_register_function(L, "pow", luby_math_pow);
    luby_register_function(L, "log", luby_math_log);
    luby_register_function(L, "exp", luby_math_exp);
    luby_register_function(L, "distance", luby_math_distance);
    luby_register_function(L, "distance_squared", luby_math_distance_squared);
    luby_register_function(L, "normalize", luby_math_normalize);
    luby_register_function(L, "dot", luby_math_dot);
    luby_register_function(L, "cross", luby_math_cross);
    luby_register_function(L, "angle", luby_math_angle);
    luby_register_function(L, "fmod", luby_math_fmod);
    // RNG functions
    luby_register_function(L, "srand", luby_rng_srand);
    luby_register_function(L, "rand", luby_rng_rand);
    luby_register_function(L, "rand_float", luby_rng_rand_float);
    luby_register_function(L, "sample", luby_rng_sample);
    luby_register_function(L, "shuffle", luby_rng_shuffle);
    luby_register_function(L, "shuffle!", luby_rng_shuffle_bang);
    // Probability helpers
    luby_register_function(L, "chance", luby_rng_chance);
    luby_register_function(L, "dice", luby_rng_dice);
    luby_register_function(L, "roll", luby_rng_roll);
    luby_register_function(L, "weighted_choice", luby_rng_weighted_choice);
    // VFS data loading
    luby_register_function(L, "load_text", luby_base_load_text);
    luby_register_function(L, "file_exists?", luby_base_file_exists);

    {
        luby_class_obj *enum_cls = luby_class_new(L, "Enumerator", NULL);
        if (enum_cls) {
            luby_value v; v.type = LUBY_T_CLASS; v.as.ptr = enum_cls;
            luby_string_view name = { "Enumerator", 10 };
            luby_set_global(L, name, v);
            luby_eval(L,
                "class Enumerator\n"
                " def next()\n"
                "  enum_next(self)\n"
                " end\n"
                " def rewind()\n"
                "  enum_rewind(self)\n"
                " end\n"
                " def each()\n"
                "  enum_each(self)\n"
                " end\n"
                "end\n",
                0,
                "<enumerator>",
                NULL);
            luby_clear_error(L);
        }
    }

    {
        luby_class_obj *co_cls = luby_class_new(L, "Coroutine", NULL);
        if (co_cls) {
            luby_value v; v.type = LUBY_T_CLASS; v.as.ptr = co_cls;
            luby_string_view name = { "Coroutine", 9 };
            luby_set_global(L, name, v);
            luby_eval(L,
                "class Coroutine\n"
                " def resume(x)\n"
                "  coroutine_resume(self, x)\n"
                " end\n"
                " def alive?()\n"
                "  coroutine_alive(self)\n"
                " end\n"
                "end\n",
                0,
                "<coroutine>",
                NULL);
            luby_clear_error(L);
        }
    }
}
LUBY_API void luby_add_search_path(luby_state *L, const char *path) {
    if (!L || !path) return;
    luby_string_list_add(L, &L->search_paths, &L->search_path_count, &L->search_path_capacity, path);
}

LUBY_API void luby_clear_search_paths(luby_state *L) {
    if (!L) return;
    for (size_t i = 0; i < L->search_path_count; i++) {
        luby_alloc_raw(L, L->search_paths[i], 0);
    }
    luby_alloc_raw(L, L->search_paths, 0);
    L->search_paths = NULL;
    L->search_path_count = 0;
    L->search_path_capacity = 0;
}

LUBY_API luby_class *luby_define_class(luby_state *L, const char *name, const char *super_name) {
    if (!L || !name) return NULL;
    // Check if class already exists
    luby_value existing = luby_get_global_value(L, name);
    if (existing.type == LUBY_T_CLASS && existing.as.ptr) {
        luby_class *c = (luby_class *)luby_alloc_raw(L, NULL, sizeof(luby_class));
        if (c) c->obj = (luby_class_obj *)existing.as.ptr;
        return c;
    }
    // Find superclass
    luby_class_obj *super = NULL;
    if (super_name) {
        luby_value sv = luby_get_global_value(L, super_name);
        if (sv.type == LUBY_T_CLASS && sv.as.ptr) super = (luby_class_obj *)sv.as.ptr;
    }
    // Create new class
    luby_class_obj *cls_obj = luby_class_new(L, name, super);
    if (!cls_obj) return NULL;
    // Register globally
    luby_value cv; cv.type = LUBY_T_CLASS; cv.as.ptr = cls_obj;
    luby_set_global_value(L, name, cv);
    // Return API handle
    luby_class *c = (luby_class *)luby_alloc_raw(L, NULL, sizeof(luby_class));
    if (c) c->obj = cls_obj;
    return c;
}

LUBY_API int luby_define_method(luby_state *L, luby_class *cls, const char *name, luby_cfunc fn) {
    if (!L || !cls || !cls->obj || !name || !fn) return 0;
    luby_cmethod *cm = (luby_cmethod *)luby_gc_alloc(L, sizeof(luby_cmethod), LUBY_GC_CMETHOD);
    if (!cm) return 0;
    cm->fn = fn;
    luby_value key = luby_symbol(L, name, 0);
    luby_value val; val.type = LUBY_T_CMETHOD; val.as.ptr = cm;
    luby_hash_set_value(L, (luby_value){ .type = LUBY_T_HASH, .as.ptr = cls->obj->methods }, key, val);
    L->method_epoch++;
    return 1;
}

LUBY_API luby_value luby_new_userdata(luby_state *L, size_t size, luby_finalizer finalize) {
    (void)finalize;
    luby_value v; v.type = LUBY_T_OBJECT; v.as.ptr = luby_alloc_raw(L, NULL, size); return v;
}

LUBY_API void *luby_userdata_ptr(luby_value v) { return v.as.ptr; }

LUBY_API luby_coroutine *luby_coroutine_new(luby_state *L, luby_value func) {
    if (!L || func.type != LUBY_T_PROC || !func.as.ptr) return NULL;
    luby_coroutine *co = (luby_coroutine *)luby_gc_alloc(L, sizeof(luby_coroutine), LUBY_GC_COROUTINE);
    if (!co) return NULL;
    co->proc = (luby_proc *)func.as.ptr;
    co->done = 0;
    co->started = 0;
    luby_vm_init(&co->vm);
    return co;
}
LUBY_API int luby_coroutine_resume(luby_state *L, luby_coroutine *co, int argc, const luby_value *argv, luby_value *out, int *out_yielded) {
    if (!co || !co->proc) { if (out) *out = luby_nil(); if (out_yielded) *out_yielded = 0; return (int)LUBY_E_TYPE; }
    if (co->done) { if (out) *out = luby_nil(); if (out_yielded) *out_yielded = 0; return (int)LUBY_E_OK; }

    if (!co->started) {
        if (!luby_vm_ensure_stack(L, &co->vm, 1)) return (int)LUBY_E_OOM;
        if (!luby_vm_push_frame(L, &co->vm, co->proc, &co->proc->chunk, "<coroutine>", luby_nil(), NULL, NULL, argc, argv, luby_nil(), 0)) {
            return (int)LUBY_E_OOM;
        }
        co->started = 1;
    } else {
        co->vm.resume_pending = 1;
        co->vm.resume_value = (argc > 0) ? argv[0] : luby_nil();
    }

    luby_coroutine *saved_co = L->current_coroutine;
    L->current_coroutine = co;
    int rc = luby_vm_run(L, &co->vm, out);
    L->current_coroutine = saved_co;

    if (co->vm.yielded) {
        if (out) *out = co->vm.yield_value;
        if (out_yielded) *out_yielded = 1;
        co->vm.yielded = 0;
        co->vm.yield_value = luby_nil();
        luby_clear_error(L);
        return (int)LUBY_E_OK;
    }

    if (rc != (int)LUBY_E_OK) {
        if (out_yielded) *out_yielded = 0;
        co->done = 1;
        return rc;
    }

    if (co->vm.frame_count == 0) {
        co->done = 1;
    }
    if (out_yielded) *out_yielded = 0;
    return rc;
}
LUBY_API int luby_yield(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    if (!L) return (int)LUBY_E_RUNTIME;
    if (L->current_coroutine) {
        if (out) *out = luby_nil();
        luby_set_error(L, LUBY_E_RUNTIME, "yield from native not supported", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }
    if (L->current_block.type != LUBY_T_PROC) {
        if (out) *out = luby_nil();
        luby_set_error(L, LUBY_E_RUNTIME, "no block given", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }
    return luby_call_block(L, (luby_proc *)L->current_block.as.ptr, argc, argv, out);
}

LUBY_API int luby_native_yield(luby_state *L, luby_value value) {
    if (!L || !L->current_coroutine || !L->current_vm) {
        if (L) luby_set_error(L, LUBY_E_RUNTIME, "no coroutine", NULL, 0, 0);
        return (int)LUBY_E_RUNTIME;
    }
    L->current_vm->yielded = 1;
    L->current_vm->yield_value = value;
    L->current_vm->native_yield = 1;
    return (int)LUBY_E_OK;
}

LUBY_API void luby_set_hook(luby_state *L, luby_hook_fn fn, void *user) { if (!L) return; L->hook = fn; L->hook_user = user; }

#endif // LUBY_IMPLEMENTATION

#endif // LUBY_H
