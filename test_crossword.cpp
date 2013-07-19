#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "crossword.h"

#define MAX_LINE_LEN 8192
char line[MAX_LINE_LEN];

static char usage[] = "usage: test_crossword filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int linelen;
  int line_no;
  int width;
  CrossWord crossword;
  char *grid;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  grid = crossword.get_grid();

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&linelen,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (!line_no)
      sscanf(line,"%d",&width);
    else {
      if (line_no > width + 1)
        break;

      if (linelen < width) {
        for ( ; linelen < width; linelen++)
          line[linelen] = ' ';
      }
      else if (linelen > width) {
        printf("line %d has a length of %d when it should have a length of %d\n",
          line_no,linelen,width);
        return 3;
      }

      memcpy(&grid[(line_no - 1) * width],line,width);
    }

    line_no++;
  }

  fclose(fptr);

  crossword.set_width(width);

  if (!crossword.validate_grid()) {
    printf("grid is invalid\n");
    return 4;
  }

  cout << crossword;

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
