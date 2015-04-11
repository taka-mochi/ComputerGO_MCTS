#include "precomp.h"

#include <iostream>
#include <unordered_map>
#include <stdio.h>
#include <sstream>

#include <getopt.h>
#include <fstream>

#include "Record/Record.h"
#include "Record/SgfReader.h"
#include "ML/PatternExtractor_3x3.h"
#include "utility/utility.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

void usage() {
  cout << 
    "-h        : help (show this message)" << endl <<
    "-m        : multi file mode" << endl << 
    "-i input  : input file (if -m, set a directory)" << endl <<
    "-o output : output file (if -m, merged result will be saved)" << endl;
}


typedef std::pair<int,int> PatternAppearCount;
typedef unordered_map<PATTERN_3x3_HASH_VALUE, PatternAppearCount> PatternAppearTable;

struct BattleStatistics {
  // 総局面数、総ゲーム数、置石、ボードサイズ、(終了の仕方、)棋力差
  int positionCount;
  int gameCount;
  map<int, int> handicapGameCounts;
  map<int, int> boardsizeCounts;
  map<int, int> skillDifferenceCounts;
  map<int, int> positionsPerGameCounts;

  BattleStatistics()
    : positionCount(0)
    , gameCount(0)
    , handicapGameCounts()
    , boardsizeCounts()
    , skillDifferenceCounts()
    , positionsPerGameCounts()
  {}

  BattleStatistics &operator += (const BattleStatistics &rhs) {
    positionCount += rhs.positionCount;
    gameCount += rhs.gameCount;
    for (map<int,int>::const_iterator it = rhs.handicapGameCounts.begin();
         it != rhs.handicapGameCounts.end(); it++) {
      handicapGameCounts[it->first] += it->second;
    }
    for (map<int,int>::const_iterator it = rhs.boardsizeCounts.begin();
         it != rhs.boardsizeCounts.end(); it++) {
      boardsizeCounts[it->first] += it->second;
    }
    for (map<int,int>::const_iterator it = rhs.skillDifferenceCounts.begin();
         it != rhs.skillDifferenceCounts.end(); it++) {
      skillDifferenceCounts[it->first] += it->second;
    }
    for (map<int,int>::const_iterator it = rhs.positionsPerGameCounts.begin();
         it != rhs.positionsPerGameCounts.end(); it++) {
      positionsPerGameCounts[it->first] += it->second;
    }

    return *this;
  }
};

void multi_file_processor(const string &input, const string &output);
BattleStatistics single_file_processor(const string &input, const string &output, shared_ptr<PatternAppearTable> result_buffer = shared_ptr<PatternAppearTable>(), bool do_clean_result_buffer = false);

int main(int argc, char **argv)
{
  // what are needed as arguments?
  //  SGF file (list)
  //  Output file (list)

  bool multi_file_mode = false;

  string input, output;

  char c;
  while ((c = getopt(argc, argv, "hmi:o:")) != -1) {
    switch (c) {
    case 'h':
      usage();
      exit(0);
    case 'm':
      multi_file_mode = true;
      break;
    case 'i':
      input = optarg;
      break;
    case 'o':
      output = optarg;
      break;
    }
  }

  if (input.empty() || output.empty()) {
    usage();
    exit(0);
  }

  if (multi_file_mode) {
    multi_file_processor(input, output);
  } else {
    single_file_processor(input, output);
  }

  return 0;
}


bool output_result(const PatternAppearTable &table, const BattleStatistics &statistics, const string &output)
{
  ofstream ofs(output);
  if (!ofs) return false;

  // header 
  // 総局面数、総ゲーム数、置石、ボードサイズ、(終了の仕方、)棋力差、手数統計
  ofs << "position_count," << statistics.positionCount << endl;
  ofs << "game_count," << statistics.gameCount << endl;
  for (map<int,int>::const_iterator it = statistics.boardsizeCounts.begin();
       it != statistics.boardsizeCounts.end(); it++) {
    ofs << "boardsize_" << it->first << "," << it->second << endl;
  }
  for (map<int,int>::const_iterator it = statistics.handicapGameCounts.begin();
       it != statistics.handicapGameCounts.end(); it++) {
    ofs << "handicap_" << it->first << "," << it->second << endl;
  }
  for (map<int,int>::const_iterator it = statistics.skillDifferenceCounts.begin();
       it != statistics.skillDifferenceCounts.end(); it++) {
    ofs << "skilldiff_" << it->first << "," << it->second << endl;
  }
  for (map<int,int>::const_iterator it = statistics.positionsPerGameCounts.begin();
       it != statistics.positionsPerGameCounts.end(); it++) {
    ofs << "positions_" << it->first << "," << it->second << endl;
  }

  // main part
  ofs << "hash,appear,executed,executed_rate" << endl;

  PatternAppearTable::const_iterator it,end = table.end();
  for (it=table.begin(); it!=end; it++) {
    ofs << hex << it->first << "," << dec << it->second.first << "," << it->second.second << "," << static_cast<double>(it->second.second)/it->second.first << endl;
  }

  return true;
}


//
// Multi Files
//
void multi_file_processor(const string &input, const string &output)
{
  stringstream command;
  command << "find " << input << " -name \"*.sgf\"";
  FILE *in_pipe = popen(command.str().c_str(), "r");

  if (in_pipe == NULL) {
    cerr << "failed to open pipe: " << command << endl;
    return;
  }

  // get 'find' result
  string result;
  char buf[512];
  int read_num=0;
  do {
    read_num = fread(buf, sizeof(char), 511, in_pipe);
    buf[read_num] = '\0';
    if (read_num>0) result += buf;
  } while (read_num > 0);
  pclose(in_pipe);

  // remove all '\r'
  while (true) {
    string::size_type pos = result.find('\r');
    if (pos == string::npos) {
      break;
    }
    result = result.substr(0,pos) + result.substr(pos+1);
  }

  // split
  vector<string> files(split(result, "\n"));

  string output_base(output);
  char lastchar = output[output.size()-1];
  if (lastchar != '/' && lastchar != '\\') {
    output_base += '/';
  }

  shared_ptr<PatternAppearTable> appear_table(new PatternAppearTable);

  BattleStatistics statistics;

  for (vector<string>::iterator it=files.begin(); it!=files.end(); it++) {
    cerr << "input SGF: " << *it << endl;
    statistics += single_file_processor(*it, "", appear_table);
  }

  if (output_result(*appear_table, statistics, output)) {
    cerr << "Success for multi processor. Output file name = " << output << endl;
  } else {
    cerr << "Failed in multi processor. Failed to write output file = " << output << endl;
  }
}


//
// Single File Processor
//

BattleStatistics single_file_processor(const string &input, const string &output, shared_ptr<PatternAppearTable> result_buffer, bool do_clean_result_buffer)
{
  // read sgf file
  Record record;
  SgfReader::SGF_READ_RESULT ret = SgfReader::readFromFile(input, record);
  if (ret != SgfReader::SGF_READ_OK) {
    cerr << "failed to read file: " << input << " with error code: " << ret << endl;
    return BattleStatistics();
  }

  PatternExtractor_3x3 pat_extractor(record.getBoardSize());
  
  // pattern_count[hash].first <= how many "hash" does appear in this SGF
  // pattern_count[hash].second <= how many "hash" was moved in this SGF
  shared_ptr<PatternAppearTable> pattern_count;
  if (result_buffer != NULL) {
    pattern_count = result_buffer;
    if (do_clean_result_buffer) pattern_count->clear();
  } else {
    pattern_count.reset(new PatternAppearTable);
  }

  // save statistics
  BattleStatistics statistics;
  statistics.gameCount = 1;
  statistics.boardsizeCounts[record.getBoardSize()]++;
  statistics.handicapGameCounts[record.getHandicapCount()]++;
  statistics.skillDifferenceCounts[Record::calcSkillDifference(record.getBlackRank(), record.getWhiteRank())]++;

  //if (record.getHandicapCount() != 0) return BattleStatistics();
  //if (record.getBoardSize() == 19) return BattleStatistics();
  //if (record.getMoveSequence().size() < 26) return BattleStatistics();

  // parse SGF
  int count = 0;
  Record::BoardIterator board_it = record.createBoardIterator();
  Point freePoints[MAX_BOARD_SIZE];
  int freePointCount = 0;
  while (!board_it.isLast()) {
    count++;
    statistics.positionCount++;

    const Board &board = board_it.getCurrentBoard();
    Color turn = board_it.getNextTurn();
    Record::Move nextMove(board_it.getNextMove());
    Point nextPoint = nextMove.x == PASS ? PASS : board.xyToPoint(nextMove.x, nextMove.y);

    board.enumerateFreeMoves(freePoints, freePointCount, turn);

    // extract patterns from all legal moves
    for (int i=0; i<freePointCount; i++) {
      if (board.checkLegalHand(freePoints[i], turn, Board::flipColor(turn)) == Board::PUT_LEGAL) {
        PATTERN_3x3_HASH_VALUE val = pat_extractor.encode(board, freePoints[i], turn == WHITE);
        ++(*pattern_count)[val].first;

        if (freePoints[i] == nextPoint) {
          ++(*pattern_count)[val].second;
        }
      }
    }
    board_it.moveNext();
  };

  statistics.positionsPerGameCounts[(count/10)*10]++;

  cerr << "Success for single processor. Board count = " << count << endl;

  if (!output.empty()) {
    if (!output_result(*pattern_count, statistics, output)) {
      cerr << "failed to write results to " << output << endl;
      return BattleStatistics();
    }
  }

  return statistics;
}
