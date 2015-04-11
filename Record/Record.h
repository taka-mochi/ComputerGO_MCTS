#pragma once

#include <vector>
#include <string>
#include <memory>

#include "common.h"

namespace Go {
  class Board;
}

namespace Common {
  class Record {
  public:
    struct Move {
      int x,y;
      Move() : x(0), y(0) {}
      Move(int x_, int y_) : x(x_), y(y_) {}
    };
  private:
    std::vector<Move> m_moveSequence;
    int m_handicapCount;
    int m_boardSize;
    Color m_winner;
    double m_komi;
    std::string m_result;
    std::string m_playerNames[2];
    std::string m_playerRank[2];
    std::string m_date;
    std::vector<Move> m_priorForHandi;

    // game result: such as, W+3.5, B+time, W+resign

  public:
    Record()
      :m_moveSequence()
      , m_handicapCount(0)
      , m_boardSize(19)
      , m_winner(BLACK)
      , m_komi(6.5)
      , m_result()
      , m_date()
      , m_priorForHandi()
    {
    }
    Record(const Record &rhs)
      : m_moveSequence(rhs.m_moveSequence)
      , m_handicapCount(rhs.m_handicapCount)
      , m_winner(rhs.m_winner)
      , m_komi(rhs.m_komi)
      , m_result(rhs.m_result)
      , m_date(rhs.m_date)
      , m_priorForHandi(rhs.m_priorForHandi)
    {
      m_playerNames[0] = rhs.m_playerNames[0];
      m_playerNames[1] = rhs.m_playerNames[1];
      m_playerRank[0] = rhs.m_playerRank[0];
      m_playerRank[1] = rhs.m_playerRank[1];
    }
    ~Record() {}

    // getter
    const std::vector<Move> &getMoveSequence() const {return m_moveSequence;}
    int getHandicapCount() const {return m_handicapCount;}
    int getBoardSize() const {return m_boardSize;}
    Color getWinner() const {return m_winner;}
    double getKomi() const {return m_komi;}
    const std::string &getBlackPlayerName() const {return m_playerNames[0];}
    const std::string &getWhitePlayerName() const {return m_playerNames[1];}
    const std::string &getBlackRank() const {return m_playerRank[0];}
    const std::string &getWhiteRank() const {return m_playerRank[1];}
    const std::string &getDate() const {return m_date;}
    const std::string &getResult() const {return m_result;}
    const std::vector<Move> &getHandicapBlacks() const {return m_priorForHandi;}

    // setter
    void addMove(const Move &p) {m_moveSequence.push_back(p);}
    void setHandicapCount(int h) {if(h<0 || h>9) return; m_handicapCount = h;}
    void setBoardSize(int s) {if (s<=0 || s>19) return; m_boardSize = s;}
    void setWinner(Color c) {if (c != BLACK && c != WHITE) return; m_winner = c;}
    void setKomi(double komi) { if (komi < 0) return; m_komi = komi;}
    void setBlackPlayerName(const std::string &name) {m_playerNames[0] = name;}
    void setWhitePlayerName(const std::string &name) {m_playerNames[1] = name;}
    void setBlackRank(const std::string &rank) {m_playerRank[0] = rank;}
    void setWhiteRank(const std::string &rank) {m_playerRank[1] = rank;}
    void setDate(const std::string &date) {m_date = date;}
    void setResult(const std::string &res) {m_result = res;}
    void setResultToDraw() {m_result = "Draw";}
    void addHandicapPoint(const Move &p) {m_priorForHandi.push_back(p);}
    bool checkConsistencyForHandicap() const {return m_priorForHandi.size() == (unsigned)m_handicapCount;}

    static int calcSkillDifference(const std::string &skill1, const std::string &skill2);

    // iterator to get Board
    class BoardIterator {
      friend class Record;

    private:
      const Record *m_record;
      std::shared_ptr<Go::Board> m_board;
      mutable int m_moveIndex;

    private:
      BoardIterator(const Record *record);
      
    public:
      BoardIterator();
      ~BoardIterator();
      void clear();
      void moveNext();
      const Go::Board &getCurrentBoard() const;
      const std::shared_ptr<Go::Board> getCurrentBoardPtr() const;
      Color getLastTurn() const;
      Color getNextTurn() const;
      Record::Move getNextMove() const;
      bool isLast() const;

      //BoardIterator &operator =(const BoardIterator &it) const;
    };
    BoardIterator createBoardIterator() const;
    
  };
}
