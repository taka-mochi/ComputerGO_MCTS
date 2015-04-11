
class ExecuteParameter {
public:
  static bool isGtpMode;
  static std::string resultFile;
  static int gameCountForEvaluation;
  static int playoutCountForMC;
  static int playoutCountForUCT;
  static int boardSize;
  static int drawLimitCount;
  static unsigned int randomSeed;
  static std::string gtpParamFile;
  static std::string blackParamFile;
  static std::string whiteParamFile;
  static bool dontChangeTurn;
};
