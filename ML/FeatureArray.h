
#pragma once

#include "../precomp.h"
#include "../common.h"
#include <sstream>
#include <iomanip>

namespace Go {
  class Board;
}

namespace ML {
  // Wrapper of array
  template <class T>
    class FeatureArrayWrapper {
  private:
    T *m_weights;
    size_t m_size;

  public:
    explicit FeatureArrayWrapper()
      : m_weights(NULL)
      , m_size(0)
    {
    }
    explicit FeatureArrayWrapper(size_t size)
      : m_weights(size > 0 ? new T[size] : NULL)
      , m_size(size)
    {
      clear();
    }
    ~FeatureArrayWrapper() 
    {
      delete [] m_weights;
    }
    FeatureArrayWrapper(const FeatureArrayWrapper<T> &rhs)
      : m_weights(NULL)
      , m_size(0)
    {
      this->operator=(rhs);
    }

    void init(size_t new_size) {
      if (m_size != new_size) {
        m_size = new_size;
        delete [] m_weights;
        m_weights = new T[m_size];
      }
      clear();
    }

    void clear() {
      memset(m_weights, 0, sizeof(T)*m_size);
      //for (size_t i=0; i<m_size; i++) {
      //  m_weights[i] = 0;
      //}
    }

    void setFromArray(T *array, size_t size) {
      if (m_size != size) {
        m_size = size;
        delete [] m_weights;
        m_weights = new T[m_size];
      }
      for (size_t i=0; i<size; i++) {
        //memcpy(m_weights, array, sizeof(T)*size);
        m_weights[i] = array[i];
      }
    }

    inline size_t size() const {return m_size;}
    inline void set(size_t index, T v) {assert(index<m_size); m_weights[index] = v;}

    inline const T &at(size_t i) const {assert(i<m_size); return m_weights[i];}
    inline T &at(size_t i) {assert(i<m_size); return m_weights[i];}
    inline const T &operator [](size_t i) const {return at(i);}
    inline T &operator [](size_t i) {return at(i);}

    inline const T* raw() const {return m_weights;}

    inline T dot(const FeatureArrayWrapper<T> &rhs) const {
      T res = 0;
      for (size_t i=0; i<m_size; i++) {
        res += m_weights[i]*rhs.m_weights[i];
      }
      return res;
    }
    inline T dot(const Common::SparseVector &rhs) const {
      T res = 0;
      //const std::vector<int> &indices = rhs.getList();
      for (size_t i=0; i<rhs.size(); i++) {
        res += m_weights[rhs[i]];
      }
      return res;
    }
    inline T multiplyAll(const Common::SparseVector &rhs) const {
      T res = 1;
      //const std::vector<int> &indices = rhs.getList();
      for (size_t i=0; i<rhs.size(); i++) {
        assert ((signed)m_size > rhs[i]);
        res *= m_weights[rhs[i]];
      }
      return res;
    }

    inline FeatureArrayWrapper<T> &operator += (const FeatureArrayWrapper<T> &rhs) {
      assert(m_size == rhs.m_size);
      for (size_t i=0; i<m_size; i++) {
        m_weights[i] += rhs.m_weights[i];
      }
      return *this;
    }

    inline FeatureArrayWrapper<T> &operator -= (const FeatureArrayWrapper<T> &rhs) {
      assert(m_size == rhs.m_size);
      for (size_t i=0; i<m_size; i++) {
        m_weights[i] -= rhs.m_weights[i];
      }
      return *this;
    }

    inline FeatureArrayWrapper<T> &plusDiffOfTwoInstances(const FeatureArrayWrapper<T> &rhs1, const FeatureArrayWrapper<T> &rhs2) {
      assert (m_size == rhs1.m_size);
      assert (m_size == rhs2.m_size);
      for (size_t i=0; i<m_size; i++) {
        m_weights[i] += rhs1.m_weights[i] - rhs2.m_weights[i];
      }
      return *this;
    }

    inline FeatureArrayWrapper<T> &minusDiffOfTwoInstances(const FeatureArrayWrapper<T> &rhs1, const FeatureArrayWrapper<T> &rhs2) {
      assert (m_size == rhs1.m_size);
      assert (m_size == rhs2.m_size);
      for (size_t i=0; i<m_size; i++) {
        m_weights[i] -= rhs1.m_weights[i] - rhs2.m_weights[i];
      }
      return *this;
    }

    inline FeatureArrayWrapper<T> operator * (T val) const {
      FeatureArrayWrapper<T> lhs(size());
      for (size_t i=0; i<m_size; i++) {
        lhs[i] = m_weights[i] * val;
      }
      return lhs;
    }

    inline FeatureArrayWrapper<T> &operator *= (T val) {
      for (size_t i=0; i<m_size; i++) {
        m_weights[i] *= val;
      }
      return *this;
    }

    inline FeatureArrayWrapper<T> &operator = (const FeatureArrayWrapper<T> &rhs) {
      if (m_size != rhs.m_size) {
        m_size = rhs.m_size;
        delete [] m_weights;
        m_weights = new T[m_size];
      }
      memcpy(m_weights, rhs.m_weights, sizeof(T)*m_size);
      /* for (size_t i=0; i<m_size; i++) { */
      /*   m_weights[i] = rhs.m_weights[i]; */
      /* } */
      return *this;
    }

    std::string toString() const {
      std::stringstream ss;
      ss << "(";
      for (size_t i=0; i+1<m_size; i++) {
        ss << std::setprecision(15) << m_weights[i] << ",";
      }
      if (m_size>0) {
        ss << m_weights[m_size-1];
      }
      ss <<  ")";
      return ss.str();
    }
  };

  typedef FeatureArrayWrapper<int>    FeatureValues;
  typedef FeatureArrayWrapper<double> FeatureWeights;
}
