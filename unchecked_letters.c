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

static char usage[] = "usage: unchecked_letters (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
int word_len_counts[MAX_WORD_LEN-2];

static int dbg_p;

static int unchecked_letters(char *filename,int bVerbose);
int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper);
void compress(char *in_buf,int width,int height);
static int count_unchecked_letters(char *in_buf,int width,int height);
static bool unchecked_letter(char *in_buf,int width,int height,int m,int n,int p);
static void print_grid(char *grid,int width,int height);

int main(int argc,char **argv)
{
  int retval;
  int curr_arg;
  int bVerbose;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = 0;

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

  retval = unchecked_letters(argv[curr_arg],bVerbose);

  if (retval) {
    printf("unchecked_letters of %s failed: %d\n",argv[curr_arg],retval);
    return 3;
  }

  return 0;
}

static int unchecked_letters(char *filename,int bVerbose)
{
  int retval;
  char *in_buf;
  int width;
  int height;
  int num_unchecked_letters;

  retval = read_grid(filename,&in_buf,&width,&height,0,0);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 1;
  }

  compress(in_buf,width,height);
  num_unchecked_letters = count_unchecked_letters(in_buf,width,height);

  if (bVerbose)
    print_grid(in_buf,width,height);

  printf("num_unchecked_letters = %d\n",num_unchecked_letters);

  free(in_buf);

  return 0;
}

static int count_unchecked_letters(char *in_buf,int width,int height)
{
  int m;
  int n;
  int p;
  int num_unchecked_letters;

  p = 0;
  num_unchecked_letters = 0;

  for (m = 0; m < height; m++) {
    for (n = 0; n < width; n++) {
      if (in_buf[p] != '.') {
        if (!unchecked_letter(in_buf,width,height,m,n,p))
          in_buf[p] = ' ';
        else
          num_unchecked_letters++;
      }

      p++;
    }
  }

  return num_unchecked_letters;
}

static bool unchecked_letter(char *in_buf,int width,int height,int m,int n,int p)
{
  bool bHaveAcross;
  bool bHaveDown;
  int dbg;

  if (p == dbg_p)
    dbg = 1;

  bHaveAcross = false;

  if (n > 0) {
    if (in_buf[p - 1] != '.')
      bHaveAcross = true;
  }

  if (!bHaveAcross) {
    if (n < width - 1) {
      if (in_buf[p + 1] != '.')
        bHaveAcross = true;
    }
  }

  if (!bHaveAcross)
    return true;

  bHaveDown = false;

  if (m > 0) {
    if (in_buf[p - width] != '.')
      bHaveDown = true;
  }

  if (!bHaveDown) {
    if (m < height - 1) {
      if (in_buf[p + width] != '.')
        bHaveDown = true;
    }
  }

  if (!bHaveDown)
    return true;

  return false;
}

static void print_grid(char *grid,int width,int height)
{
  int m;
  int n;
  int p;

  p = 0;

  for (m = 0; m < height; m++) {
    for (n = 0; n < width; n++)
      putchar(grid[p++]);

    putchar(0x0a);
  }
}
