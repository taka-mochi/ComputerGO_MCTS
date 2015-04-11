
#pragma once

#include <iostream>
#include <vector>

namespace Gtp {
  class GtpCore {
    std::ostream &m_out, &m_console;
    std::istream &m_in;

    GtpCore(std::ostream &out, std::istream &in, std::ostream &console);

    void write(const std::string &line);
    void flush();
  public:
    static GtpCore &getInstance() {
      static GtpCore core(std::cout, std::cin, std::cerr);
      return core;
    }

    void sendGtp(const std::string &command);
    bool isEndOfInput();
    bool getLine(std::string &line);
    bool getCommand(std::vector<std::string> &parsed_commands);
    void writeConsoleLn(const std::string &line);
  };
}
