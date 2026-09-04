#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#ifndef CYGWIN
#define O_BINARY 0
#endif
#endif
#include "str_list.h"

#define LINEFEED 0x0a

static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
extern char word[];
extern int word_len_counts[];

int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper)
{
  int m;
  int n;
  struct stat statbuf;
  off_t mem_amount;
  char *in_buf;
  int in_buf_ix;
  int fhndl;
  int bytes_to_io;
  int width;
  int height;
  int save_width;

  if (stat(filename,&statbuf) == -1) {
    printf(couldnt_get_status,filename);
    return 1;
  }

  mem_amount = (size_t)statbuf.st_size;

  if ((in_buf = (char *)malloc(mem_amount)) == NULL) {
    printf(malloc_failed,mem_amount);
    return 2;
  }

  if ((fhndl = open(filename,O_BINARY | O_RDONLY,0)) == -1) {
    printf(couldnt_open,filename);
    free(in_buf);
    return 3;
  }

  bytes_to_io = (int)mem_amount;

  if (read(fhndl,in_buf,bytes_to_io) != bytes_to_io) {
    printf(read_failed,filename,bytes_to_io);
    free(in_buf);
    close(fhndl);
    return 4;
  }

  if (lower) {
    for (n = 0; n < bytes_to_io; n++) {
      if ((in_buf[n] >= 'A') && (in_buf[n] <= 'Z'))
        in_buf[n] += ('a' - 'A');
    }
  }

  if (upper) {
    for (n = 0; n < bytes_to_io; n++) {
      if ((in_buf[n] >= 'a') && (in_buf[n] <= 'z'))
        in_buf[n] -= ('a' - 'A');
    }
  }

  for (n = 0; n < MAX_WORD_LEN - 2; n++)
    word_len_counts[n] = 0;

  height = 0;
  m = 0;

  for (n = 0; n < bytes_to_io; n++) {
    if (in_buf[n] == LINEFEED) {
      width = n - m;
      m = n + 1;
      height++;

      if (height == 1) {
        save_width = width;
        continue;
      }

      if (width != save_width) {
        printf("length of line %d doesn't conform\n",height);
        free(in_buf);
        close(fhndl);
        return 5;
      }
    }
  }

  close(fhndl);

  *in_buf_pt = in_buf;
  *width_pt = width;
  *height_pt = height;

  return 0;
}

void compress(char *in_buf,int width,int height)
{
  int m;
  int n;
  int p;
  int area;
  int dbg;

  area = width * height;

  m = width;
  n = width + 1;

  for (p = 0; p < (height - 1) * width; p++) {
    in_buf[m++] = in_buf[n++];

    if (in_buf[n] == LINEFEED)
      n++;
  }

  if (m == area)
    dbg = 1;
}

void do_across(char *in_buf,int width,int height,struct info_list *words)
{
  int m;
  int n;
  int offset;
  bool bInWord;
  int word_len;
  struct info_list_elem *work_elem;
  int ix;

  for (m = 0; m < height; m++) {
    offset = m * width;
    bInWord = false;

    for (n = 0; n < width; n++) {
      if (in_buf[offset + n] != '.') {
        if (!bInWord) {
          bInWord = true;
          word_len = 0;
        }

        word[word_len++] = in_buf[offset + n];
      }
      else if (bInWord) {
        if (word_len > 1) {
          word[word_len] = 0;
          add_info_list_elem(words,word,1,word_len,0,0,false);
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        word[word_len] = 0;
        add_info_list_elem(words,word,1,word_len,0,0,false);
      }
    }
  }
}

void do_down(char *in_buf,int width,int height,struct info_list *words)
{
  int m;
  int n;
  bool bInWord;
  int word_len;
  struct info_list_elem *work_elem;
  int ix;

  for (m = 0; m < width; m++) {
    bInWord = false;

    for (n = 0; n < height; n++) {
      if (in_buf[m + n * width] != '.') {
        if (!bInWord) {
          bInWord = true;
          word_len = 0;
        }

        word[word_len++] = in_buf[m + n * width];
      }
      else if (bInWord) {
        if (word_len > 1) {
          word[word_len] = 0;
          add_info_list_elem(words,word,1,word_len,0,0,false);
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        word[word_len] = 0;
        add_info_list_elem(words,word,1,word_len,0,0,false);
      }
    }
  }
}
