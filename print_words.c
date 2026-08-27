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

#define LINEFEED 0x0a

static char usage[] = "usage: print_words (-terse_modemode) (-verbose) (-exact_word_lenval) (-lower) (-upper)\n"
"  filename\n";

static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
static char word[MAX_WORD_LEN+1];
int word_len_counts[MAX_WORD_LEN-2];

int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper);
void compress(char *in_buf,int width,int height);
static int do_across(char *in_buf,int width,int height,int terse_mode,bool bVerbose,int *num_letters_pt,int exact_word_len);
static int do_down(char *in_buf,int width,int height,int terse_mode,bool bVerbose,int *num_letters_pt,int exact_word_len);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int terse_mode;
  bool bVerbose;
  int exact_word_len;
  int lower;
  int upper;
  int retval;
  char *in_buf;
  int width;
  int height;
  int total_words;
  int num_across_letters;
  int num_down_letters;
  int total_letters;

  if ((argc < 2) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  terse_mode = 0;
  bVerbose = false;
  exact_word_len = -1;
  lower = 0;
  upper = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-terse_mode",11))
      sscanf(&argv[curr_arg][11],"%d",&terse_mode);
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-exact_word_len",15))
      sscanf(&argv[curr_arg][15],"%d",&exact_word_len);
    else if (!strcmp(argv[curr_arg],"-lower"))
      lower = 1;
    else if (!strcmp(argv[curr_arg],"-upper"))
      upper = 1;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (terse_mode && bVerbose) {
    printf("can't specify both -terse_mode and -verbose\n");
    return 3;
  }

  if (lower + upper > 1) {
    printf("can't specify both -lower and -upper\n");
    return 4;
  }

  retval = read_grid(argv[curr_arg],&in_buf,&width,&height,lower,upper);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 5;
  }

  compress(in_buf,width,height);

  total_words = do_across(in_buf,width,height,terse_mode,bVerbose,&num_across_letters,exact_word_len);
  total_words += do_down(in_buf,width,height,terse_mode,bVerbose,&num_down_letters,exact_word_len);
  total_letters = num_across_letters + num_down_letters;

  if (bVerbose)
    printf("\ntotal_words = %d, total_letters = %d\n\n",total_words,total_letters);

  if ((terse_mode == 1) || bVerbose) {
    for (n = 0; n < MAX_WORD_LEN - 2; n++) {
      if (word_len_counts[n])
        printf("%2d %2d\n",word_len_counts[n],n+2);
    }
  }

  free(in_buf);

  return 0;
}

static int do_across(char *in_buf,int width,int height,int terse_mode,bool bVerbose,int *num_letters_pt,int exact_word_len)
{
  int m;
  int n;
  int num_words;
  int offset;
  bool bInWord;
  int word_len;
  int num_letters;

  if (!terse_mode)
    printf("Across\n\n");

  num_words = 0;
  num_letters = 0;

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
          num_words++;
          word[word_len] = 0;
          word_len_counts[word_len - 2]++;
          num_letters += word_len;

          if (!terse_mode || (terse_mode == 2)) {
            if ((exact_word_len == -1) || (word_len == exact_word_len)) {
              if (!bVerbose)
                printf("%s%s\n",((!terse_mode) ? "  " : ""),word);
              else
                printf("  %s (%d)\n",word,word_len);
            }
          }
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        num_words++;
        word[word_len] = 0;
        word_len_counts[word_len - 2]++;
        num_letters += word_len;

        if (!terse_mode || (terse_mode == 2)) {
          if ((exact_word_len == -1) || (word_len == exact_word_len)) {
            if (!bVerbose)
              printf("%s%s\n",((!terse_mode) ? "  " : ""),word);
            else
              printf("  %s (%d)\n",word,word_len);
          }
        }
      }
    }
  }

  if (bVerbose && (exact_word_len == -1))
    printf("\nnum_words = %d, num_letters = %d\n",num_words,num_letters);

  *num_letters_pt = num_letters;

  return num_words;
}

static int do_down(char *in_buf,int width,int height,int terse_mode,bool bVerbose,int *num_letters_pt,int exact_word_len)
{
  int m;
  int n;
  int num_words;
  bool bInWord;
  int word_len;
  int num_letters;

  if (!terse_mode)
    printf("\nDown\n\n");

  num_words = 0;
  num_letters = 0;

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
          num_words++;
          word[word_len] = 0;
          word_len_counts[word_len - 2]++;
          num_letters += word_len;

          if (!terse_mode || (terse_mode == 2)) {
            if ((exact_word_len == -1) || (word_len == exact_word_len)) {
              if (!bVerbose)
                printf("%s%s\n",((!terse_mode) ? "  " : ""),word);
              else
                printf("  %s (%d)\n",word,word_len);
            }
          }
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        num_words++;
        word[word_len] = 0;
        word_len_counts[word_len - 2]++;
        num_letters += word_len;

        if (!terse_mode || (terse_mode == 2)) {
          if ((exact_word_len == -1) || (word_len == exact_word_len)) {
            if (!bVerbose)
              printf("%s%s\n",((!terse_mode) ? "  " : ""),word);
            else
              printf("  %s (%d)\n",word,word_len);
          }
        }
      }
    }
  }

  if (bVerbose)
    printf("\nnum_words = %d, num_letters = %d\n",num_words,num_letters);

  *num_letters_pt = num_letters;

  return num_words;
}
