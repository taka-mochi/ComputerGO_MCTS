
#pragma once

#include "Record/Record.h"

namespace Common {
  class SgfReader {
  public:
    enum SGF_READ_RESULT {
      SGF_READ_OK, SGF_FAILED_TO_OPEN_FILE, SGF_MULTI_SEQUENCE, SGF_NOT_GO, SGF_NOT_SUPPORTED,
      SGF_ILLEGAL_PROPERTY, SGF_NO_CONSISTENCY_FOR_HANDICAP, 
      SGF_UNKNOWN_PROPERTY, SGF_UNKNOWN_ERROR
    };

  private:
    static std::string m_prevProperty;

    static SGF_READ_RESULT processMoveSequenceLine(const std::string &l, Record &result);
    static SGF_READ_RESULT processOneline(const std::string &line, Record &result);

  public:


    static SGF_READ_RESULT readFromFile(const std::string &fname, Record &result);
  };
}
