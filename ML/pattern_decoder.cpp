#include "../precomp.h"
#include "PatternExtractor_3x3.h"

using namespace std;
using namespace Common;
using namespace ML;

char convert(Color c) {
  if (c == BLACK) {
    return 'b';
  } else if (c == WHITE) {
    return 'w';
  } else if (c == FREE) {
    return '+';
  } else if (c == WALL) {
    return '*';
  }
  return 'e';
}

int main(void) {
  string line;
  while (getline(cin, line)) {
    PATTERN_3x3_HASH_VALUE value = static_cast<PATTERN_3x3_HASH_VALUE>(strtol(line.c_str(), NULL, 16));
    bool border_left, border_right, border_up, border_down;
    Color pattern[8];
    PatternExtractor_3x3::decode(value, pattern, border_left, border_right, border_up, border_down);

    cout << " ";
    if (border_up) {
      cout << "***" << endl;
    } else {
      cout << "+++" << endl;
    }
    cout << (border_left ? "*" : "+") << convert(pattern[0]) << convert(pattern[1]) << convert(pattern[2]) << (border_right ? "*" : "+") << endl;
    cout << (border_left ? "*" : "+") << convert(pattern[3]) << " " << convert(pattern[4]) << (border_right ? "*" : "+") << endl;
    cout << (border_left ? "*" : "+") << convert(pattern[5]) << convert(pattern[6]) << convert(pattern[7]) << (border_right ? "*" : "+") << endl;
    cout << (border_down ? " ***" : " +++") << endl;
  }
  return 0;
}
