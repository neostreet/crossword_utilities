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

static char usage[] = "usage: print_words (-verbose) filename\n";

static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
static char word[MAX_WORD_LEN+1];

static int do_across(char *in_buf,int num_lines,int line_len,bool bVerbose);
static int do_down(char *in_buf,int num_lines,int line_len,bool bVerbose);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bVerbose;
  struct stat statbuf;
  off_t mem_amount;
  char *in_buf;
  int in_buf_ix;
  int fhndl;
  int bytes_to_io;
  int num_lines;
  int line_len;
  int save_line_len;
  int total_words;

  if ((argc < 2) || (argc >3)) {
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

  if (stat(argv[curr_arg],&statbuf) == -1) {
    printf(couldnt_get_status,argv[curr_arg]);
    return 3;
  }

  mem_amount = (size_t)statbuf.st_size;

  if ((in_buf = (char *)malloc(mem_amount)) == NULL) {
    printf(malloc_failed,mem_amount);
    return 3;
  }

  if ((fhndl = open(argv[curr_arg],O_BINARY | O_RDONLY,0)) == -1) {
    printf(couldnt_open,argv[curr_arg]);
    free(in_buf);
    return 4;
  }

  bytes_to_io = (int)mem_amount;

  if (read(fhndl,in_buf,bytes_to_io) != bytes_to_io) {
    printf(read_failed,argv[curr_arg],bytes_to_io);
    free(in_buf);
    close(fhndl);
    return 5;
  }

  num_lines = 0;
  m = 0;

  for (n = 0; n < bytes_to_io; n++) {
    if (in_buf[n] == LINEFEED) {
      line_len = n - m;
      m = n + 1;
      num_lines++;

      if (num_lines == 1) {
        save_line_len = line_len;
        continue;
      }

      if (line_len != save_line_len) {
        printf("length of line %d doesn't conform\n",num_lines);
        return 3;
      }
    }
  }

  total_words = do_across(in_buf,num_lines,save_line_len,bVerbose);
  total_words += do_down(in_buf,num_lines,save_line_len,bVerbose);

  free(in_buf);

  close(fhndl);

  return 0;
}

static int do_across(char *in_buf,int num_lines,int line_len,bool bVerbose)
{
  int m;
  int n;
  int num_words;
  int offset;
  bool bInWord;
  int word_len;
  int total_letters;

  printf("Across\n\n");

  num_words = 0;

  if (bVerbose)
    total_letters = 0;

  for (m = 0; m < num_lines; m++) {
    offset = m * (line_len + 1);
    bInWord = false;

    for (n = 0; n < line_len; n++) {
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

          if (!bVerbose)
            printf("  %s\n",word);
          else {
            printf("  %s (%d)\n",word,word_len);
            total_letters += word_len;
          }
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        num_words++;
        word[word_len] = 0;

        if (!bVerbose)
          printf("  %s\n",word);
        else {
          printf("  %s (%d)\n",word,word_len);
          total_letters += word_len;
        }
      }
    }
  }

  if (bVerbose)
    printf("\nnum_words = %d, total_letters = %d\n",num_words,total_letters);

  return num_words;
}

static int do_down(char *in_buf,int num_lines,int line_len,bool bVerbose)
{
  int m;
  int n;
  int num_words;
  bool bInWord;
  int word_len;
  int total_letters;

  printf("\nDown\n\n");

  num_words = 0;

  if (bVerbose)
    total_letters = 0;

  for (m = 0; m < line_len; m++) {
    bInWord = false;

    for (n = 0; n < num_lines; n++) {
      if (in_buf[m + n * (line_len + 1)] != '.') {
        if (!bInWord) {
          bInWord = true;
          word_len = 0;
        }

        word[word_len++] = in_buf[m + n * (line_len + 1)];
      }
      else if (bInWord) {
        if (word_len > 1) {
          num_words++;
          word[word_len] = 0;

          if (!bVerbose)
            printf("  %s\n",word);
          else {
            printf("  %s (%d)\n",word,word_len);
            total_letters += word_len;
          }
        }

        bInWord = false;
      }
    }

    if (bInWord) {
      if (word_len > 1) {
        num_words++;
        word[word_len] = 0;

        if (!bVerbose)
          printf("  %s\n",word);
        else {
          printf("  %s (%d)\n",word,word_len);
          total_letters += word_len;
        }
      }
    }
  }

  if (bVerbose)
    printf("\nnum_words = %d, total_letters = %d\n",num_words,total_letters);

  return num_words;
}
