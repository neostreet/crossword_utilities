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

static char usage[] = "usage: transpose_grid (-debug) filename\n";
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

int main(int argc,char **argv)
{
  int m;
  int n;
  int retval;
  char *in_buf;
  int curr_arg;
  bool bDebug;
  int width;
  int height;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
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

  for (m = 0; m < width; m++) {
    for (n = 0; n < height; n++) {
      putchar(in_buf[m + n * width]);
    }

    putchar(0x0a);
  }

  free(in_buf);

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
