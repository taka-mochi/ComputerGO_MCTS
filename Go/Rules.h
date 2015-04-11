
#pragma once

namespace Go {
  class Rules {
    static bool s_isEyeAllowed;
    static double s_Komi;

    Rules(){}
    Rules(const Rules &){}
    Rules &operator =(const Rules &) {return *this;}

  public:
    static bool isPutEyeAllowed() {return s_isEyeAllowed;}
    static void setPutEyeAllowed(bool allow) {s_isEyeAllowed = allow;}
    static double getKomi() {return s_Komi;}
    static void setKomi(double komi) {if (komi<0) return; s_Komi = komi;}
  };
}
