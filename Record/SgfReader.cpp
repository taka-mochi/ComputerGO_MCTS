
#include "precomp.h"
#include "Record/SgfReader.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using namespace Common;
using Common::SgfReader;

namespace Common {

  std::string SgfReader::m_prevProperty;

  typedef SgfReader::SGF_READ_RESULT (*PropertyProcesser)(const std::string &val, Record &result);

  std::map<std::string, PropertyProcesser> processor_map;

  SgfReader::SGF_READ_RESULT processor_GM(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_SZ(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_FF(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_PW(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_WR(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_BR(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_PB(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_DT(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_PC(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_KM(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_RE(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_RU(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_OT(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_CA(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_ST(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_AP(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_TM(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_HA(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_AB(const std::string &val, Record &result);
  SgfReader::SGF_READ_RESULT processor_AW(const std::string &val, Record &result);

  //
  // Parse a move in SGF to my custom Move
  // 
  Record::Move parseMove(const std::string &move, int board_size) {
    // we don't support large board size. 
    // move character is represented by 'a'-'s'
    if (move.size() == 0 || move == "tt") return Record::Move(PASS, PASS);
    if (move.size() != 2) {
      cerr << move << endl;
    }
    assert (move.size() == 2);
    return Record::Move(move[0]-'a', (move[1]-'a'));
  }

  SgfReader::SGF_READ_RESULT SgfReader::processMoveSequenceLine(const std::string &l, Record &result) {
    std::string line(l);
    if (line[line.size()-1] == ')') line = line.substr(0, line.size()-1);
    //if (line[0] == ';') line = line.substr(1);

    size_t index = 0;
    while (index+3 < line.size()) {
      // B/W[xx];
      if (index+3>=line.size()) break;

      char B_or_W = line[index+1];
      if (B_or_W != 'B' && B_or_W != 'W') return SGF_UNKNOWN_ERROR;
      if (line[index+2] != '[') return SGF_UNKNOWN_ERROR;

      Record::Move move;
      if (line[index+3] == ']') {
        // pass. B/W[]
        move.x = move.y = PASS;
        index += 3;
      } else {
        if (index+5>=line.size()) break;
        move = parseMove(line.substr(index+3, 2), result.getBoardSize());
        index += 5;
      }
      //if (line[index] != ';') break;

      result.addMove(move);

      index++;
    }

    return SGF_READ_OK;
  }

  //
  // Process a line in SGF 
  // 
  SgfReader::SGF_READ_RESULT SgfReader::processOneline(const std::string &line, Record &result) {
    if (line[0] == ';') {
      // move sequences
      return processMoveSequenceLine(line, result);
    } else {
      // some properties
      if (line.size() == 0) return SGF_READ_OK;

      if (line[0] == ')') {
        // end of the file
        return SGF_READ_OK;
      }

      if (line.size() < 3) {
        // ???
        return SGF_ILLEGAL_PROPERTY;
      }

      if (line[0] == '(' && line[1] == ';') {
        // multi sequence
        return SGF_MULTI_SEQUENCE;
      }

      string::size_type begin_param_pos = line.find('[');
      if (begin_param_pos == string::npos) return SGF_ILLEGAL_PROPERTY;

      std::string prop_name(line.substr(0,begin_param_pos));
      string::size_type end_param_pos = line.find(']');
      //cerr << line << " begin:" << begin_param_pos << " end:" << end_param_pos << endl;
      std::string prop_val;
      std::string rest_str;
      if (end_param_pos == string::npos) {
        prop_val = line.substr(begin_param_pos);
      } else {
        prop_val = line.substr(begin_param_pos,end_param_pos-begin_param_pos+1);
        rest_str = line.substr(end_param_pos+1);
      }
      if (prop_name.empty()) {
        // the same property name as previous
        prop_name = m_prevProperty;
        //prop_val = line;
      }
      prop_val = prop_val.substr(0, prop_val.size()-1);
      prop_val = prop_val.substr(1);
      //if (prop_val.size() == 0) return SGF_NOT_SUPPORTED;
      // handler check
      if (processor_map[prop_name] == NULL) {
        std::cerr << "Cannot handle property '" << prop_name << "'" << std::endl;
        //return SGF_NOT_SUPPORTED;
      } else {
        // handle property
        SGF_READ_RESULT ret;
        if ((ret = processor_map[prop_name](prop_val, result)) != SGF_READ_OK) {
          std::cerr << "Error occurred for property '" << prop_name << "'. Error code: " << ret << std::endl;
          return ret;
        }
        m_prevProperty = prop_name;
      }

      if (!rest_str.empty()) {
        return processOneline(rest_str, result);
      }
    }

    return SGF_READ_OK;
  }

  //
  // Read SGF entry point
  //
  SgfReader::SGF_READ_RESULT SgfReader::readFromFile(const string &fname, Record &result) {

    m_prevProperty = "";

    processor_map["GM"] = processor_GM;
    processor_map["SZ"] = processor_SZ;
    processor_map["FF"] = processor_FF;
    processor_map["PW"] = processor_PW;
    processor_map["WR"] = processor_WR;
    processor_map["BR"] = processor_BR;
    processor_map["PB"] = processor_PB;
    processor_map["DT"] = processor_DT;
    processor_map["PC"] = processor_PC;
    processor_map["KM"] = processor_KM;
    processor_map["RE"] = processor_RE;
    processor_map["RU"] = processor_RU;
    processor_map["OT"] = processor_OT;
    processor_map["CA"] = processor_CA;
    processor_map["ST"] = processor_ST;
    processor_map["AP"] = processor_AP;
    processor_map["TM"] = processor_TM;
    processor_map["HA"] = processor_HA;
    processor_map["AB"] = processor_AB;
    processor_map["AW"] = processor_AW;

    // open file
    ifstream ifs(fname.c_str());
    if (!ifs) return SGF_FAILED_TO_OPEN_FILE;

    std::string line;
    std::getline(ifs, line);
    while (line.size()>0 && (line[line.size()-1] == '\n' || line[line.size()-1] == '\r')) line = line.substr(0, line.size()-1);

    if (line[0] == '(' && line[1] == ';') {
      line = line.substr(2);
    } else {
      return SGF_NOT_SUPPORTED;
    }

    SGF_READ_RESULT ret;
    if ((ret = processOneline(line, result)) != SGF_READ_OK) {
      return ret;
    }

    // read sequence
    while (!ifs.eof()) {

      std::getline(ifs, line);
      while (line.size()>0 && (line[line.size()-1] == '\n' || line[line.size()-1] == '\r')) line = line.substr(0, line.size()-1);
      if (line.size() == 0) {
        continue;
      }

      if ((ret = processOneline(line, result)) != SGF_READ_OK) {
        return ret;
      }
    }

    if (!result.checkConsistencyForHandicap()) {
      return SGF_NO_CONSISTENCY_FOR_HANDICAP;
    }


    return SGF_READ_OK;
  }

  //
  // handlers for each Property
  //
  SgfReader::SGF_READ_RESULT processor_GM(const std::string &val, Record &result) {
    if (val != "1") return SgfReader::SGF_NOT_GO;
    return SgfReader::SGF_READ_OK;
  }

  SgfReader::SGF_READ_RESULT processor_SZ(const std::string &val, Record &result) {
    result.setBoardSize(atoi(val.c_str()));
    return SgfReader::SGF_READ_OK;
  }

  SgfReader::SGF_READ_RESULT processor_FF(const std::string &val, Record &result) {
    //std::cerr << "Version of SGF = " << val << endl;
    return SgfReader::SGF_READ_OK;
  }

  SgfReader::SGF_READ_RESULT processor_PB(const std::string &val, Record &result)
  {
    result.setBlackPlayerName(val);
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_PW(const std::string &val, Record &result) 
  {
    result.setWhitePlayerName(val);
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_WR(const std::string &val, Record &result)
  {
    result.setWhiteRank(val);
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_BR(const std::string &val, Record &result)
  {
    result.setBlackRank(val);
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_DT(const std::string &val, Record &result)
  {
    result.setDate(val);
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_PC(const std::string &val, Record &result)
  {
    // PC: place. ignore
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_KM(const std::string &val, Record &result)
  {
    if (val == "0" || val == "0.0" || val == "0.00" || val == "0.000") result.setKomi(0);
    else {
      double komi = atof(val.c_str());
      if (komi != 0) result.setKomi(komi);
      else return SgfReader::SGF_NOT_SUPPORTED;
    }
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_RE(const std::string &val, Record &result)
  {
    // [B/W]+[Num/R/time]
    // Draw?
    string::size_type index = val.find('+');
    if (index != string::npos) {
      // winner
      string winner = val.substr(0, index);
      string res = val.substr(index+1);
      if (winner == "W" || winner == "w") {
        result.setWinner(WHITE);
      } else if (winner == "B" || winner == "b") {
        result.setWinner(BLACK);
      } else {
        return SgfReader::SGF_ILLEGAL_PROPERTY;
      }
      result.setResult(res);
    } else {
      // Draw game
      result.setResultToDraw();
    }
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_RU(const std::string &val, Record &result)
  {
    // Rule set: Japanese, Chinese, ...
    // This is not so important
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_OT(const std::string &val, Record &result)
  {
    // OT: Overtime system (byo-yomi, mochi-time, ...)
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_CA(const std::string &val, Record &result)
  {
    // CA: Character Encoding
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_ST(const std::string &val, Record &result)
  {
    // ???
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_AP(const std::string &val, Record &result)
  {
    // AP: Application
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_TM(const std::string &val, Record &result)
  {
    // TM: Timelimit
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_HA(const std::string &val, Record &result)
  {
    // HA: Handicap
    if (val == "0") result.setHandicapCount(0);
    else {
      int h = atoi(val.c_str());
      if (h != 0) result.setHandicapCount(h);
      else return SgfReader::SGF_ILLEGAL_PROPERTY;
    }
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_AB(const std::string &val, Record &result)
  {
    // AB: Add black
    result.addHandicapPoint(parseMove(val, result.getBoardSize()));
    return SgfReader::SGF_READ_OK;
  }
  SgfReader::SGF_READ_RESULT processor_AW(const std::string &val, Record &result)
  {
    // AW: Add white
    // Prior putting for white is not supported!!!
    return SgfReader::SGF_NOT_SUPPORTED;
  }

}
