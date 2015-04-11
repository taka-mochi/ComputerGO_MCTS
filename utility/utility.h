
#pragma once

#include "precomp.h"
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <cassert>

namespace Common {
  std::vector<std::string> split(const std::string &str, const char *delim_list);
  void split(const std::string &str, std::vector<std::string> &result, const char *delim_list);
  std::string changeExt(const std::string &str, const std::string &newext);
  std::string toUpper(const std::string &str);

  class StdlibRandom {
    double m_inverse_rndmax;
  public:
    StdlibRandom(unsigned int);
    StdlibRandom();
    double from_0_to_1() const;
    unsigned int operator()(unsigned int max) const;
    unsigned int operator()() const;
  };

  // Mersenne twister
  class MTRandom {
    std::mt19937 m_rnd;
    unsigned int m_min, m_max;
    //std::uniform_int_distribution<unsigned int> m_intDist;
    //std::uniform_real_distribution<double> m_doubleDist;
    //std::uniform_real_distribution<double> m_doubleDist;
  public:
    MTRandom(unsigned int seed);
    MTRandom();
    void init(unsigned int seed);
    double from_0_to_1() ;
    unsigned int operator()(unsigned int max) ;
    unsigned int operator()() ;

    static MTRandom &getInstance() {
      static MTRandom rnd;
      return rnd;
    }
  };

  // X or Shift
  /* class XorSRandom { */
  /*   XorSRandom(unsigned int seed); */
  /*   XorSRandom(); */
  /*   double from_0_to_1() const; */
  /*   unsigned int operator()(unsigned int max) const; */
  /*   unsigned int operator() const; */
  /* } */

  class ApproximatedMath {
    double *m_logTable;
    int m_logTableMax;
  public:
    explicit ApproximatedMath(int logMax = 100000);
    ~ApproximatedMath();
    static double sqrt(double v) {
      assert (v>=0);
      
      // by http://www.riken.jp/brict/Ijiri/study/fastsqrt.html
      /* double xHalf = 0.5 * v; */
      /* long long int  tmp   = 0x5FE6EB50C7B537AAl - ( *(long long int*)&v >> 1);//initial guess */
      /* double xRes  = * (double*)&tmp; */

      /* xRes *= ( 1.5 - ( xHalf * xRes * xRes ) ); */
      /* xRes *= ( 1.5 - ( xHalf * xRes * xRes ) );//コメントアウトを外すと精度が上がる */
      /* return xRes * v; */

      return std::sqrt(v);

      static const double epsilon = 0.001;

      double x1 = v,xn = v,xn_minus1 = v*2;
      while (xn_minus1 - xn > epsilon) {
      xn_minus1 = xn;
      xn = (xn + x1/xn)/2;
      }
      return xn;

    }

    static double log(double v) {
      return fmath::log(v);
    }

    double logint(int N) {
      assert (0<N);
      if (N>=m_logTableMax) {
        return logint(N/(m_logTableMax-1)) + m_logTable[m_logTableMax-1];
      } else {
        return m_logTable[N];
      }
    }
  };
}
