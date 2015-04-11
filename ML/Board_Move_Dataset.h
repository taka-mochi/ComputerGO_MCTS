#pragma once

#include <memory>
#include <vector>
#include "common.h"

namespace Go {
  class Board;
};

namespace ML {
  struct Board_Move_Data {
    std::shared_ptr<Go::Board> state;
    Common::Point move;
    Common::Color turn;
    Common::Point kouMove;
    Board_Move_Data() : state(), move(-1), turn(-1), kouMove(-1) {}
    Board_Move_Data(std::shared_ptr<Go::Board> s, Common::Point m, Common::Color c, Common::Point kou) : state(s), move(m), turn(c), kouMove(kou) {}
    ~Board_Move_Data() {state.reset();}
  };

  //template <typename IteratorType>
  class Board_Move_Dataset {
  public:
    //typedef IteratorType IterData;

  public:
    //Board_Move_Dataset() {}
    virtual ~Board_Move_Dataset() {}

    //virtual size_t size() const = 0;
    //virtual const Board_Move_Data &get(size_t i) const = 0;
    //virtual Board_Move_Data &get(size_t i) = 0;
    
    virtual void seekToBegin() = 0;
    virtual bool empty() const = 0;
    virtual bool next() = 0; // if next() returns false, there is no next data 
    virtual Board_Move_Data get() const = 0;
    //virtual Board_Move_Data &get() = 0;

    //virtual IteratorType begin() = 0;
    //virtual IteratorType end() = 0;
  };


  typedef std::vector<Board_Move_Data> VectorMoveData;
  typedef VectorMoveData::iterator VectorMoveDataIter;

  //class Board_Move_Dataset_vector : public Board_Move_Dataset<VectorMoveDataIter> {
  class Board_Move_Dataset_vector : public Board_Move_Dataset {
  public:
    //typedef std::vector<Data>::const_iterator IterConstData;

  private:
    mutable size_t m_index;
    VectorMoveData m_dataSet;

  public:
    Board_Move_Dataset_vector();
    virtual ~Board_Move_Dataset_vector();

    void addData(const Board_Move_Data &d);
    void addData(std::shared_ptr<Go::Board> state, Common::Point move, Common::Color turn, Common::Point kouMove);

    //size_t size() const {return m_dataSet.size();}
    void seekToBegin() {m_index = 0;};
    bool empty() const {return m_dataSet.empty();}
    bool next() {
      if (m_index+1 >= m_dataSet.size()) return false;
      m_index++; return true;
    }
    virtual Board_Move_Data get() const {return m_dataSet.at(m_index);}
    //virtual Board_Move_Data &get() {return m_dataSet.at(m_index);}
    
    //VectorMoveDataIter begin() {return m_dataSet.begin();}
    //VectorMoveDataIter end() {return m_dataSet.end();}
  };
}
