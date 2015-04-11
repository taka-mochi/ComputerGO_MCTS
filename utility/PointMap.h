
#pragma once

#include "common.h"

#include <vector>
#include <bitset>
#include <set>
#include <list>
#include <algorithm>

#include <iostream>
#include <cstring>

namespace Common {

  //typedef std::vector<Point>::const_iterator PointListConstIterator;

  // Custom set for Go Point
  template <typename T>
    class PointMap {

    struct exist_chain_list {
      T m_value;
      exist_chain_list *next, *prev;
      inline void insertNext(exist_chain_list *insertData) {
        if (next) {
          next->prev = insertData;
        }
        insertData->next = next;
        insertData->prev = this;
        this->next = insertData;
      }
      inline void removeThis() {
        if (next) {
          next->prev = prev;
        }
        if (prev) {
          prev->next = next;
        }
        next = prev = NULL;
      }

    exist_chain_list()
    : next(NULL), prev(NULL)
      {
      }
      inline bool isDead() const {
        return (next==NULL && prev==NULL);
      }
    };

  public:
    class PointMapConstIterator {
      template <typename Type> friend class PointMap;
      exist_chain_list *current;
    public:
    PointMapConstIterator() : current(NULL) {}
      explicit PointMapConstIterator(exist_chain_list *init) : current(init) {}
      
      inline bool operator==(const PointMapConstIterator &rhs) const {
        return current == rhs.current;
      }
      inline bool operator!=(const PointMapConstIterator &rhs) const {
        return current != rhs.current;
      }

      inline const T &operator*() const {
        return current->m_value;
      }
      inline PointMapConstIterator &operator++() {
        current = current->next;
        return *this;
      }
      inline PointMapConstIterator &operator--() {
        current = current->prev;
        return *this;
      }
      inline PointMapConstIterator &operator++(int) {
        current = current->next;
        return *this;
      }
      inline PointMapConstIterator &operator--(int) {
        current = current->prev;
        return *this;
      }
    };

  private:
    std::bitset<MAX_BOARD_SIZE> m_hasFlagSet;
    //std::vector<Point> m_pointList;
    //exist_chain_list m_pointList[MAX_BOARD_SIZE];
    T m_pointList[MAX_BOARD_SIZE];
    int m_pointCount;
    //exist_chain_list m_listHead;

    //mutable PointMapConstIterator m_begin, m_end;

    inline void copyFrom(const PointMap<T> &rhs) {
      m_pointCount = rhs.m_pointCount;
      m_hasFlagSet = rhs.m_hasFlagSet;
      std::copy(rhs.m_pointList, rhs.m_pointList+m_pointCount, m_pointList);
    }
  public:
    PointMap() 
      : m_hasFlagSet()
      //, m_pointList()
      , m_pointCount(0)
      //, m_listHead()
      //, m_begin(), m_end()
    {
      //m_pointList.reserve(MAX_BOARD_SIZE);
    }

    inline PointMap(const PointMap<T> &rhs)
      : m_hasFlagSet()
      , m_pointList()
      , m_pointCount(0)
    {
      copyFrom(rhs);
    }
    inline PointMap &operator = (const PointMap<T> &rhs) {
      copyFrom(rhs);
      return *this;
    }

    void clear() {
      m_hasFlagSet = 0;
      //std::fill(m_hasFlagSet, m_hasFlagSet + MAX_BOARD_SIZE, false);
      m_pointCount = 0;
    }

    void insert(Point p, T v) {
      //if (m_pointList[p].isDead()) {
      if (!m_hasFlagSet[p]) {
        // exist_chain_list.isDead == true => this value does not exist
        //m_pointList[p].m_value = v;
        //m_listHead.insertNext(m_pointList+p);
        m_pointList[m_pointCount] = v;
        m_pointCount++;
        m_hasFlagSet.set(p);
      }
    }

    void erase(Point p) {
      if (m_hasFlagSet[p]) {
        //if (!m_pointList[p].isDead()) {
        // exist_chain_list.isDead == false => this value exists (should be removed)
        //m_pointList[p].removeThis();
        size_t index = std::find(m_pointList, m_pointList+m_pointCount, p) - m_pointList;
        m_pointList[index] = m_pointList[m_pointCount-1];
        m_pointCount--;
        m_hasFlagSet.reset(p);
      }
    }

    inline bool contains(Point p) const {
      return m_hasFlagSet[p];
    }

    inline size_t size() const {
      return m_pointCount;
    }

    inline T &operator[](size_t index) {
      return m_pointList[index];
    }
    inline const T &operator[](size_t index) const {
      return m_pointList[index];
    }

    //typedef PointMapConstIterator ListConstIterator;
    typedef const T * ListConstIterator;
    inline ListConstIterator begin() const {
      //m_begin.current = m_listHead.next;
      //return m_begin;
      return m_pointList;
    }
    inline ListConstIterator end() const {
      //return m_end;
      return m_pointList+m_pointCount;
    }

    inline ListConstIterator find(T v) const {
      return std::find(m_pointList, m_pointList+m_pointCount, v);
      /*
        PointMapConstIterator current;
        current.current = m_listHead.next;
        while (current != m_end) {
        if (*current == v) {
        return current;
        }
        current++;
        }
        return m_end;
      */
    }
  };

  typedef PointMap<Point> PointSet;
  //typedef const Point * PointListConstIterator;
  typedef PointSet::ListConstIterator PointListConstIterator;
}
