#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#define HIJACK_xALLOC
#define HIJACK_FREE

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define UNUSED(v) ((void)v)

//
// Flag for init; NOT thread-safe
//
enum {
    INIT_NOT,
    INIT_ING,
    INIT_DONE
};
static int __init_status = INIT_NOT;

//
// Original functions
//
#ifdef HIJACK_xALLOC
typedef void* (*MallocFunc)(size_t size);
typedef void* (*ReallocFunc)(void *ptr, size_t size);
typedef void* (*CallocFunc)(size_t num, size_t size);

static MallocFunc __real_malloc = NULL;
static ReallocFunc __real_realloc = NULL;
static CallocFunc __real_calloc = NULL;
#endif

typedef void (*FreeFunc)(void*);
static FreeFunc __real_free = NULL;

//
// Init: load lib functions
//
/* static void do_init() __attribute__((constructor)); */
static void do_init() {
    __init_status = INIT_ING;
    // Override original functions
#ifdef HIJACK_xALLOC
    __real_malloc = (MallocFunc)dlsym(RTLD_NEXT, "malloc");
    __real_realloc = (ReallocFunc)dlsym(RTLD_NEXT, "realloc");
    __real_calloc = (CallocFunc)dlsym(RTLD_NEXT, "calloc");
    if (!__real_malloc || !__real_realloc || !__real_calloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    } else {
        fprintf(stderr, "NOTICE: malloc() replaced @%p\n", __real_malloc);
        fprintf(stderr, "NOTICE: realloc() replaced @%p\n", __real_realloc);
        fprintf(stderr, "NOTICE: calloc() replaced @%p\n", __real_calloc);
    }
#endif

#ifdef HIJACK_FREE
    __real_free = (FreeFunc)dlsym(RTLD_NEXT, "free");
    if (!__real_free) {
       fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    } else {
        fprintf(stderr, "NOTICE: free() replaced @%p\n", __real_free);
    }
#endif
    __init_status = INIT_DONE;
}

#ifdef HIJACK_xALLOC

#define TMP_BUF_SIZE 4096
static char __tmp_buf[TMP_BUF_SIZE];
static size_t __tmp_buf_pos = 0;
static void* tmp_alloc(size_t size) {
    char *ptr = &__tmp_buf[__tmp_buf_pos];
    __tmp_buf_pos += size;
    return ptr;
}

/* static int is_tmp_alloc(void *ptr) { */
    /* return __tmp_buf <= (char*)ptr && (char*)ptr < __tmp_buf + TMP_BUF_SIZE; */
/* } */

static void* do_alloc(size_t size) {
    void *ptr = NULL;
    if (__init_status == INIT_ING) {
        ptr = tmp_alloc(sizeof(size_t) + size);
    } else {
        ptr = __real_malloc(sizeof(size_t) + size);
    }
    if (!ptr) {
        return NULL;
    }

    *((size_t*)ptr) = size;

    return (char*)ptr + sizeof(size_t);
}

static size_t fetch_size(void *ptr) {
    if (!ptr) {
        return 0;
    }

    char *header_ptr = (char*)ptr - sizeof(size_t);
    return *((size_t*)header_ptr);
}

void* malloc(size_t size) {
    if(__init_status == INIT_NOT) {
         do_init();
    }

    return do_alloc(size);
}

void* realloc(void *ptr, size_t size) {
    if(__init_status == INIT_NOT) {
         do_init();
    }

    void *new_ptr = do_alloc(size);
    if (!ptr || !new_ptr) {
        return new_ptr;
    }
    size_t origin_size = fetch_size(ptr);
    memcpy(new_ptr, ptr, MIN(origin_size, size));
    return new_ptr;
}

void* calloc(size_t num, size_t size) {
    if(__init_status == INIT_NOT) {
         do_init();
    }

    void *ptr = do_alloc(num * size);
    memset(ptr, 0, num * size);
    return ptr;
}

#endif

#ifdef HIJACK_FREE

void free(void* ptr) {
    UNUSED(ptr);

    if(__real_free==NULL) {
        do_init();
    }

    /* fprintf(stderr, "NOTICE: free(%p)\n", ptr); */
    /* if (!is_tmp_alloc(ptr)) { */
        /* __real_free(ptr); */
    /* } */

    return;
}

#endif
