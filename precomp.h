#include <string>
#include <vector>
//#include <memory>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

#include "common.h"
#include "Go/Rules.h"
#include "Go/Board.h"
#include "Go/Block.h"
#include "utility/PointMap.h"
#include "utility/SparseVector.h"
#include "utility/fmath.hpp"
#include "utility/utility.h"

#define SAFE_DECREF(p) {if(p) {(p)->decRef();}}
