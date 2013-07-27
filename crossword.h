struct offset_len {
  int offset;
  int len;
};

struct histogram {
  int value;
  int observations;
};

class CrossWord {
  public:

  // default constructor
  CrossWord();
  // copy constructor
  CrossWord(const CrossWord&);
  // assignment operator
  CrossWord& operator=(const CrossWord&);
  // destructor
  ~CrossWord();

  void set_width(int width);
  const int get_width() const;

  const int get_total_squares() const;

  const int get_num_letters() const;

  string& get_solution();
  bool validate_solution();

  string& get_grid();
  bool validate_grid();

  vector<struct offset_len>& get_across_words();

  vector<struct histogram>& get_across_words_histogram();

  vector<struct offset_len>& get_down_words();

  vector<struct histogram>& get_down_words_histogram();

  void transpose();

  void print(ostream& out);

  private:

  int _width;
  int _total_squares;
  int _midpoint;
  int _num_letters;
  string _solution;
  string _grid;

  vector<struct offset_len> _across_words;

  vector<struct histogram> _across_words_histogram;

  vector<struct offset_len> _down_words;

  vector<struct histogram> _down_words_histogram;
};

ostream& operator<<(ostream& out,CrossWord& crossword);
