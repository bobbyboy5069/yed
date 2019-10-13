#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/mman.h>
#if defined(__linux__)
#include <linux/mman.h> /* linux mmap flags */
#endif
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>

#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>


#include "tree.h"

#define inline
typedef struct yed_key_binding_t *yed_key_binding_ptr_t;
use_tree(int, yed_key_binding_ptr_t);

typedef char *yed_command_name_t;
typedef void (*yed_command)(int, char**);
use_tree(yed_command_name_t, yed_command);

struct yed_frame_t;
typedef struct yed_frame_t *yed_frame_ptr_t;
typedef char *yed_frame_id_t;
use_tree(yed_frame_id_t, yed_frame_ptr_t);

use_tree(int, int);

typedef char *yed_plugin_name_t;
struct yed_plugin_t;
typedef struct yed_plugin_t *yed_plugin_ptr_t;
use_tree(yed_plugin_name_t, yed_plugin_ptr_t);
#undef inline

#include "array.h"
#include "bucket_array.h"
#include "yed.h"
#include "term.h"
#include "key.h"
#include "fs.h"
#include "buffer.h"
#include "frame.h"
#include "command.h"
#include "getRSS.h"
#include "plugin.h"

#define likely(x)   (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

#ifdef YED_DO_ASSERTIONS
void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str);
#define ASSERT(cond, msg)                            \
do { if (unlikely(!(cond))) {                        \
    yed_assert_fail(msg, __FILE__, __LINE__, #cond); \
} } while (0)
#else
#define ASSERT(cond, mst) ;
#endif

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define LIMIT(x, lower, upper) do { \
    if ((x) < (lower)) {            \
        (x) = (lower);              \
    } else if ((x) > (upper)) {     \
        (x) = (upper);              \
    }                               \
} while (0)

/* @incomplete */
#define ERR(msg) ASSERT(0, #msg)


#define UINT(w) uint##w##_t
#define SINT(w) int##w##_t

#define u8  UINT(8 )
#define u16 UINT(16)
#define u32 UINT(32)
#define u64 UINT(64)

#define i8  SINT(8 )
#define i16 SINT(16)
#define i32 SINT(32)
#define i64 SINT(64)

#define XOR_SWAP_64(a, b) do {   \
    a = ((u64)(a)) ^ ((u64)(b)); \
    b = ((u64)(b)) ^ ((u64)(a)); \
    a = ((u64)(a)) ^ ((u64)(b)); \
} while (0);

#define XOR_SWAP_PTR(a, b) do {           \
    a = (void*)(((u64)(a)) ^ ((u64)(b))); \
    b = (void*)(((u64)(b)) ^ ((u64)(a))); \
    a = (void*)(((u64)(a)) ^ ((u64)(b))); \
} while (0);

#define ALIGN(x, align)         ((__typeof(x))((((u64)(x)) + (((u64)align) - 1ULL)) & ~(((u64)align) - 1ULL)))
#define IS_ALIGNED(x, align)    (!(((u64)(x)) & (((u64)align) - 1ULL)))
#define IS_ALIGNED_PP(x, align) (!((x) & ((align) - 1ULL)))
#define IS_POWER_OF_TWO(x)      ((x) != 0 && IS_ALIGNED((x), (x)))
#define IS_POWER_OF_TWO_PP(x)   ((x) != 0 && IS_ALIGNED_PP((x), (x)))

#define LOG2_8BIT(v)  (8 - 90/(((v)/4+14)|1) - 2/((v)/2+1))
#define LOG2_16BIT(v) (8*((v)>255) + LOG2_8BIT((v) >>8*((v)>255)))
#define LOG2_32BIT(v)                                        \
    (16*((v)>65535L) + LOG2_16BIT((v)*1L >>16*((v)>65535L)))
#define LOG2_64BIT(v)                                        \
    (32*((v)/2L>>31 > 0)                                     \
     + LOG2_32BIT((v)*1L >>16*((v)/2L>>31 > 0)               \
                         >>16*((v)/2L>>31 > 0)))

uint64_t next_power_of_2(uint64_t x);

#define KiB(x) ((x) * 1024ULL)
#define MiB(x) ((x) * 1024ULL * KiB(1ULL))
#define GiB(x) ((x) * 1024ULL * MiB(1ULL))
#define TiB(x) ((x) * 1024ULL * GiB(1ULL))
char *pretty_bytes(uint64_t n_bytes);

typedef struct yed_state_t {
    yed_lib_t      *yed_lib;
    array_t         output_buffer;
    struct termios  sav_term;
    int             term_cols,
                    term_rows;
    int             cur_x,
                    cur_y,
                    save_cur_x,
                    save_cur_y;
    array_t         buff_list;
    yed_buffer      yank_buff;
    tree(yed_frame_id_t, yed_frame_ptr_t) frames;
    yed_frame      *active_frame;
    int             accepting_command;
    array_t         cmd_buff;
    int             cmd_cursor_x;
    int             status;
    tree(yed_command_name_t, yed_command) commands;
    tree(yed_command_name_t, yed_command) default_commands;
    char           *small_message;
    tree(yed_plugin_name_t, yed_plugin_ptr_t) plugins;
    array_t         plugin_dirs;
    yed_key_binding *real_key_map[REAL_KEY_MAX];
    tree(int, yed_key_binding_ptr_t) key_seq_map;
    array_t         key_sequences;
    int             seq_key_counter;
    array_t         released_seq_keys;
} yed_state;

extern yed_state *ys;

void yed_init_output_stream(void);

void yed_add_new_buff(void);

void clear_output_buff(void);
int output_buff_len(void);
void append_n_to_output_buff(char *s, int n);
void append_to_output_buff(char *s);
void append_int_to_output_buff(int i);
void flush_output_buff(void);

void yed_service_reload(void);

int s_to_i(const char *s);

#endif
