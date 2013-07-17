#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "crossword.h"

// default constructor

CrossWord::CrossWord()
{
  _width = 0;
  _total_squares = 0;
  _num_black_space_structs = 0;
  _num_letters = 0;
}

// copy constructor

CrossWord::CrossWord(const CrossWord& crossword)
{
  int n;

  _width = crossword._width;
  _total_squares = crossword._total_squares;
  _num_black_space_structs = crossword._num_black_space_structs;

  for (n = 0; n < _num_black_space_structs; n++)
    _black_space_structs[n] = crossword._black_space_structs[n];

  _num_letters = crossword._num_letters;
}

// assignment operator

CrossWord& CrossWord::operator=(const CrossWord& crossword)
{
  int n;

  _width = crossword._width;
  _total_squares = crossword._total_squares;

  _num_black_space_structs = crossword._num_black_space_structs;

  for (n = 0; n < _num_black_space_structs; n++)
    _black_space_structs[n] = crossword._black_space_structs[n];

  _num_letters = crossword._num_letters;

  return *this;
}

// destructor

CrossWord::~CrossWord()
{
}

void CrossWord::set_width(int width)
{
  _width = width;
  _total_squares = _width * _width;
}

const int CrossWord::get_width() const
{
  return _width;
}

const int CrossWord::get_total_squares() const
{
  return _total_squares;
}

const int CrossWord::get_num_black_space_structs() const
{
  return _num_black_space_structs;
}

void CrossWord::set_black_space_structs(struct offset_len *black_space_structs,int num_black_space_structs)
{
  int n;
  int midpoint;
  int num_black_squares;

  _num_black_space_structs = num_black_space_structs;

  num_black_squares = 0;

  for (n = 0; n < _num_black_space_structs; n++) {
    _black_space_structs[n] = black_space_structs[n];
    num_black_squares += black_space_structs[n].len;
  }

  midpoint = _total_squares / 2;

  if (black_space_structs[num_black_space_structs - 1].offset +
    black_space_structs[num_black_space_structs - 1].len - 1 == midpoint) {
    _num_letters = _total_squares - (num_black_squares * 2) + 1;
  }
  else
    _num_letters = _total_squares - (num_black_squares * 2);
}

const struct offset_len* CrossWord::get_black_space_structs() const
{
  return _black_space_structs;
}

const int CrossWord::get_num_letters() const
{
  return _num_letters;
}

void CrossWord::print(ostream& out) const
{
  out << "_width = " << _width << endl;
  out << "_total_squares = " << _total_squares << endl;
  out << "_num_black_space_structs = " << _num_black_space_structs << endl;
  out << "_num_letters = " << _num_letters << endl;
}

ostream& operator<<(ostream& out,const CrossWord& crossword)
{
  crossword.print(out);

  return out;
}
