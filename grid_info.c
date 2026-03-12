#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: grid_info filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int count_blocks(char *line,int line_len);
static int grid_info(char *filename);

int main(int argc,char **argv)
{
  int retval;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  retval = grid_info(argv[1]);

  if (retval) {
    printf("grid_info of %s failed: %d\n",argv[1],retval);
    return 2;
  }

  return 0;
}

static int grid_info(char *filename)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int save_line_len;
  int puzzle_size;
  int blocks;
  int total_blocks;
  double block_pct;

  if ((fptr = fopen(filename,"r")) == NULL) {
    printf(couldnt_open,filename);
    return 1;
  }

  line_no = 0;
  total_blocks = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (line_no == 1)
      save_line_len = line_len;
    else if (line_len != save_line_len) {
      printf("length of line %d doesn't conform\n",line_no);
      return 2;
    }

    blocks = count_blocks(line,line_len);
    total_blocks += blocks;
  }

  fclose(fptr);

  puzzle_size = save_line_len * line_no;
  block_pct = (double)total_blocks / (double)puzzle_size * (double)100;

  printf("%d x %d, %6.2lf (%d %d)\n",save_line_len,line_no,block_pct,total_blocks,puzzle_size);

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

static int count_blocks(char *line,int line_len)
{
  int n;
  int blocks;

  blocks = 0;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '.')
      blocks++;
  }

  return blocks;
}
