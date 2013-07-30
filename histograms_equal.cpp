#include <iostream>
#include <list>
#include <vector>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "crossword.h"

#define MAX_LINE_LEN 8192
char line[MAX_LINE_LEN];

static char usage[] = "usage: histograms_equal (-not) filename (filename ...)\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bNot;
  int m;
  int n;
  FILE *fptr;
  bool bErr;
  int linelen;
  int line_no;
  int width;
  CrossWord crossword;

  if (argc < 2) {
    printf(usage);
    return 1;
  }

  bNot = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else
      break;
  }

  if (argc - curr_arg < 1) {
    printf(usage);
    return 2;
  }

  for ( ; curr_arg < argc; curr_arg++) {
    if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
      printf(couldnt_open,argv[curr_arg]);
      continue;
    }

    crossword.clear();

    string& solution = crossword.get_solution();

    line_no = 0;
    bErr = false;

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
          bErr = true;
          break;
        }

        solution.append(line);
      }

      line_no++;

      if (line_no == width + 1)
        break;
    }

    fclose(fptr);

    if (bErr)
      continue;

    if (!crossword.validate_solution()) {
      printf("solution is invalid\n");
      continue;
    }

    list<struct histogram>& across_words_histogram =
      crossword.get_across_words_histogram();
    list<struct histogram>& down_words_histogram =
      crossword.get_down_words_histogram();

    if (!bNot) {
      if (across_words_histogram.size() != down_words_histogram.size())
        continue;
    }
    else {
      if (across_words_histogram.size() != down_words_histogram.size()) {
        printf("%s\n",argv[curr_arg]);
        continue;
      }
    }

    list<struct histogram>::iterator it1 = across_words_histogram.begin();
    list<struct histogram>::iterator it2 = down_words_histogram.begin();

    for ( ; it1 != across_words_histogram.end(); it1++, it2++) {
      if ((it1->value != it2->value) || (it1->observations != it2->observations))
        break;
    }

    if (!bNot) {
      if (it1 == across_words_histogram.end())
        printf("%s\n",argv[curr_arg]);
    }
    else {
      if (it1 != across_words_histogram.end())
        printf("%s\n",argv[curr_arg]);
    }
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
