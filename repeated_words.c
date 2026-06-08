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

static char usage[] = "usage: repeated_words (-verbose) filename\n";

static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
static char word[MAX_WORD_LEN+1];

static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt);
static void compress(char *in_buf,int width,int height);
static void do_across(char *in_buf,int width,int height,struct info_list *words);
static void do_down(char *in_buf,int width,int height,struct info_list *words);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  int retval;
  char *in_buf;
  int width;
  int height;
  struct info_list words;
  struct info_list_elem *work_elem;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  retval = read_grid(argv[curr_arg],&in_buf,&width,&height);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 3;
  }

  compress(in_buf,width,height);

  words.num_elems = 0;

  do_across(in_buf,width,height,&words);
  do_down(in_buf,width,height,&words);

  free(in_buf);

  work_elem = words.first_elem;

  for (n = 0; n < words.num_elems; n++) {
    if (work_elem->int1 > 1) {
      if (!bVerbose)
        printf("%s\n",work_elem->str);
      else
        printf("%d %s\n",work_elem->int1,work_elem->str);
    }

    work_elem = work_elem->next_elem;
  }

  free_info_list(&words);

  return 0;
}

static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt)
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

  for (n = 0; n < bytes_to_io; n++) {
    if ((in_buf[n] >= 'A') && (in_buf[n] <= 'Z'))
      in_buf[n] += ('a' - 'A');
  }

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

static void compress(char *in_buf,int width,int height)
{
  int m;
  int n;
  int p;

  m = width;
  n = width + 1;

  for (p = 0; p < (height - 1) * width; p++) {
    in_buf[m++] = in_buf[n++];

    if (in_buf[n] == LINEFEED)
      n++;
  }
}

static void do_across(char *in_buf,int width,int height,struct info_list *words)
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

          if (member_of_info_list(words,word,&ix)) {
            if (get_info_list_elem(words,ix,&work_elem)) {
              work_elem->int1++;
            }
          }
          else
            add_info_list_elem(words,word,1,0,0,0,true);
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        word[word_len] = 0;

        if (member_of_info_list(words,word,&ix)) {
          if (get_info_list_elem(words,ix,&work_elem)) {
            work_elem->int1++;
          }
        }
        else
          add_info_list_elem(words,word,1,0,0,0,true);
      }
    }
  }
}

static void do_down(char *in_buf,int width,int height,struct info_list *words)
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

          if (member_of_info_list(words,word,&ix)) {
            if (get_info_list_elem(words,ix,&work_elem)) {
              work_elem->int1++;
            }
          }
          else
            add_info_list_elem(words,word,1,0,0,0,true);
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        word[word_len] = 0;

        if (member_of_info_list(words,word,&ix)) {
          if (get_info_list_elem(words,ix,&work_elem)) {
            work_elem->int1++;
          }
        }
        else
          add_info_list_elem(words,word,1,0,0,0,true);
      }
    }
  }
}
