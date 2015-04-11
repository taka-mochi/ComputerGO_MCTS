#pragma once

#include "common.h"
#include "ML/Board_Move_Dataset.h"
#include "Record/Record.h"

namespace ML {
  class Board_Move_Dataset_record : public Board_Move_Dataset {
    std::vector<std::shared_ptr<Common::Record> > m_records;
    mutable int m_currentRecord;
    Common::Record::BoardIterator m_currentBoard;
    //mutable Board_Move_Data m_currentMoveData;

    bool changeToNextRecord();

  public:
    Board_Move_Dataset_record();
    virtual ~Board_Move_Dataset_record();

    const std::shared_ptr<Common::Record> getRecord(size_t index) const {return m_records[index];}
    bool readAndAddRecordFromSGF(const std::string &filename);
    bool addRecord(std::shared_ptr<Common::Record> record);

    bool isEndOfData() const;
    void seekToBegin();
    bool empty() const;
    bool next(); // if next() returns false, there is no next data 
    Board_Move_Data get() const;
    //Board_Move_Data &get();
  };
}
