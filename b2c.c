/* Copyright (c) 2012 Karel Sedlàček <k@ksdlck.com>. All rights reserved.
 * See LICENSE for details.
 */

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void
help(void) {
  printf(
    "usage: b2c [options]\n"
    "\n"
    "-h                     show this help\n"
    "-l <line length>       line length\n"
    "-n <name>              array name and length prefix\n"
    "-s <suffix>            length suffix (default: \"len\")\n"
    "-q <qualifiers>        array and length qualifiers\n"
    "-p                     print length\n"
    "-f <character format>  printf format for each character\n"
    "-t <length type>       length type (default: size_t)\n"
    "-b <buffer length>     input buffer length\n"
    );
}

#define D_LINE_LEN 8
#define D_NAM "out"
#define D_LEN_SFX "_len"
#define D_QUALS ""
#define D_PRINT_LEN 0
#define D_CHAR_FMT "'\\x%02X'"
#define D_LEN_TYP "size_t"
#define D_BUF_LEN 4096

int
main(const int argc, char **argv)
{
  long line_len, buf_len;
  int print_len;
  const char *nam, *len_sfx, *quals, *char_fmt, *len_typ, *quals_spc;

  size_t tc;
  int i, r;
  char *buf;

  line_len = D_LINE_LEN;
  nam = D_NAM;
  len_sfx = D_LEN_SFX;
  quals = D_QUALS;
  print_len = D_PRINT_LEN;
  char_fmt = D_CHAR_FMT;
  len_typ = D_LEN_TYP;
  buf_len = D_BUF_LEN;

  opterr = 0; /* TODO remove? */
  while (-1 != (r = getopt(argc, argv, "hl:n:s:q:pf:t:b:"))) {
    switch(r) {
      case 'h':
        help();
        return 0;
        break;
      case 'l':
        line_len = strtol(optarg, NULL, 10);
        if (EINVAL == errno || ERANGE == errno || 1 > line_len ||
            INT_MAX < line_len) {
          fprintf(stderr,
            "error: -l takes a reasonably-bounded integer in decimal >= 1\n");
          return __LINE__;
        }
        break;
      case 'n':
        nam = optarg;
        break;
      case 's':
        len_sfx = optarg;
        break;
      case 'q':
        quals = optarg;
        break;
      case 'p':
        print_len = 1;
        break;
      case 'f':
        char_fmt = optarg;
        break;
      case 't':
        len_typ = optarg;
        break;
      case 'b':
        buf_len = strtoul(optarg, NULL, 10);
        if (EINVAL == errno || ERANGE == errno || 0 > buf_len ||
            INT_MAX < buf_len) {
          fprintf(stderr,
            "error: -b takes a reasonably-bounded integer in decimal >= 1\n");
          return __LINE__;
        }
        break;
      case '?':
        fprintf(stderr,
          "error: unknown or poorly-used option -%c, use -h for help\n",
          optopt);
        return __LINE__;
        break;
      default:
        fprintf(stderr, "error: option parser internal failure\n");
        return __LINE__;
    }
  }

  buf = (char *)malloc(buf_len * sizeof(char));
  if (NULL == buf) {
    fprintf(stderr, "error: failed to allocate input buffer\n");
    return __LINE__;
  }

  quals_spc = 0 < strlen(quals) ? " " : "";

  printf("%s%schar %s[] = {\n  ", quals, quals_spc, nam);

  for (tc = 0;;) {
    r = read(0, buf, buf_len);
    if (0 > r) {
      perror("read");
      return __LINE__;
    }

    if (0 == r)
      break;

    for (i = 0; i < r; i++, tc++) {
      if (0 < tc) {
        if (tc % line_len == 0)
          printf(",\n  ");
        else
          printf(", ");
      }
      printf(char_fmt, 0xff & buf[i]);
    }
  }

  printf("\n};\n");

  if (print_len)
    printf("%s%s%s %s%s = %zu;\n", quals, quals_spc, len_typ, nam, len_sfx,
      tc);

  return 0;
}
