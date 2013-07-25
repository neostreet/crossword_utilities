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

  for (n = 0; n < MAX_GRID_SIZE; n++) {
    _grid[n] = crossword._grid[n];
    _solution[n] = crossword._solution[n];
  }

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

  for (n = 0; n < MAX_GRID_SIZE; n++) {
    _grid[n] = crossword._grid[n];
    _solution[n] = crossword._solution[n];
  }
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

char* CrossWord::get_solution()
{
  return _solution;
}

bool CrossWord::validate_solution()
{
  int m;
  int n;
  int p;
  int q;
  int r;
  int num_letters;

  num_letters = 0;

  for (n = 0; n < _midpoint; n++) {
    if (_solution[n] != '.')
      num_letters++;
    else if (_solution[_total_squares - 1 - n] != '.')
        return false;
  }

  num_letters *= 2;

  if (_total_squares % 2) {
    if (_solution[_midpoint] != '.')
      num_letters++;
  }

  _num_letters = num_letters;

  // calculate the number of across words in the solution, and
  // initialize _across_words and _across_words_histograms

  p = 0;
  _num_across_words = 0;
  _num_across_words_histogram_values = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_solution[p] != '.') && (!m || (_solution[p-1] == '.'))) {
        _across_words[_num_across_words].offset = p;

        for (q = p + 1;
             ((q < (n + 1) * _width) && (_solution[q] != '.'));
             q++)
          ;

        _across_words[_num_across_words].len = q - p;

        _num_across_words++;

        for (r = 0; r < _num_across_words_histogram_values; r++) {
          if (_across_words_histogram[r].value == q - p)
            break;
        }

        if (r == _num_across_words_histogram_values) {
          _across_words_histogram[r].value = q - p;
          _across_words_histogram[r].observations = 1;
          _num_across_words_histogram_values++;
        }
        else
          _across_words_histogram[r].observations++;
      }

      p++;
    }
  }

  // calculate the number of down words in the solution, and
  // initialize _down_words and _down_words_histograms

  _num_down_words = 0;
  _num_down_words_histogram_values = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_solution[n + m * _width] != '.') && (!m || (_solution[n + (m - 1) * _width] == '.'))) {
        _down_words[_num_down_words].offset = n + m * _width;

        for (p = 0, q = n + (m + 1) * _width;
             ((q < n + _width * _width) && (_solution[q] != '.'));
             p++, q += _width)
          ;

        _down_words[_num_down_words].len = p + 1;
        _num_down_words++;

        for (r = 0; r < _num_down_words_histogram_values; r++) {
          if (_down_words_histogram[r].value == p + 1)
            break;
        }

        if (r == _num_down_words_histogram_values) {
          _down_words_histogram[r].value = p + 1;
          _down_words_histogram[r].observations = 1;
          _num_down_words_histogram_values++;
        }
        else
          _down_words_histogram[r].observations++;
      }
    }
  }

  return true;
}

char* CrossWord::get_grid()
{
  return &_grid[0];
}

bool CrossWord::validate_grid()
{
  int n;

  for (n = 0; n < _total_squares; n++) {
    if ((_grid[n] == '.') && (_solution[n] != '.'))
      return false;

    if ((_solution[n] == '.') && (_grid[n] != '.'))
      return false;
  }

  return true;
}

const int CrossWord::get_num_across_words() const
{
  return _num_across_words;
}

struct offset_len* CrossWord::get_across_words()
{
  return _across_words;
}

const int CrossWord::get_num_down_words() const
{
  return _num_down_words;
}

struct offset_len* CrossWord::get_down_words()
{
  return _down_words;
}

void CrossWord::transpose()
{
  int m;
  int n;
  int p;
  char work[MAX_GRID_SIZE];

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++)
      work[_width * n + m] = _solution[p++];
  }

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++) {
      _solution[p] = work[p];
      p++;
    }
  }

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++)
      work[_width * n + m] = _grid[p++];
  }

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++) {
      _grid[p] = work[p];
      p++;
    }
  }
}

const int CrossWord::get_num_across_words_histogram_values() const
{
  return _num_across_words_histogram_values;
}

struct histogram* CrossWord::get_across_words_histogram()
{
  return _across_words_histogram;
}

const int CrossWord::get_num_down_words_histogram_values() const
{
  return _num_down_words_histogram_values;
}

struct histogram* CrossWord::get_down_words_histogram()
{
  return _down_words_histogram;
}

void CrossWord::print(ostream& out)
{
  int m;
  int n;
  int p;
  char row[MAX_WIDTH+1];

  out << "_width = " << _width << endl;
  out << "_total_squares = " << _total_squares << endl;
  out << "_midpoint = " << _midpoint << endl;
  out << "_num_letters = " << _num_letters << endl;
  out << "_num_across_words = " << _num_across_words << endl;
  out << "_num_down_words = " << _num_down_words << endl;

  p = 0;

  row[_width] = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++)
      row[n] = _solution[p++];

    cout << row << endl;
  }

  cout << endl << "ACROSS" << endl << endl;

  for (m = 0; m < _num_across_words; m++) {
    for (n = 0; n < _across_words[m].len; n++)
      row[n] = _solution[_across_words[m].offset + n];

    row[n] = 0;

    cout << row << endl;
  }

  cout << endl << "across words histogram" << endl << endl;

  for (n = 0; n < _num_across_words_histogram_values; n++) {
    cout << _across_words_histogram[n].value << " " <<
      _across_words_histogram[n].observations << endl;
  }

  cout << endl << "DOWN" << endl << endl;

  for (m = 0; m < _num_down_words; m++) {
    for (n = 0; n < _down_words[m].len; n++)
      row[n] = _solution[_down_words[m].offset + n * _width];

    row[n] = 0;

    cout << row << endl;
  }

  cout << endl << "down words histogram" << endl << endl;

  for (n = 0; n < _num_down_words_histogram_values; n++) {
    cout << _down_words_histogram[n].value << " " <<
      _down_words_histogram[n].observations << endl;
  }
}

ostream& operator<<(ostream& out,CrossWord& crossword)
{
  crossword.print(out);

  return out;
}
