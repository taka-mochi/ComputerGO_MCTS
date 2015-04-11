#include "precomp.h"

#include <fstream>
#include <sstream>

#include "GoBook.h"

using namespace std;
using namespace Common;

namespace Go {
  GoBook::GoBook()
    : m_boardSizeToHashEntry()
  {
    for (int i=0; i<20; i++) {
      m_boardSizeToHashEntry.push_back(HashToMoves());
    }
  }
  GoBook::~GoBook() 
  {
  }

  void GoBook::clear() {
    for (int i=0; i<20; i++) {
      m_boardSizeToHashEntry.push_back(HashToMoves());
    }
  }

  bool GoBook::readFromFile(const std::string &file) {
    std::ifstream in(file.c_str());
    if (!in) return false;

    clear();

    while (in)
    {
      string line;
      getline(in, line);
      if (line == "") continue;

      std::istringstream iss(line);
      int size;
      iss >> size;
      if (size < 1) continue;
      if (size >= 20) continue;
      
      vector<pair<int,int> > seq = readPoints(iss);
      vector<pair<int,int> > moves = readPoints(iss);
      addEntryForSequence(seq, moves, size);
    }

    return true;
  }

  vector<pair<int,int> > GoBook::readPoints(std::istream &in) {
    vector<pair<int,int> > result;
    while (true) {
      string s;
      in >> s;
      if (!in || s == "|") break;
      std::istringstream in2(s);
      int col,row;
      string pos;
      in2 >> pos;
      if (pos == "") break;

      if (pos == "PASS" || pos == "pass") {col = PASS, row = PASS;}
      else if (pos == "RESIGN" || pos == "resign") {col = POINT_NULL, row = POINT_NULL;}
      else {
        char c = pos[0];
        if (c >= 'A' && c <= 'Z')
          c = char(c - 'A' + 'a');
        else if (c < 'a' || c > 'z') {
          in.setstate(ios::failbit);
          return vector<pair<int,int> >();
        }
        col = c - 'a' + 1;
        if (c >= 'j')
          --col;
        istringstream sin(pos.substr(1));
        sin >> row;
        col--; row--;
        if (!sin || (col < 0 || col >= MAX_BOARD_SIZE) || (row < 0 || row >= MAX_BOARD_SIZE)) {
          return vector<pair<int,int> >();
        }
      }
      result.push_back(pair<int,int>(col, row));
    }
    return result;
  }

  bool GoBook::existMoveInBook(const Board &board) const {
    if (board.getSize() >= 20) return false;
    const HashToMoves &moves = m_boardSizeToHashEntry[board.getSize()];
    
    HashToMoves::const_iterator it = moves.find(board.getHash().get());
    return it != moves.end();
  }

  std::vector<Common::Point> GoBook::getMovesInBook(const Board &board) const {
    if (board.getSize() >= 20) return std::vector<Common::Point>();
    const HashToMoves &moves = m_boardSizeToHashEntry[board.getSize()];
    
    HashToMoves::const_iterator it = moves.find(board.getHash().get());

    if (it == moves.end()) return std::vector<Common::Point>();
    return it->second;
  }

  Point rotPoint(Point p, bool inverse, int rot_index, const Board &board) {
    int x = board.pointToXY(p).first;
    int y = board.pointToXY(p).second;

    if (inverse) {x = board.getSize() - 1 - x;}
    
    switch (rot_index) {
    case 0:
      return board.xyToPoint(x,y);
    case 1:
      return board.xyToPoint(board.getSize()-y-1, x);
    case 2:
      return board.xyToPoint(board.getSize()-x-1, board.getSize()-y-1);
    case 3:
      return board.xyToPoint(y, board.getSize()-x-1);
    default:
      return POINT_NULL;
    }
  }

  void GoBook::addEntryForSequence(const std::vector<pair<int,int> > &sequence, const std::vector<pair<int,int> > &moves, int boardsize) {
      
    for (int inv=0; inv<2; inv++) {
      for (int rot=0; rot<4; rot++) {
      Color c = BLACK;
        
        Board board(boardsize);
        for (vector<pair<int,int> >::const_iterator it = sequence.begin(); it != sequence.end(); it++) {
          Point mo = board.xyToPoint(it->first, it->second);
          if (mo != PASS) {
            mo = rotPoint(mo, inv == 1, rot, board);
            assert (board.checkLegalHand(mo, c, Board::flipColor(c)) == Board::PUT_LEGAL);
          }
          board.put(mo, c);
          c = Board::flipColor(c);
        }

        for (vector<pair<int,int> >::const_iterator it = moves.begin(); it != moves.end(); it++) {
          Point mo = board.xyToPoint(it->first, it->second);
          if (mo != PASS) mo = rotPoint(mo, inv == 1, rot, board);
          addEntry(board, mo);
        }
      }
    }
  }

  bool GoBook::addEntry(const Board &board, Common::Point move) {
    if (board.getSize() >= 20) return false;

    HashToMoves &moves = m_boardSizeToHashEntry[board.getSize()];
    BOARD_HASH_VALUE h = board.getHash().get();
    HashToMoves::iterator it = moves.find(h);
    std::vector<Common::Point> *target;
    if (it != moves.end()) {
      target = &it->second;
    } else {
      moves[h] = std::vector<Common::Point>();
      target = &moves[h];
    }
    if (find(target->begin(), target->end(), move) == target->end()) {
      target->push_back(move);
    }

    return true;
  }
}
