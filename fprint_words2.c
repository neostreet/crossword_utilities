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
static char outfilename[MAX_FILENAME_LEN];

static char usage[] = "usage: fprint_words2 (-verbose) (-lower) (-upper) (-theme_words) filename\n";
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
static bool is_a_theme_word(char *word);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  int lower;
  int upper;
  struct info_list across_words;
  struct info_list down_words;
  struct info_list_elem *work_elem;
  int theme_words;
  FILE *fptr0;
  int filename_len;
  int retval;
  char *in_buf;
  int width;
  int height;
  int num_across_words;
  int num_down_words;
  int total_words;
  int num_across_letters;
  int num_down_letters;
  int total_letters;
  bool bPrinted;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  lower = 0;
  upper = 0;
  theme_words = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-lower"))
      lower = 1;
    else if (!strcmp(argv[curr_arg],"-upper"))
      upper = 1;
    else if (!strcmp(argv[curr_arg],"-theme_words"))
      theme_words = 1;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (lower + upper + theme_words > 1) {
    printf("can't specify more than one of -lower, -upper, and -theme_words\n");
    return 3;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    printf("%s\n",filename);

    retval = read_grid(filename,&in_buf,&width,&height,lower,upper);

    if (retval) {
      printf("read_grid(() failed: %d\n",retval);
      continue;
    }

    compress(in_buf,width,height);

    across_words.num_elems = 0;
    down_words.num_elems = 0;

    do_across(in_buf,width,height,&across_words);
    do_down(in_buf,width,height,&down_words);

    bPrinted = false;
    num_across_words = 0;
    num_across_letters = 0;
    work_elem = across_words.first_elem;

    for (n = 0; n < across_words.num_elems; n++) {
      if (!theme_words || is_a_theme_word(work_elem->str)) {
        if (!bPrinted) {
          printf("  Across\n");
          bPrinted = true;
        }

        num_across_words++;
        num_across_letters += work_elem->int2;
        word_len_counts[work_elem->int2 - 2]++;

        if (!bVerbose)
          printf("    %s\n",work_elem->str);
        else
          printf("    %s (%d)\n",work_elem->str,work_elem->int2);
      }

      work_elem = work_elem->next_elem;
    }

    if (bVerbose && !theme_words)
      printf("    num_words = %d, num_letters = %d\n",num_across_words,num_across_letters);

    free_info_list(&across_words);

    bPrinted = false;
    num_down_words = 0;
    num_down_letters = 0;
    work_elem = down_words.first_elem;

    for (n = 0; n < down_words.num_elems; n++) {
      if (!theme_words || is_a_theme_word(work_elem->str)) {
        if (!bPrinted) {
          printf("  Down\n");
          bPrinted = true;
        }

        num_down_words++;
        num_down_letters += work_elem->int2;
        word_len_counts[work_elem->int2 - 2]++;

        if (!bVerbose)
          printf("    %s\n",work_elem->str);
        else
          printf("    %s (%d)\n",work_elem->str,work_elem->int2);
      }

      work_elem = work_elem->next_elem;
    }

    if (bVerbose && !theme_words)
      printf("    num_words = %d, num_letters = %d\n",num_down_words,num_down_letters);

    free_info_list(&down_words);

    total_words = num_across_words + num_down_words;
    total_letters = num_across_letters + num_down_letters;

    if (bVerbose)
      printf("\n  total_words = %d, total_letters = %d\n",total_words,total_letters);

    if (bVerbose) {
      for (n = 0; n < MAX_WORD_LEN - 2; n++) {
        if (word_len_counts[n])
          printf("  %2d %2d\n",word_len_counts[n],n+2);
      }

      putchar(0x0a);
    }

    free(in_buf);
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

static bool is_a_theme_word(char *word)
{
  int n;

  // theme words are all in caps

  for (n = 0; (word[n]); n++) {
    if ((word[n] < 'A') || (word[n] > 'Z'))
      return false;
  }

  // theme words must be at least three letters

  if (n < 3)
    return false;

  return true;
}
