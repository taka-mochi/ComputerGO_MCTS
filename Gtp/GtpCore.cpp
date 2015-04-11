
#include "precomp.h"
#include "Gtp/GtpCore.h"

using namespace std;

namespace Gtp {
  GtpCore::GtpCore(ostream &out, istream &in, ostream &console)
    : m_out(out)
    , m_console(console)
    , m_in(in)
  {
  }
  void GtpCore::write(const std::string &line) {
    m_out << line;
  }
  void GtpCore::flush() {
    m_out.flush();
  }
  void GtpCore::writeConsoleLn(const std::string &line) {
    m_console << line << endl;
  }

  void GtpCore::sendGtp(const std::string &command) {
    write("= " + command + "\n\n");
    flush();
  }
  bool GtpCore::isEndOfInput() {
    return m_in.fail();
  }
  bool GtpCore::getLine(std::string &line) {
    return !getline(m_in, line).fail();
  }
  bool GtpCore::getCommand(std::vector<std::string> &parsed_commands) {
    parsed_commands.clear();
    string line;
    if (!getLine(line)) return false;
    char tmp[10240];
    strcpy(tmp, line.c_str());
    char *tmp1 = strtok(tmp, " \r\n");
    while (tmp1 != NULL) {
      parsed_commands.push_back(tmp1);
      tmp1 = strtok(NULL, " \r\n");
    }
    return true;
  }

}
