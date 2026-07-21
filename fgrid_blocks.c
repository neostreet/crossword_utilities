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

#define MAX_FILES 100
#define MAX_FILENAME_LEN 256

static char filename[MAX_FILENAME_LEN];

struct filename_info {
  char filename[MAX_FILENAME_LEN];
  int blocks;
  int puzzle_size;
  double block_pct;
};

static struct filename_info filename_inf[MAX_FILES];
static int num_files;
static int ixs[MAX_FILES];

static char usage[] = "usage: fgrid_blocks (-sort) (-ascending) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

static bool bAscending;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int grid_info(char *filename);
static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt);
static void compress(char *in_buf,int width,int height);
static int count_blocks(char *in_buf,int puzzle_size);
static int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int n;
  int retval;
  int curr_arg;
  int bSort;
  FILE *fptr0;
  int filename_len;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bSort = false;
  bAscending = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-sort"))
      bSort = true;
    else if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = true;
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

  num_files = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if (num_files == MAX_FILES) {
      printf("limit of %d files exceeded\n",MAX_FILES);
      return 4;
    }

    retval = grid_info(filename);

    if (retval) {
      printf("grid_info of %s failed: %d\n",filename,retval);
      return 5;
    }

    num_files++;
  }

  fclose(fptr0);

  for (n = 0; n < num_files; n++)
    ixs[n] = n;

  if (bSort)
    qsort(ixs,num_files,sizeof (int),elem_compare);

  for (n = 0; n < num_files; n++) {
    printf("%6.2lf%% blocks (%d %d) %s\n",
      filename_inf[ixs[n]].block_pct,
      filename_inf[ixs[n]].blocks,
      filename_inf[ixs[n]].puzzle_size,
      filename_inf[ixs[n]].filename);
  }

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

static int grid_info(char *filename)
{
  int m;
  int n;
  int p;
  int retval;
  char *in_buf;
  int width;
  int height;
  int puzzle_size;
  int blocks;
  double block_pct;

  retval = read_grid(filename,&in_buf,&width,&height);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 1;
  }

  compress(in_buf,width,height);

  puzzle_size = width * height;
  blocks = count_blocks(in_buf,puzzle_size);

  strcpy(filename_inf[num_files].filename,filename);
  filename_inf[num_files].blocks = blocks;
  filename_inf[num_files].puzzle_size = puzzle_size;
  filename_inf[num_files].block_pct = (double)blocks / (double)puzzle_size * (double)100;

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

static int count_blocks(char *in_buf,int puzzle_size)
{
  int n;
  int blocks;

  blocks = 0;

  for (n = 0; n < puzzle_size; n++) {
    if (in_buf[n] == '.')
      blocks++;
  }

  return blocks;
}

static int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  if (bAscending) {
    ix1 = *(int *)elem1;
    ix2 = *(int *)elem2;
  }
  else {
    ix1 = *(int *)elem2;
    ix2 = *(int *)elem1;
  }

  if (filename_inf[ix1].block_pct > filename_inf[ix2].block_pct)
    return 1;
  else if (filename_inf[ix1].block_pct < filename_inf[ix2].block_pct)
    return -1;
  else
    return 0;
}
