
#pragma once

#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <unordered_set>
//#include "pointSet.h"

#define MAX_BOARD_SIZE (21*21)

namespace Common {
  typedef int Point;
  typedef int Color;
  typedef uint_fast64_t BOARD_HASH_VALUE;

  template <typename T> class PointMap;
  typedef PointMap<Point> PointList;

  extern const Color FREE;
  extern const Color BLACK;
  extern const Color WHITE;
  extern const Color WALL;
  extern const size_t COLOR_NUM;
  extern const Point PASS;
  extern const Point POINT_NULL;

  extern const double HISTORY_TABLE_INIT_VALUE;

  //typedef std::unordered_set<Point> PointList;
  //typedef std::unordered_set<Point> PointListConstIterator;
/*
  class Point {
  public:
    int x;
    int y;

    Point()
      : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
    Point(const Point &rhs) : x(rhs.x), y(rhs.y) {}
    Point &operator= (const Point &rhs) {
      this->x = rhs.x; this->y = rhs.y;
      return *this;
    }
    bool operator==(const Point &rhs) const{
      return this->x == rhs.x && this->y == rhs.y;
    }
  };
*/
}

namespace ML {
  typedef uint32_t PATTERN_3x3_HASH_VALUE;
}
