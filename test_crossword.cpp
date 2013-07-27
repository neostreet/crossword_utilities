#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "crossword.h"

#define MAX_LINE_LEN 8192
char line[MAX_LINE_LEN];

static char usage[] = "usage: test_crossword (-transpose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTranspose;
  int m;
  int n;
  FILE *fptr;
  int linelen;
  int line_no;
  int width;
  CrossWord crossword;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bTranspose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-transpose"))
      bTranspose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  string& solution = crossword.get_solution();

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&linelen,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (!line_no) {
      sscanf(line,"%d",&width);
      crossword.set_width(width);
    }
    else {
      if (linelen != width) {
        printf("line %d has a length of %d when it should have a length of %d\n",
          line_no,linelen,width);
        return 4;
      }

      solution.append(line);
    }

    line_no++;

    if (line_no == width + 1)
      break;
  }

  if (!crossword.validate_solution()) {
    printf("solution is invalid\n");
    return 5;
  }

  string& grid = crossword.get_grid();

  m = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&linelen,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (linelen < width) {
      for ( ; linelen < width; linelen++)
        line[linelen] = ' ';
    }
    else if (linelen != width) {
      printf("line %d has a length of %d when it should have a length of %d\n",
        line_no,linelen,width);
      return 6;
    }

    grid.append(line);

    line_no++;
    m++;

    if (m == width)
      break;
  }

  fclose(fptr);

  if (!crossword.validate_grid()) {
    printf("grid is invalid\n");
    return 7;
  }

  if (bTranspose)
    crossword.transpose();

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
