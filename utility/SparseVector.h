#pragma once

#include <sstream>
#include <vector>

namespace Common {
  class SparseVector {
  private:
    //std::vector<int> m_indices;
    int m_indices[100];
    size_t m_size;
    
  public:
    explicit SparseVector(int predicted_max_index = 100)
      //: m_indices()//new int[predicted_max_index])
      : m_size(0)
    {
      //m_indices.reserve(predicted_max_index);
    }

    SparseVector(const SparseVector &rhs) {
      memcpy(m_indices, rhs.m_indices, sizeof(int)*rhs.m_size);
      m_size = rhs.m_size;
    }

    SparseVector &operator =(const SparseVector &rhs) {
      memcpy(m_indices, rhs.m_indices, sizeof(int)*rhs.m_size);
      m_size = rhs.m_size;
      return *this;
    }

    void clear() {
      //m_indices.clear();
      m_size = 0;
    }

    inline size_t size() const {
      return m_size;
      //return m_indices.size();}
    }
    inline void add(int index) {
      //m_indices.push_back(index);
      m_indices[m_size] = index;
      m_size++;
    }
    inline bool contains(int index) const {
      //return std::find(m_indices.begin(), m_indices.end(), index) != m_indices.end();
      return std::find(m_indices, m_indices+m_size, index) != m_indices+m_size;
    }
    //const std::vector<int> &getList() const {return m_indices;}
    const int * getList() const {return m_indices;}

    int at(size_t index) const { return m_indices[index]; }
    int operator [](size_t index) const {
      return m_indices[index];
    }

    void erase(size_t index) {
      assert (index < m_size);
      m_size--;
      if (index == m_size) {
         return;
      }
      std::swap(m_indices[index], m_indices[m_size]);
    }
    
    std::string toString() const {
      std::stringstream ss;
      ss << "(";
      for (int i=0; i<static_cast<int>(m_size)-1; i++) {
        ss << m_indices[i] << ",";
      }
      if (m_size != 0) ss << m_indices[m_size-1];
      ss << ")";
      return ss.str();      
    }
  };
}
