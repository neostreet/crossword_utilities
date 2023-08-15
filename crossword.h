struct offset_len {
  int offset;
  int len;
  int clue_number;
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

  void clear();

  void set_width(int width);
  const int get_width() const;

  void set_height(int height);
  const int get_height() const;

  const int get_total_squares() const;

  const int get_num_letters() const;

  string& get_solution();
  bool validate_solution();

  vector<struct offset_len>& get_across_words();

  list<struct histogram>& get_across_words_histogram();

  vector<struct offset_len>& get_down_words();

  list<struct histogram>& get_down_words_histogram();

  void transpose();

  void print(ostream& out);

  private:

  int _width;
  int _height;
  int _total_squares;
  int _midpoint;
  int _num_letters;
  string _solution;

  vector<struct offset_len> _across_words;

  list<struct histogram> _across_words_histogram;

  vector<struct offset_len> _down_words;

  list<struct histogram> _down_words_histogram;
};

ostream& operator<<(ostream& out,CrossWord& crossword);
