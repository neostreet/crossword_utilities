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
char word[MAX_WORD_LEN+1];
int word_len_counts[MAX_WORD_LEN-2];

int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper);
void compress(char *in_buf,int width,int height);
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

  retval = read_grid(argv[curr_arg],&in_buf,&width,&height,0,0);

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
