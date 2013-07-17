#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "crossword.h"

static struct offset_len black_space_structs[] = {
  {4, 1}, {10, 1}, {19, 1}, {25, 1}, {40, 1}, {48, 1}, {53, 1},
  {60, 3}, {67, 1}, {81, 1}, {88,2}, {95,1}, {101, 1}, {109,1}
};
#define NUM_BLACK_SPACE_STRUCTS (sizeof black_space_structs / sizeof (struct offset_len))

int main(int argc,char **argv)
{
  CrossWord crossword;

  crossword.set_width(15);
  crossword.set_black_space_structs(black_space_structs,NUM_BLACK_SPACE_STRUCTS);

  cout << crossword;

  return 0;
}
