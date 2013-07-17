struct offset_len {
  int offset;
  int len;
};

#define MAX_BLACK_SPACE_STRUCTS 100

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

  const int get_num_black_space_structs() const;

  void set_black_space_structs(struct offset_len *black_space_structs,int num_black_space_structs);
  const struct offset_len* get_black_space_structs() const;

  const int get_num_letters() const;

  void print(ostream& out) const;

  private:

  int _width;
  int _total_squares;
  int _num_black_space_structs;
  int _num_letters;
  struct offset_len _black_space_structs[MAX_BLACK_SPACE_STRUCTS];
};

ostream& operator<<(ostream& out,const CrossWord& crossword);
