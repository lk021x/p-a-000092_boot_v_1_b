/*
 * fcntl.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Access constants for _open. Note that the permissions constants are
 * in sys/stat.h (ick).
 *
 */
#ifndef _FCNTL_H_
#define _FCNTL_H_
#include <stdbool.h>
#include <string.h>
// #define NULL  0
// #define BOOL  unsigned char
// #define TRUE  1
// #define FALSE 0
#define BOOL  bool
#define __IO  volatile                  /*!< defines 'read / write' permissions   */


/* POSIX/Non-ANSI names for increased portability */
/* Specifiy one of these flags to define the access mode. */
#define	O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_ACCMODE   (O_RDONLY|O_WRONLY|O_RDWR)

#define	O_APPEND    0x0008/* Writes will add to the end of the file. */
#define	O_CREAT     0x0100/* Create the file if it does not exist. */
#define	O_TRUNC     0x0200/* Truncate the file if it does exist. */
#define	O_EXCL      0x0400/* Open only if the file does not exist. */
#define	O_TEXT      0x4000/* CR-LF in file becomes LF in memory. */
#define	O_BINARY    0x8000/* Input and output is not translated. */
#define O_NONBLOCK	0x00004000
#define O_DSYNC		0x00010000 /* used to be O_SYNC, see below */
#define __O_SYNC	0x04000000
#define O_SYNC		(__O_SYNC|O_DSYNC)

/* Make the file dissappear after closing.
 * WARNING: Even if not created by _open! */
#define	O_TEMPORARY  0x0040
#define O_NOINHERIT  0x0080
#define O_SEQUENTIAL 0x0020
#define	O_RANDOM     0x0010

#define DSUCCESS     0
#define EFAULT      -1
#define EBUSY       -16

#define SEEK_SET     0
#define SEEK_CUR     1
#define SEEK_END     2


#endif	/* Not _FCNTL_H_ */

