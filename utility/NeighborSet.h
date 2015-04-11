
#pragma once

#define ITERATOR_CLASS_DEFINITION(CNAME)     


namespace Common {
  template <typename T> class NeighborSet {

  public:
    // iterator class
    
  /*   class Iterator { */
  /*   private:  */
  /*     T *m_current; */
  /*   public: */
  /*     Iterator() : m_current(NULL) {} */
  /*     Iterator(T *init) : m_current(init) {} */
  /*     Iterator(const Iterator &rhs) : m_current(rhs.m_current) {} */
  /*     inline Iterator& operator=(const Iterator &rhs)  { */
  /*       m_current = rhs.m_current; */
  /*       return *this; */
  /*     } */
  /*     inline Iterator& operator++() { */
	/* m_current++; */
  /*       return *this; */
  /*     } */
  /*     inline Iterator& operator--() { */
  /*       m_current--; */
  /*       return *this; */
  /*     } */
  /*     inline  T& operator *() { */
	/* assert(m_current); */
  /*       return *m_current; */
  /*     } */
  /*     inline bool operator==(const Iterator &rhs) const { */
  /*       return m_current == rhs.m_current; */
  /*     } */
  /*     inline bool operator!=(const Iterator &rhs) const { */
  /*       return !(m_current == rhs.m_current); */
  /*     } */
  /*   }; */

  private:
    T m_elems[4];  // max element does not exceed 4
    size_t m_elem_count;
    //Iterator m_begin, m_end;
    //Const_Iterator m_cbegin, m_cend;

  public:
  NeighborSet() : m_elem_count(0) //, m_begin(m_elems), m_end(m_elems)
      //, m_cbegin(this), m_cend(this, 0)
    {
    }
    ~NeighborSet() {m_elems[0] = m_elems[1] = m_elems[2] = m_elems[3] = T();}

    size_t size() const {
      return m_elem_count;
    }

    void push_back(const T &t) {
      m_elems[m_elem_count++] = t;
      //++m_end;
      //++m_cend;
    }
    void erase(const T &t) {
      for (int i=0; i<m_elem_count; i++) {
        if (t == m_elems[i]) {
          for (int j=i+1; j<m_elem_count; j++) {
            m_elems[j-1] = m_elems[j];
          }
          m_elem_count--;
          //--m_end;
          //--m_cend;
        }
      }
    }
    void clear() {
      m_elem_count = 0;
      //m_end = Iterator(m_elems);
    }
    int find(const T &t) {
      for (int i=0; i<m_elem_count; i++) {
        if (t == m_elems[i]) {
          return i;
        }
      }
      return -1;
    }
/*
    Const_Iterator find(const T &t) const {
      for (int i=0; i<m_elem_count; i++) {
        if (t == m_elems[i]) {
          return const_iterator(this, i);
        }
      }
      return m_cend;
    }
*/
    inline T &at(size_t i) {
      return m_elems[i];
    }
    inline const T &at(size_t i) const{
      return m_elems[i];
    }
    T &operator [](size_t i) {
      return m_elems[i];
    }
    const T &operator [] (size_t i) const {
      return m_elems[i];
    }
    /* inline Iterator begin() { */
    /*   return m_begin; */
    /* } */
    /* inline Iterator end() { */
    /*   return m_end; */
    /* } */
    inline bool contains(const T& t) const {
      return std::find(m_elems, m_elems + m_elem_count, t) != (m_elems + m_elem_count);
      /* for (size_t i=0; i<m_elem_count; i++) { */
      /*   if (m_elems[i] == t) return true; */
      /* } */
      /* return false; */
    }
/*
    const_Iterator begin() const {
      return m_cbegin;
    }
    Const_Iterator end() const {
      return m_cend;
    }
*/
  };
}
