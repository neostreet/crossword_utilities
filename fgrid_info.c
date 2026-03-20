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

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fgrid_info (-terse) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char couldnt_get_status[] = "couldn't get status of %s\n";

static char malloc_failed[] = "malloc of %d bytes failed\n";
static char read_failed[] = "%s: read of %d bytes failed\n";

#define MAX_WORD_LEN 20
static char word[MAX_WORD_LEN+1];
static int word_len_counts[MAX_WORD_LEN-2];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int grid_info(char *filename,bool bTerse);
static int read_grid(char *filename,char **in_buf_pt,int *width_pt,int *height_pt);
static void compress(char *in_buf,int width,int height);
static int count_blocks(char *in_buf,int puzzle_size);
static int has_symmetry(char *in_buf,int puzzle_size);
static int count_theme_letters(char *in_buf,int puzzle_size);

int main(int argc,char **argv)
{
  int retval;
  int curr_arg;
  bool bTerse;
  FILE *fptr0;
  int filename_len;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bTerse = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
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
    GetLine(fptr0,filename,&filename_len,MAX_LINE_LEN);

    if (feof(fptr0))
      break;

    retval = grid_info(filename,bTerse);

    if (retval)
      printf("grid_info of %s failed: %d\n",filename,retval);
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


static int grid_info(char *filename,bool bTerse)
{
  int retval;
  char *in_buf;
  int width;
  int height;
  int puzzle_size;
  int blocks;
  double block_pct;
  bool bHasSymmetry;
  int theme_letters;
  double theme_letters_pct;

  retval = read_grid(filename,&in_buf,&width,&height);

  if (retval) {
    printf("read_grid(() failed: %d\n",retval);
    return 1;
  }

  compress(in_buf,width,height);

  puzzle_size = width * height;
  blocks = count_blocks(in_buf,puzzle_size);
  block_pct = (double)blocks / (double)puzzle_size * (double)100;

  if (!bTerse) {
    bHasSymmetry = has_symmetry(in_buf,puzzle_size);
    theme_letters = count_theme_letters(in_buf,puzzle_size);
    theme_letters_pct = (double)theme_letters / (double)puzzle_size * (double)100;
    printf("%s: %d x %d, %s, blocks %6.2lf%% (%d %d) theme_letters %6.2lf%% (%d %d)\n",
      filename,width,height,
      (bHasSymmetry ? "symmetric" : "asymmetric"),
      block_pct,blocks,puzzle_size,
      theme_letters_pct,theme_letters,puzzle_size);
  }
  else  {
    printf("%5.2lf (%d %d) %d x %d %s\n",block_pct,blocks,puzzle_size,width,height,filename);
  }

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

static int has_symmetry(char *in_buf,int puzzle_size)
{
  int m;
  int n;

  for (m = 0,n = puzzle_size - 1; (m < n); m++,n--) {
    if (((in_buf[n] == '.') && (in_buf[m] != '.')) ||
        ((in_buf[n] != '.') && (in_buf[m] == '.'))) {

      return false;
    }
  }

  return true;
}

static int count_theme_letters(char *in_buf,int puzzle_size)
{
  int n;
  int theme_letters;

  theme_letters = 0;

  for (n = 0; n < puzzle_size; n++) {
    if ((in_buf[n] >= 'A') && (in_buf[n] <= 'Z'))
      theme_letters++;
  }

  return theme_letters;
}
