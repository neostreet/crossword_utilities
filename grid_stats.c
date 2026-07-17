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

struct grid_shape {
  int width;
  int height;
  int count;
};

#define  MAX_GRID_SHAPES 20
static struct grid_shape grid_shapes[MAX_GRID_SHAPES];

#define LINEFEED 0x0a

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

static char usage[] = "usage: grid_stats (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int read_grid(char *filename,int *width_pt,int *height_pt);

int main(int argc,char **argv)
{
  int n;
  int retval;
  int curr_arg;
  bool bDebug;
  FILE *fptr0;
  int filename_len;
  int width;
  int height;
  int num_grid_shapes;

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

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  num_grid_shapes = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    retval = read_grid(filename,&width,&height);

    if (retval) {
      printf("read_grid(() failed: %d\n",retval);
      return 4;
    }

    for (n = 0; n < num_grid_shapes; n++) {
      if ((width == grid_shapes[n].width) && (height == grid_shapes[n].height))
        break;
    }

    if (n == num_grid_shapes) {
      if (num_grid_shapes == MAX_GRID_SHAPES) {
        printf("grid_shapes array is too small\n");
        return 5;
      }

      grid_shapes[n].width = width;
      grid_shapes[n].height = height;
      grid_shapes[n].count = 1;
      num_grid_shapes++;
    }
    else
      grid_shapes[n].count++;

  }

  fclose(fptr0);

  for (n = 0; n < num_grid_shapes; n++)
    printf("%3d %d X %d\n",grid_shapes[n].count,grid_shapes[n].width,grid_shapes[n].height);

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

static int read_grid(char *filename,int *width_pt,int *height_pt)
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
  free(in_buf);

  *width_pt = width;
  *height_pt = height;

  return 0;
}
