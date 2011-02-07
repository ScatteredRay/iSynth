#define TABLE_SIZE 2048
#define TABLE_ROWS 10

static float saw_table[TABLE_ROWS][TABLE_SIZE];
static float tri_table[TABLE_ROWS][TABLE_SIZE];

void table_init() {
  int freq=55;
  for (int i(0); i < TABLE_ROWS; ++i) {
    for (int j(0); j < TABLE_SIZE; ++j) {
      float saw_acc=0, tri_acc=0;
      for (int k(1); k*freq < 22050; ++k) {
        float s = sin(2*pi*k*j/TABLE_SIZE);
        saw_acc += s/k;
        if (k&1)
          tri_acc += s/(k*k) * (k&2? -1: 1);
      }
      saw_table[i][j] = -2 * saw_acc / pi;
      tri_table[i][j] = 8 * tri_acc / (pi*pi);
    }
    freq *= 2;
  }
}

int table_row(float freq) {
  int ifreq = int(freq);
  int row=0;
  while (ifreq > 55) {
    ifreq >>= 1;
    ++row;
  }
  return row;
}

