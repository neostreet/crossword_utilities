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
  _midpoint = 0;
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
  _midpoint = crossword._midpoint;

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
  _midpoint = crossword._midpoint;

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
  _midpoint = _total_squares / 2;
}

const int CrossWord::get_width() const
{
  return _width;
}

const int CrossWord::get_total_squares() const
{
  return _total_squares;
}

const int CrossWord::get_num_letters() const
{
  return _num_letters;
}

char* CrossWord::get_grid()
{
  return &_grid[0];
}

bool CrossWord::validate_grid()
{
  int m;
  int n;
  int p;
  int num_letters;

  num_letters = 0;

  for (n = 0; n < _midpoint; n++) {
    if (_grid[n] == ' ')
      num_letters++;
    else if (_grid[n] != '.')
      return false;

    if (_grid[n] != _grid[_total_squares - 1 - n])
        return false;
  }

  num_letters *= 2;

  if (_total_squares % 2) {
    if ((_grid[_midpoint] != ' ') && (_grid[_midpoint] != '.'))
      return false;

    num_letters++;
  }

  _num_letters = num_letters;

  // calculate the number of across words in the grid

  p = 0;
  _num_across_words = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_grid[p] != '.') && (!m || (_grid[p-1] == '.')))
        _num_across_words++;

      p++;
    }
  }

  // calculate the number of down words in the grid

  _num_down_words = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_grid[n + m * _width] != '.') && (!m || (_grid[n + (m - 1) * _width] == '.')))
        _num_down_words++;
    }
  }

  return true;
}

char* CrossWord::get_solution()
{
  return _solution;
}

bool CrossWord::validate_solution()
{
  int n;

  for (n = 0; n < _num_letters; n++) {
    if ((_solution[n] < 'A') || (_solution[n] > 'Z'))
      return false;
  }

  return true;
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
  int q;
  char row[MAX_WIDTH+1];

  out << "_width = " << _width << endl;
  out << "_total_squares = " << _total_squares << endl;
  out << "_midpoint = " << _midpoint << endl;
  out << "_num_letters = " << _num_letters << endl;
  out << "_num_across_words = " << _num_across_words << endl;
  out << "_num_down_words = " << _num_down_words << endl;

  p = 0;
  q = 0;

  row[_width] = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      row[m] = _grid[p++];

      if (row[m] == ' ')
        row[m] = _solution[q++];
    }

    cout << row << endl;
  }
}

ostream& operator<<(ostream& out,const CrossWord& crossword)
{
  crossword.print(out);

  return out;
}
