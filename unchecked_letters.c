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

static int dbg_p;

static int unchecked_letters(char *filename,int bVerbose);
static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt);
static void compress(char *in_buf,int width,int height);
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
  int m;
  int n;
  int p;
  int retval;
  char *in_buf;
  int width;
  int height;
  int num_unchecked_letters;

  retval = read_grid(filename,&in_buf,&width,&height);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 1;
  }

  compress(in_buf,width,height);

  num_unchecked_letters = 0;
  p = 0;

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

  if (bVerbose)
    print_grid(in_buf,width,height);

  printf("num_unchecked_letters = %d\n",num_unchecked_letters);

  free(in_buf);

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

  bytes_to_io = mem_amount;

  if (read(fhndl,in_buf,bytes_to_io) != bytes_to_io) {
    printf(read_failed,filename,bytes_to_io);
    free(in_buf);
    close(fhndl);
    return 4;
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
