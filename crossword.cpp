#include <iostream>
#include <vector>
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
}

// copy constructor

CrossWord::CrossWord(const CrossWord& crossword)
{
  int n;

  _width = crossword._width;
  _total_squares = crossword._total_squares;
  _midpoint = crossword._midpoint;
  _num_letters = crossword._num_letters;
  _solution = crossword._solution;
  _grid = crossword._grid;
}

// assignment operator

CrossWord& CrossWord::operator=(const CrossWord& crossword)
{
  int n;

  _width = crossword._width;
  _total_squares = crossword._total_squares;
  _midpoint = crossword._midpoint;
  _num_letters = crossword._num_letters;
  _solution = crossword._solution;
  _grid = crossword._grid;

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

string& CrossWord::get_solution()
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
  struct offset_len work1;
  struct histogram work2;

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

  _across_words.clear();
  _down_words.clear();

  p = 0;

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_solution[p] != '.') && (!m || (_solution[p-1] == '.'))) {
        work1.offset = p;

        for (q = p + 1;
             ((q < (n + 1) * _width) && (_solution[q] != '.'));
             q++)
          ;

        work1.len = q - p;

        _across_words.push_back(work1);

        for (r = 0; r < _across_words_histogram.size(); r++) {
          if (_across_words_histogram[r].value == q - p)
            break;
        }

        if (r == _across_words_histogram.size()) {
          work2.value = q - p;
          work2.observations = 1;
          _across_words_histogram.push_back(work2);
        }
        else
          _across_words_histogram[r].observations++;
      }

      p++;
    }
  }

  // calculate the number of down words in the solution, and
  // initialize _down_words and _down_words_histograms

  for (n = 0; n < _width; n++) {
    for (m = 0; m < _width; m++) {
      if ((_solution[n + m * _width] != '.') && (!m || (_solution[n + (m - 1) * _width] == '.'))) {
        work1.offset = n + m * _width;

        for (p = 0, q = n + (m + 1) * _width;
             ((q < n + _width * _width) && (_solution[q] != '.'));
             p++, q += _width)
          ;

        work1.len = p + 1;
        _down_words.push_back(work1);;

        for (r = 0; r < _down_words_histogram.size(); r++) {
          if (_down_words_histogram[r].value == p + 1)
            break;
        }

        if (r == _down_words_histogram.size()) {
          work2.value = p + 1;
          work2.observations = 1;
          _down_words_histogram.push_back(work2);
        }
        else
          _down_words_histogram[r].observations++;
      }
    }
  }

  return true;
}

string& CrossWord::get_grid()
{
  return _grid;
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

vector<struct offset_len>& CrossWord::get_across_words()
{
  return _across_words;
}

vector<struct histogram>& CrossWord::get_across_words_histogram()
{
  return _across_words_histogram;
}

vector<struct offset_len>& CrossWord::get_down_words()
{
  return _down_words;
}

vector<struct histogram>& CrossWord::get_down_words_histogram()
{
  return _down_words_histogram;
}

void CrossWord::transpose()
{
  int m;
  int n;
  int p;
  string work;

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++)
      work.push_back(_solution[_width * n + m]);
  }

  p = 0;

  _solution.clear();
  _solution = work;

  validate_solution();

  work.clear();

  p = 0;

  for (m = 0; m < _width; m++) {
    for (n = 0; n < _width; n++)
      work.push_back(_grid[_width * n + m]);
  }

  _grid.clear();
  _grid = work;

  validate_grid();
}

void CrossWord::print(ostream& out)
{
  int m;
  int n;
  int p;

  out << "_width = " << _width << endl;
  out << "_total_squares = " << _total_squares << endl;
  out << "_midpoint = " << _midpoint << endl;
  out << "_num_black_squares = " << _total_squares - _num_letters << endl;
  out << "_num_letters = " << _num_letters << endl;
  out << "_num_across_words = " << _across_words.size() << endl;
  out << "_num_down_words = " << _down_words.size() << endl;

  p = 0;

  for (m = 0; m < _width; m++)
    cout << _solution.substr(m * _width,_width) << endl;

  cout << endl << "ACROSS" << endl << endl;

  for (m = 0; m < _across_words.size(); m++)
    cout << _solution.substr(_across_words[m].offset,_across_words[m].len) << endl;

  cout << endl << "across words histogram" << endl << endl;

  for (n = 0; n < _across_words_histogram.size(); n++) {
    cout << _across_words_histogram[n].value << " " <<
      _across_words_histogram[n].observations << endl;
  }

  cout << endl << "DOWN" << endl << endl;

  for (m = 0; m < _down_words.size(); m++) {
    for (n = 0; n < _down_words[m].len; n++)
      cout << _solution.substr(_down_words[m].offset + n * _width,1);

    cout << endl;
  }

  cout << endl << "down words histogram" << endl << endl;

  for (n = 0; n < _down_words_histogram.size(); n++) {
    cout << _down_words_histogram[n].value << " " <<
      _down_words_histogram[n].observations << endl;
  }
}

ostream& operator<<(ostream& out,CrossWord& crossword)
{
  crossword.print(out);

  return out;
}
