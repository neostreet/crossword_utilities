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

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

static char usage[] = "usage: frepeated_words (-verbose) filename\n";

static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
char word[MAX_WORD_LEN+1];
int word_len_counts[MAX_WORD_LEN-2];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper);
void compress(char *in_buf,int width,int height);
void do_across(char *in_buf,int width,int height,struct info_list *words);
void do_down(char *in_buf,int width,int height,struct info_list *words);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  FILE *fptr0;
  int filename_len;
  bool bVerbose;
  bool bPrinted;
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

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    retval = read_grid(filename,&in_buf,&width,&height,0,0);

    if (retval) {
      printf("read_grid(() of %s failed: %d\n",filename,retval);
      continue;
    }

    compress(in_buf,width,height);

    words.num_elems = 0;

    do_across(in_buf,width,height,&words);
    do_down(in_buf,width,height,&words);

    free(in_buf);

    work_elem = words.first_elem;

    bPrinted = false;

    for (n = 0; n < words.num_elems; n++) {
      if (work_elem->int1 > 1) {
        if (!bPrinted) {
          printf("%s\n",filename);
          bPrinted = true;
        }

        if (!bVerbose)
          printf("  %s\n",work_elem->str);
        else
          printf("  %d %s\n",work_elem->int1,work_elem->str);
      }

      work_elem = work_elem->next_elem;
    }

    free_info_list(&words);
  }

  fclose(fptr0);

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}
