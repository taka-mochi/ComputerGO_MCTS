
#pragma once

#include <vector>
#include <iostream>
#include <typeinfo>
#include <cxxabi.h>

namespace Common {
  template <class T, int AllocUnit> class SingletonMemoryPool {
  private:
    std::vector<T *> m_usableList;
    int m_expandedCount;

    inline void expandUsableList() {
#ifdef DEBUG
      int status;
      std::cerr << "----------- Expansion for " << abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status) << " is executed -----------" << std::endl;
#endif
      m_usableList.reserve(m_usableList.size()+AllocUnit);
      for (int i=0; i<AllocUnit; i++) {
        m_usableList.push_back(new T);
      }
      m_expandedCount++;
    }

    SingletonMemoryPool()
      : m_usableList()
      , m_expandedCount(0)
    {
      expandUsableList();
    }
    ~SingletonMemoryPool()
    {
      int rest = (m_expandedCount * AllocUnit) - (signed)m_usableList.size();
#ifdef DEBUG
      if (rest >= 0) {
#else
      if (rest > 0) {
#endif
        int status;
        std::cerr << rest << " instances are not released!! at memory pool of " << abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status) << std::endl;
      }
      for (size_t i=0; i<m_usableList.size(); i++) {
        delete m_usableList[i];
      }
    }
    SingletonMemoryPool(const SingletonMemoryPool<T,AllocUnit> &) {}
    SingletonMemoryPool<T,AllocUnit> &operator=(const SingletonMemoryPool<T,AllocUnit> &){return *this;}

  public:
    inline static SingletonMemoryPool<T, AllocUnit> &getInstance() {
      static SingletonMemoryPool<T, AllocUnit> s;
      return s;
    }

    inline size_t capacity() const {
      return m_usableList.size();
    }

    inline T *allocate() {
      if (m_usableList.size() == 0) {
        expandUsableList();
      }
      T *p = m_usableList.back();
      m_usableList.pop_back();

      return p;
    }

    inline void release(T *p) {
      m_usableList.push_back(p);
    }
  };
}
