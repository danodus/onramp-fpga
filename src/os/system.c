// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: libc in onramp

#include "system.h"

#define EINVAL 4
#define ENOMEM 5
#define EIO 12
#define EBADF 11

#define INT_MAX 2147483647


#define __ONRAMP_PIT_VERSION 0
#define __ONRAMP_PIT_BREAK 1
#define __ONRAMP_PIT_SYSCALLS 2
#define __ONRAMP_PIT_INPUT 3
#define __ONRAMP_PIT_OUTPUT 4
#define __ONRAMP_PIT_ERROR 5
#define __ONRAMP_PIT_ARGS 6
#define __ONRAMP_PIT_ENVIRON 7
#define __ONRAMP_PIT_WORKDIR 8
#define __ONRAMP_PIT_CAPABILITIES 9
#define __ONRAMP_PIT_COUNT 10

int __argc;
char** __argv;

unsigned* __process_info_table;

static int* free_list;

int errno;

static char* empty_environment[1];
char** environ;

static char* empty_argv[2];

static int input_echo;
static int input_block;
static int input_canonical;


extern int main(void);

extern void* __constructors[];
extern void* __destructors[];
extern void __call_constructor(int argc, char** argv, char** envp, void* func);
extern void __call_destructor(void* func);
static void call_constructors(void);
static void call_destructors(void);

int __sys_fwrite(int handle, const void* buffer, unsigned size);


// ref: https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
char* uitoa(unsigned int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    for (int i = 0; i < 8; ++i) {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    }

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

void putchar(char c) {
    *(int *)(0x20000004) = c;
}

void print(char* s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

void printhex(char *s, int v) {
    print(s);
    char buf[32];
    uitoa(v, buf, 16);
    print(buf);
    print("\r\n");
}

static void exit(int status) {
    call_destructors();    
    *(int *)(0x20000000) = status;
}

static void call_constructors(void) {
    void** constructors = __constructors;
    while (*constructors) {
        __call_constructor(0, 0, NULL, *constructors++);
    }
}

static void call_destructors(void) {
    void** destructors = __destructors;
    while (*destructors) {
        __call_destructor(*destructors++);
    }
}

static void __argv_setup(void) {
    __argv = (char**)__process_info_table[__ONRAMP_PIT_ARGS];

    // The VM is allowed to leave argv NULL or point it to an empty
    // (null-terminated) array. We make sure there is always at least
    // a program name.
    if (__argv == NULL || __argv[0] == NULL) {
        // We don't have initializers or local static variables in opC so we
        // have to declare it above and initialize it here.
        empty_argv[0] = "<unnamed>";
        __argv = empty_argv;
    }

    // Count command-line args
    for (__argc = 0; __argv[__argc]; ++__argc) {}
}

void __environ_setup(void) {
    environ = (char**)__process_info_table[__ONRAMP_PIT_ENVIRON];

    // The VM is allowed to leave the environment NULL. We make sure it is at
    // least an empty array.
    if (environ == 0) {
        environ = empty_environment;
    }
}

void __time_setup(void) {
    // nothing
}

void __malloc_init(void) {
    char* heap_start;
    char* heap_end;

    // The heap start is the program break aligned to 32 bits.
    heap_start = *(__process_info_table + __ONRAMP_PIT_BREAK);

    // The heap end is the bottom of the stack. For now we reserve 32kB for
    // the stack. We just subtract it from the address of a local variable,
    // close enough.
    char* dummy;
    heap_end = ((char*)&dummy - 32768);

    // align start and end pointers
    heap_start = (char*)((int)(heap_start + 3) & (~3));
    heap_end = (char*)((int)heap_end & (~3));

    int heap_size = ((int)(heap_end - heap_start) >> 2);
    if (heap_size <= 2) {
        // no heap space, just return. the free list will be empty so malloc()
        // will fail with out-of-memory.
        return;
    }

    free_list = ((int*)heap_start + 1);
    *(free_list - 1) = (heap_size - 1);
    *free_list = 0;
    //printf("free list is at %p\n",&free_list);
}

void* malloc(size_t usize) {
    if (usize == 0) {
        usize = 1;
    }
//puts("--------");
//fputs("malloc ", stdout);
//putd(usize);
//fputc('\n', stdout);
    if (usize > (INT_MAX >> 2)) {
        return 0;
    }
    int size = (int)usize;

    // Convert to a number of ints rounded up.
    size = ((size + 3) >> 2);
    //printf("malloc() size %zi\n",size);

//fputs("malloc size in words: ", stdout);
//putd(size);
//fputc('\n', stdout);

    // Scan the list for the smallest allocation that fits. (In case of ties,
    // use the first one.)
    int* best = 0;            // the best allocation
    int best_size = INT_MAX;  // size of the best allocation
    int* best_link = 0;       // linked list pointer to the best allocation
    int* current_link = (int*)&free_list;
    while (*current_link != 0) {
        int* alloc = (int*)*current_link;
        int alloc_size = *(alloc - 1);

        if (alloc_size == size) {
            // We've found an exact match. Pop this allocation and return it.
//fputs("exact match: ", stdout);
//putd((int)alloc);
//fputc('\n', stdout);
            *current_link = *alloc;
            return alloc;
        }

        if ((alloc_size > size) & (alloc_size < best_size)) {
            // We've found an allocation large enough.
            best_link = current_link;
            best = alloc;
            best_size = alloc_size;
            // Keep searching for a better fit.
        }

        current_link = alloc;
    }

    // If none fit, we're out of memory.
    if (best == 0) {
        errno = ENOMEM;
        return NULL;
    }

//fputs("found: ", stdout);
//putd((int)best);
//fputc('\n', stdout);

    // We've found a free allocation but it's too large. We need at least 8
    // extra bytes to split it.
    if (best_size < (size + 8)) {
        // Can't split. Just pop and return.
//puts("can't split, returning");
        *best_link = *best;
        return best;
    }

    // Split it...
    int* remainder = ((best + size) + 1);
    //printf("best %p remainder %p\n",best,remainder);
    *(best - 1) = size;
    *(remainder - 1) = ((best_size - size) - 1);

    // ...and insert the remainder in the list.
    *best_link = (int)remainder;
    *remainder = *best;
//puts("split. returning");
    return best;
}

/*
 * A POSIX file descriptor.
 */
typedef struct posixfile_t {
    int fd;           // The POSIX file descriptor
    unsigned handle;  // The underlying Onramp file or directory handle
    unsigned flags;   // The flags with which the file was opened
    int is_dir;      // true if this is a directory
    int std_stream;  // true if this is a standard stream (which must not be closed)
    char* path;       // The path to the file
} posixfile_t;

static posixfile_t* posixfiles[32];
#define POSIXFILES_CAPACITY (int)(sizeof(posixfiles)/sizeof(*posixfiles))

// one of these is required
#define O_RDONLY     0x1
#define O_WRONLY     0x2
#define O_RDWR       0x4

void* memset(void* vdest, int c, size_t count) {
    /* TODO test this */
    // TODO vectorize
    unsigned char* restrict dest = (unsigned char*)vdest;
    unsigned char* start = dest;
    unsigned char* end = dest + count;
    unsigned char uc = (unsigned char)c;
    while (dest != end)
        *dest++ = uc;
    return start;
}

void* calloc(unsigned int count, unsigned int element_size) {
    // TODO overflow
    unsigned int size = count * element_size;
    void* ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

size_t strlen(const char* s) {
    const char* end = s;
    while (*end != 0)
        ++end;
    return (size_t)(end - s);
}

void* memmove(void* vdest, const void* vsrc, size_t count) {
    void* start = vdest;
    const unsigned char* src = (const unsigned char*)vsrc;
    unsigned char* dest = (unsigned char*)vdest;
    if (dest < src) {
        // TODO vectorize
        unsigned char* end = dest + count;
        while (dest != end)
            *dest++ = *src++;
    } else if (dest > src) {
        // TODO vectorize
        unsigned char* end = dest;
        dest += count;
        src += count;
        while (dest != end)
            *--dest = *--src;
    }
    return start;
}

void* memcpy(void* dest, const void* src, size_t count) {
    return memmove(dest, src, count);
}

char* strdup(const char* src) {
    size_t len = strlen(src);
    void* dest = malloc(len + 1);
    if (!dest) {
        // malloc() set errno
        return NULL;
    }
    memcpy(dest, src, len + 1);
    return dest;
}

static posixfile_t* posixfile_new(int fd, int handle, int flags, const char* path) {
    posixfile_t* posixfile = calloc(1, sizeof(posixfile_t));
    if (posixfile == NULL) {
        return NULL;
    }

    posixfile->fd = fd;
    posixfile->handle = handle;
    posixfile->flags = flags;
    posixfile->path = strdup(path);
    if (posixfile->path == NULL) {
        //free(posixfile);
        return NULL;
    }

    return posixfile;
}

void __io_init(void) {

    // By default we match POSIX
    input_echo = 1;
    input_block = 1;
    input_canonical = 1;

    // Create POSIX file descriptors
    posixfiles[0] = posixfile_new(0, __process_info_table[__ONRAMP_PIT_INPUT], O_RDONLY, "/dev/stdin");
    posixfiles[1] = posixfile_new(1, __process_info_table[__ONRAMP_PIT_OUTPUT], O_WRONLY, "/dev/stdout");
    posixfiles[2] = posixfile_new(2, __process_info_table[__ONRAMP_PIT_ERROR], O_WRONLY, "/dev/stderr");

    // Mark the standard streams so we don't close them
    posixfiles[0]->std_stream = 1;
    posixfiles[1]->std_stream = 1;
    posixfiles[2]->std_stream = 1;
}

ssize_t write(int fd, const void* buffer, size_t count) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    if (posixfile->is_dir || (posixfile->flags & O_RDONLY)) {
        errno = EINVAL; // this is not writeable
        return -1;
    }

    int result = __sys_fwrite(posixfile->handle, buffer, count);
    
    if (result < 0) {
        // TODO parse out the result code
        errno = EIO; // io error
        return -1;
    }

    return result;
}



void __start_c(unsigned* process_info, unsigned stack_base) {
    __process_info_table = process_info;
    __argv_setup();
    __environ_setup();
    __time_setup();
    __malloc_init(/*process_info[__ONRAMP_PIT_BREAK], stack_base*/);
    __io_init();      
    call_constructors();
    exit(main());
}