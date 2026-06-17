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

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];
static char outfilename[MAX_FILENAME_LEN];

static char usage[] = "usage: fprint_words2 (-verbose) (-lower) (-theme_words) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
static char word[MAX_WORD_LEN+1];
static int word_len_counts[MAX_WORD_LEN-2];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,bool bLower);
static void compress(char *in_buf,int width,int height);
static int do_across(char *in_buf,int width,int height,bool bVerbose,int *num_letters_pt,bool bThemeWords);
static int do_down(char *in_buf,int width,int height,bool bVerbose,int *num_letters_pt,bool bThemeWords);
static bool is_a_theme_word(char *word);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  bool bLower;
  bool bThemeWords;
  FILE *fptr0;
  int filename_len;
  int retval;
  char *in_buf;
  int width;
  int height;
  int total_words;
  int num_across_letters;
  int num_down_letters;
  int total_letters;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bLower = false;
  bThemeWords = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-lower"))
      bLower = true;
    else if (!strcmp(argv[curr_arg],"-theme_words"))
      bThemeWords = true;
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

    printf("%s\n",filename);

    retval = read_grid(filename,&in_buf,&width,&height,bLower);

    if (retval) {
      printf("read_grid(() failed: %d\n",retval);
      continue;
    }

    compress(in_buf,width,height);

    total_words = do_across(in_buf,width,height,bVerbose,&num_across_letters,bThemeWords);
    total_words += do_down(in_buf,width,height,bVerbose,&num_down_letters,bThemeWords);
    total_letters = num_across_letters + num_down_letters;

    if (bVerbose)
      printf("\n  total_words = %d, total_letters = %d\n\n",total_words,total_letters);

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

static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,bool bLower)
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

  if (bLower) {
    for (n = 0; n < bytes_to_io; n++) {
      if ((in_buf[n] >= 'A') && (in_buf[n] <= 'Z'))
        in_buf[n] += ('a' - 'A');
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

static int do_across(char *in_buf,int width,int height,bool bVerbose,int *num_letters_pt,bool bThemeWords)
{
  int m;
  int n;
  int num_words;
  int offset;
  bool bInWord;
  int word_len;
  int num_letters;
  bool bPrinted;

  bPrinted = false;

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

          if (!bThemeWords || is_a_theme_word(word)) {
            if (!bPrinted) {
              printf("  Across\n\n");
              bPrinted = true;
            }

            if (!bVerbose)
              printf("    %s\n",word);
            else
              printf("    %s (%d)\n",word,word_len);
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

        if (!bThemeWords || is_a_theme_word(word)) {
          if (!bPrinted) {
            printf("  Across\n\n");
            bPrinted = true;
          }

          if (!bVerbose)
            printf("    %s\n",word);
          else
            printf("    %s (%d)\n",word,word_len);
        }
      }
    }
  }

  if (bPrinted) {
    putchar(0x0a);

    if (bVerbose)
      printf("    num_words = %d, num_letters = %d\n\n",num_words,num_letters);

    *num_letters_pt = num_letters;
  }

  return num_words;
}

static int do_down(char *in_buf,int width,int height,bool bVerbose,int *num_letters_pt,bool bThemeWords)
{
  int m;
  int n;
  int num_words;
  bool bInWord;
  int word_len;
  int num_letters;
  bool bPrinted;

  bPrinted = false;

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

          if (!bThemeWords || is_a_theme_word(word)) {
            if (!bPrinted) {
              printf("  Down\n\n");
              bPrinted = true;
            }

            if (!bVerbose)
              printf("    %s\n",word);
            else
              printf("    %s (%d)\n",word,word_len);
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

        if (!bThemeWords || is_a_theme_word(word)) {
          if (!bPrinted) {
            printf("  Down\n\n");
            bPrinted = true;
          }

          if (!bVerbose)
            printf("    %s\n",word);
          else
            printf("    %s (%d)\n",word,word_len);
        }
      }
    }
  }

  if (bPrinted) {
    putchar(0x0a);

    if (bVerbose)
      printf("    num_words = %d, num_letters = %d\n",num_words,num_letters);
  }

  *num_letters_pt = num_letters;

  return num_words;
}

static bool is_a_theme_word(char *word)
{
  int n;

  // theme words are all in caps

  for (n = 0; (word[n]); n++) {
    if ((word[n] < 'A') || (word[n] > 'Z'))
      return false;
  }

  return true;
}
