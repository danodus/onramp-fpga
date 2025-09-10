// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Based on XV6 (RISC-V)
// Copyright (c) 2006-2024 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology

#include <fs.h>
#include "globals.h"
#include "common.h"
#include "buf.h"

// Read the super block.
static void readsb(int dev, struct superblock* sb) {
    struct buf* bp;

    bp = bread(dev, 1);
    memmove(sb, bp->data, sizeof(*sb));
}

// Init fs
void fsinit(int dev) {
    bios_globals_t* g = (bios_globals_t*)BIOS_GLOBALS;

    readsb(dev, &g->sb);
    if(g->sb.magic != FSMAGIC)
        panic("invalid file system");
}

/*
// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode* namex(char *path, int nameiparent, char *name) {
  struct inode *ip, *next;

  if(*path == '/')
    ip = iget(ROOTDEV, ROOTINO);
  else
    ip = idup(myproc()->cwd);

  while((path = skipelem(path, name)) != 0){
    ilock(ip);
    if(ip->type != T_DIR){
      iunlockput(ip);
      return 0;
    }
    if(nameiparent && *path == '\0'){
      // Stop one level early.
      iunlock(ip);
      return ip;
    }
    if((next = dirlookup(ip, name, 0)) == 0){
      iunlockput(ip);
      return 0;
    }
    iunlockput(ip);
    ip = next;
  }
  if(nameiparent){
    iput(ip);
    return 0;
  }
  return ip;
}
*/

struct inode* namei(const char* path) {
//   char name[DIRSIZ];
//   return namex(path, 0, name);
    return 0;
}