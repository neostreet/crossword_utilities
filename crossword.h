#define MAX_WIDTH 23
#define MAX_ACROSS_WORDS 200
#define MAX_DOWN_WORDS   200
#define MAX_GRID_SIZE (MAX_WIDTH * MAX_WIDTH)

#define MAX_HISTOGRAM_VALUES 21

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

  char* get_solution();
  bool validate_solution();

  char* get_grid();
  bool validate_grid();

  const int get_num_across_words() const;
  struct offset_len* get_across_words();

  const int get_num_down_words() const;
  struct offset_len* get_down_words();

  void transpose();

  const int get_num_across_words_histogram_values() const;
  struct histogram* get_across_words_histogram();

  const int get_num_down_words_histogram_values() const;
  struct histogram* get_down_words_histogram();

  void print(ostream& out);

  private:

  int _width;
  int _total_squares;
  int _midpoint;
  int _num_letters;
  char _grid[MAX_GRID_SIZE];
  char _solution[MAX_GRID_SIZE];

  int _num_across_words;
  struct offset_len _across_words[MAX_ACROSS_WORDS];

  int _num_down_words;
  struct offset_len _down_words[MAX_DOWN_WORDS];

  int _num_across_words_histogram_values;
  struct histogram _across_words_histogram[MAX_HISTOGRAM_VALUES];

  int _num_down_words_histogram_values;
  struct histogram _down_words_histogram[MAX_HISTOGRAM_VALUES];
};

ostream& operator<<(ostream& out,CrossWord& crossword);
