#pragma once

#include <memory>
#include <vector>

namespace Common {
  class Board;
};

namespace ML {
  class Board_Minimax_Dataset {
  public:
    struct Data {
      std::shared_ptr<Board> state;
      double miniMaxValue;
      Data() : state(), miniMaxValue(0) {}
      Data(std::shared_ptr<Board> s, double v) : state(s), m_miniMaxValue(v) {}
      ~Data() {state.reset();}
    };

  private:
    std::vector<Data> m_dataSet;

  public:
    Board_Minimax_Dataset();
    virtual ~Board_Minimax_Dataset();

    void addData(Data &d);
    void addData(std::shared_ptr<Board> state, double miniMaxValue);

    size_t size() const {return m_dataSet.size();}
    const Data &get(size_t i) const {return m_dataSet.at(i);}
    Data &get(size_t i) {return m_dataSet.at(i);}
    
  };
}
