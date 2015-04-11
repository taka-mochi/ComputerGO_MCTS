
#include "precomp.h"
#include "ExecuteParameter.h"

bool ExecuteParameter::isGtpMode = false;
std::string ExecuteParameter::resultFile = "result.dat";
int ExecuteParameter::gameCountForEvaluation = 1000;
int ExecuteParameter::playoutCountForMC = 10000/81;
int ExecuteParameter::playoutCountForUCT = 10000;
int ExecuteParameter::boardSize = 9;
int ExecuteParameter::drawLimitCount = 500;
unsigned int ExecuteParameter::randomSeed = 0;
std::string ExecuteParameter::gtpParamFile = "";
std::string ExecuteParameter::blackParamFile = "";
std::string ExecuteParameter::whiteParamFile = "";
bool ExecuteParameter::dontChangeTurn = false;
