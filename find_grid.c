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

static char usage[] = "usage: find_grid (-debug) width height filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
int word_len_counts[MAX_WORD_LEN-2];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt,int lower,int upper);

int main(int argc,char **argv)
{
  int n;
  int retval;
  char *in_buf;
  int curr_arg;
  bool bDebug;
  FILE *fptr0;
  int filename_len;
  int width;
  int height;
  int curr_width;
  int curr_height;

  if ((argc < 4) || (argc > 5)) {
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

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&width);
  sscanf(argv[curr_arg+1],"%d",&height);

  if ((fptr0 = fopen(argv[curr_arg+2],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+2]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    retval = read_grid(filename,&in_buf,&curr_width,&curr_height,0,0);

    if (retval) {
      printf("read_grid(() failed: %d\n",retval);
      return 4;
    }

    if ((curr_width == width) && (curr_height == height))
      printf("%s\n",filename);

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
