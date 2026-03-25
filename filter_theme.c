#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: filter_theme filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int filter_line(char *line,int line_len);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int theme_letters;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  theme_letters = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    theme_letters += filter_line(line,line_len);
    printf("%s\n",line);
  }

  printf("\ntheme_letters = %d\n",theme_letters);

  fclose(fptr);

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

static int filter_line(char *line,int line_len)
{
  int n;
  int theme_letters;

  theme_letters = 0;

  for (n = 0; n < line_len; n++) {
    if ((line[n] >= 'A') && (line[n] <= 'Z'))
      theme_letters++;
    else
      line[n] = ' ';
  }

  return theme_letters;
}
