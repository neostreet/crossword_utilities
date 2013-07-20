#define MAX_WIDTH 25
#define MAX_GRID_SIZE (MAX_WIDTH * MAX_WIDTH)

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

  char* get_grid();
  bool validate_grid();

  char* get_solution();
  bool validate_solution();

  const int get_num_across_words() const;
  const int get_num_down_words() const;

  void print(ostream& out) const;

  private:

  int _width;
  int _total_squares;
  int _midpoint;
  int _num_letters;
  char _grid[MAX_GRID_SIZE];
  char _solution[MAX_GRID_SIZE];
  int _num_across_words;
  int _num_down_words;
};

ostream& operator<<(ostream& out,const CrossWord& crossword);
