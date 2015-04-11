
#include "precomp.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cassert>

#include "utility.h"
using namespace std;

class ToUpperFunc : public unary_function<char, char>{
public:
  char operator ()(char c) const {
    return toupper(c);
  }
};

namespace Common {
  std::vector<std::string> split(const std::string &str, const char *delim_list) {
    vector<string> ret;
    split(str, ret, delim_list);
    return ret;
  }

  void split(const std::string &str, std::vector<std::string> &result, const char *delim_list) {
    result.clear();

    string::size_type next_pos = 0, last_pos = 0;

    while ((next_pos = str.find(delim_list, next_pos)) != string::npos) {
      size_t len = next_pos - last_pos;
      string split_str(str.substr(last_pos, len));
      if (!split_str.empty()) {
        result.push_back(split_str);
      }
      next_pos++;
      last_pos = next_pos;
    }
    if (last_pos != str.size()) {
      result.push_back(str.substr(last_pos));
    }
  }

  std::string changeExt(const std::string &str, const std::string &newext) {
    
    int pos = -1;
    for (int i = (signed)str.size()-1; i>=0; i--) {
      if (str[i] == '.') {
        pos = i;
        break;
      }
    }
    // no ext
    if (pos == -1) {
      return str;
    }

    return str.substr(0, pos+1) + newext;
  }

  std::string toUpper(const std::string &str) {
    std::string ret(str);
    std::transform(ret.begin(), ret.end(), ret.begin(), ToUpperFunc());
    return ret;
  }


  ApproximatedMath::ApproximatedMath(int logMax)
    : m_logTable(new double[logMax+1])
    , m_logTableMax(logMax+1)
  {
    for (int i=1; i<logMax+1; i++) {
      m_logTable[i] = log(i);
    }
  }

  ApproximatedMath::~ApproximatedMath() {
    delete [] m_logTable;
  }
}
