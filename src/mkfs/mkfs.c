// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Based on XV6 (RISC-V)
// Copyright (c) 2006-2024 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <fs.h>

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE / BPB + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGBLOCKS + 1; // Header followed by LOGBLOCKS data blocks.
int nmeta;                // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;              // Number of data blocks

int fsfd;
struct superblock sb;
char zeroes[BSIZE];
uint32_t freeinode = 1;
uint32_t freeblock;

void balloc(int);
void wsect(uint32_t, void*);
void winode(uint32_t, struct dinode*);
void rinode(uint32_t inum, struct dinode* ip);
void rsect(uint32_t sec, void* buf);
uint32_t ialloc(uint16_t type);
void iappend(uint32_t inum, void* p, int n);
void die(const char*);

// convert to riscv byte order
uint16_t xshort(uint16_t x) {
    uint16_t y;
    uint8_t* a = (uint8_t *)&y;
    a[0] = x;
    a[1] = x >> 8;
    return y;
}

uint32_t xint(uint32_t x) {
    uint32_t y;
    uint8_t* a = (uint8_t *)&y;
    a[0] = x;
    a[1] = x >> 8;
    a[2] = x >> 16;
    a[3] = x >> 24;
    return y;
}

int main(int argc, char *argv[]) {
    int i, cc, fd;
    uint32_t rootino, inum, off;
    struct dirent de;
    char buf[BSIZE];
    struct dinode din;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: mkfs fs.img files...\n");
        exit(1);
    }

    //assert((BSIZE % sizeof(struct dinode)) == 0);
    //assert((BSIZE % sizeof(struct dirent)) == 0);

    fsfd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fsfd < 0)
        die(argv[1]);

    // 1 fs block = 1 disk sector
    nmeta = 2 + nlog + ninodeblocks + nbitmap;
    nblocks = FSSIZE - nmeta;

    sb.magic = FSMAGIC;
    sb.size = xint(FSSIZE);
    sb.nblocks = xint(nblocks);
    sb.ninodes = xint(NINODES);
    sb.nlog = xint(nlog);
    sb.logstart = xint(2);
    sb.inodestart = xint(2 + nlog);
    sb.bmapstart = xint(2 + nlog + ninodeblocks);

    printf("nmeta %d (boot, super, log blocks %u, inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
           nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

    freeblock = nmeta; // the first free block that we can allocate

    for (i = 0; i < FSSIZE; i++)
        wsect(i, zeroes);

    memset(buf, 0, sizeof(buf));
    memmove(buf, &sb, sizeof(sb));
    wsect(1, buf);

    rootino = ialloc(T_DIR);
    //assert(rootino == ROOTINO);

    bzero(&de, sizeof(de));
    de.inum = xshort(rootino);
    strcpy(de.name, ".");
    iappend(rootino, &de, sizeof(de));

    bzero(&de, sizeof(de));
    de.inum = xshort(rootino);
    strcpy(de.name, "..");
    iappend(rootino, &de, sizeof(de));

    for (i = 2; i < argc; i++)
    {
        // get rid of "user/"
        char *shortname;
        if (strncmp(argv[i], "user/", 5) == 0)
            shortname = argv[i] + 5;
        else
            shortname = argv[i];

        //assert(index(shortname, '/') == 0);

        if ((fd = open(argv[i], O_RDONLY)) < 0)
            die(argv[i]);

        // Skip leading _ in name when writing to file system.
        // The binaries are named _rm, _cat, etc. to keep the
        // build operating system from trying to execute them
        // in place of system binaries like rm and cat.
        if (shortname[0] == '_')
            shortname += 1;

        //assert(strlen(shortname) <= DIRSIZ);

        inum = ialloc(T_FILE);

        bzero(&de, sizeof(de));
        de.inum = xshort(inum);
        strncpy(de.name, shortname, DIRSIZ);
        iappend(rootino, &de, sizeof(de));

        while ((cc = read(fd, buf, sizeof(buf))) > 0)
            iappend(inum, buf, cc);

        close(fd);
    }

    // fix size of root inode dir
    rinode(rootino, &din);
    off = xint(din.size);
    off = ((off / BSIZE) + 1) * BSIZE;
    din.size = xint(off);
    winode(rootino, &din);

    balloc(freeblock);

    exit(0);
}

ssize_t readall(int fd, void *buf, size_t n) {
    size_t nn = n;
    do {
        ssize_t c = read(fd, buf, nn);
        if (c < 0)
            return c;
        nn -= (size_t)c;
        buf += (size_t)c;
    } while (nn > 0);
    return n;
}

ssize_t writeall(int fd, const void *buf, size_t n) {
    size_t nn = n;
    do {
        ssize_t c = write(fd, buf, nn);
        if (c < 0)
            return c;
        nn -= (size_t)c;
        buf += (size_t)c;
    } while (nn > 0);
    return n;
}

void wsect(uint32_t sec, void *buf) {
    if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
        die("lseek");
    if (writeall(fsfd, buf, BSIZE) != BSIZE)
        die("write");
}

void winode(uint32_t inum, struct dinode* ip) {
    char buf[BSIZE];
    uint32_t bn;
    struct dinode* dip;

    bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    wsect(bn, buf);
}

void rinode(uint32_t inum, struct dinode* ip) {
    char buf[BSIZE];
    uint32_t bn;
    struct dinode *dip;

    bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    dip = ((struct dinode *)buf) + (inum % IPB);
    *ip = *dip;
}

void rsect(uint32_t sec, void* buf) {
    if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
        die("lseek");
    if (readall(fsfd, buf, BSIZE) != BSIZE)
        die("read");
}

uint32_t ialloc(uint16_t type) {
    uint32_t inum = freeinode++;
    struct dinode din;

    bzero(&din, sizeof(din));
    din.type = xshort(type);
    din.nlink = xshort(1);
    din.size = xint(0);
    winode(inum, &din);
    return inum;
}

void balloc(int used) {
    uint8_t buf[BSIZE];
    int i;

    printf("balloc: first %d blocks have been allocated\n", used);
    //assert(used < BPB);
    bzero(buf, BSIZE);
    for (i = 0; i < used; i++)
    {
        buf[i / 8] = buf[i / 8] | (0x1 << (i % 8));
    }
    printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
    wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void iappend(uint32_t inum, void* xp, int n) {
    char* p = (char *)xp;
    uint32_t fbn, off, n1;
    struct dinode din;
    char buf[BSIZE];
    uint32_t indirect[NINDIRECT];
    uint32_t x;

    rinode(inum, &din);
    off = xint(din.size);
    // printf("append inum %d at off %d sz %d\n", inum, off, n);
    while (n > 0)
    {
        fbn = off / BSIZE;
        //assert(fbn < MAXFILE);
        if (fbn < NDIRECT)
        {
            if (xint(din.addrs[fbn]) == 0)
            {
                din.addrs[fbn] = xint(freeblock++);
            }
            x = xint(din.addrs[fbn]);
        }
        else
        {
            if (xint(din.addrs[NDIRECT]) == 0)
            {
                din.addrs[NDIRECT] = xint(freeblock++);
            }
            rsect(xint(din.addrs[NDIRECT]), (char *)indirect);
            if (indirect[fbn - NDIRECT] == 0)
            {
                indirect[fbn - NDIRECT] = xint(freeblock++);
                wsect(xint(din.addrs[NDIRECT]), (char *)indirect);
            }
            x = xint(indirect[fbn - NDIRECT]);
        }
        n1 = min(n, (fbn + 1) * BSIZE - off);
        rsect(x, buf);
        bcopy(p, buf + off - (fbn * BSIZE), n1);
        wsect(x, buf);
        n -= n1;
        off += n1;
        p += n1;
    }
    din.size = xint(off);
    winode(inum, &din);
}

void die(const char* s) {
    printf("Error: %s\n", s);
    //perror(s);
    exit(1);
}
