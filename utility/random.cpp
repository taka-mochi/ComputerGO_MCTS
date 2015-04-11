
#include "precomp.h"

#include "utility.h"
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <tuple>
#include <random>
#include <vector>
#include <map>
#include <array>

using namespace std;

namespace Common {
  StdlibRandom::StdlibRandom(unsigned int seed)
    : m_inverse_rndmax(1.0/RAND_MAX)
  {
    srand(seed);
  }
  StdlibRandom::StdlibRandom()
    : m_inverse_rndmax(1.0/RAND_MAX)
  {
  }

  unsigned int StdlibRandom::operator()(unsigned int max) const {
    unsigned int r = rand()%(max);
    return r;
  }
  unsigned int StdlibRandom::operator()() const {
    return rand();
  }

  double StdlibRandom::from_0_to_1() const {
    return static_cast<double>(rand())*m_inverse_rndmax;
  }

  // Mersenne twister
  MTRandom::MTRandom(unsigned int seed)
    : m_rnd()
      //, m_intDist(0, 0xffffffff)
      //, m_doubleDist(0.0, 1.0)
  {
    init(seed);
  }
  MTRandom::MTRandom()
    : m_rnd()
      //, m_intDist(0, 0xffffffff)
      //, m_doubleDist(0.0, 1.0)
  {
    init(0);
  }

  void MTRandom::init(unsigned int seed) {
    // std::random_device rnd;
    // std::vector<std::uint32_t> v(seed);
    // std::generate(v.begin(), v.end(), std::ref(rnd));
    // std::seed_seq rnd_seed(v.begin(), v.end());
    
    m_rnd = std::mt19937(seed);
    srand(seed);

    // m_min = 0; //m_rnd.min();
    // m_max = RAND_MAX; //m_rnd.max();
    m_min = m_rnd.min();
    m_max = m_rnd.max();
    //srand(seed);
  }

  double MTRandom::from_0_to_1() {
    return (*this)()/static_cast<double>(m_max-m_min);
  }

  unsigned int MTRandom::operator()(unsigned int max) {
    return (*this)()%max;
  }

  unsigned int MTRandom::operator()() {
    return m_rnd() - m_min;
    //return rand() - m_min;
  }

  // X or Shift
  // XorSRandom::XorSRandom(unsigned int seed) {
  // }
  // XorSRandom::XorSRandom() {
  // }
  // double XorSRandom::from_0_to_1() const {
  // }
  // unsigned int XorSRandom::operator()(unsigned int max) const {
  // }
  // unsigned int XorSRandom::operator() const {
  // }
}
