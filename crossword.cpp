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
  _num_across_words = 0;
  _num_down_words = 0;
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
  _num_across_words = crossword._num_across_words;
  _num_down_words = crossword._num_down_words;
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
  _num_across_words = crossword._num_across_words;
  _num_down_words = crossword._num_down_words;

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

void CrossWord::set_solution(char *solution,int num_letters)
{
  int n;

  for (n = 0; n < num_letters; n++)
    _solution[n] = solution[n];
}

const char* CrossWord::get_solution() const
{
  return _solution;
}

void CrossWord::initialize_grid()
{
  int m;
  int n;
  int p;
  int q;
  int midpoint;

  // copy the solution letters to the correct spots in the grid;
  // initialize black squares in the grid to zero

  midpoint = _total_squares / 2;

  p = 0;
  q = 0;

  for (n = 0; n < _num_black_space_structs; n++) {
    for ( ; p < _black_space_structs[n].offset; p++,q++) {
      _grid[p] = _solution[q];
      _grid[_total_squares - 1 - p] = _solution[_num_letters - 1 - q];
    }

    for (m = 0; m < _black_space_structs[n].len; m++) {
      _grid[_black_space_structs[n].offset + m] = 0;
      _grid[_total_squares - 1 - _black_space_structs[n].offset - m] = 0;
    }

    p += _black_space_structs[n].len;
  }

  // if there is a word which spans the middle of the grid, fill it in

  for ( ; p < midpoint; p++,q++) {
    _grid[p] = _solution[q];
    _grid[_total_squares - 1 - p] = _solution[_num_letters - 1 - q];
  }

  // if there's an odd number of squares in the grid, and the middle square
  // is not a black square, fill it in

  if (_total_squares % 2) {
    if (_black_space_structs[_num_black_space_structs - 1].offset +
      _black_space_structs[_num_black_space_structs - 1].len - 1 != midpoint) {
      _grid[p] = _solution[q];
    }
  }

  // calculate the number of across words in the grid

  p = 0;
  _num_across_words = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if (_grid[p] && (!m || !_grid[p-1]))
        _num_across_words++;

      p++;
    }
  }

  // calculate the number of down words in the grid

  _num_down_words = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if (_grid[n + m * _width] && (!m || !_grid[n + (m - 1) * _width]))
        _num_down_words++;
    }
  }
}

const int CrossWord::get_num_across_words() const
{
  return _num_across_words;
}

const int CrossWord::get_num_down_words() const
{
  return _num_down_words;
}

void CrossWord::print(ostream& out) const
{
  int m;
  int n;
  int p;
  char row[MAX_WIDTH+1];

  out << "_width = " << _width << endl;
  out << "_total_squares = " << _total_squares << endl;
  out << "_num_black_space_structs = " << _num_black_space_structs << endl;
  out << "_num_letters = " << _num_letters << endl;
  out << "_num_across_words = " << _num_across_words << endl;
  out << "_num_down_words = " << _num_down_words << endl;

  p = 0;

  row[_width] = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if (_grid[p])
        row[m] = _grid[p++];
      else {
        row[m] = ' ';
        p++;
      }
    }

    cout << row << endl;
  }
}

ostream& operator<<(ostream& out,const CrossWord& crossword)
{
  crossword.print(out);

  return out;
}
