#include "precomp.h"

#include "main_functions.h"

#include <stdio.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sstream>

#include "AI/PureMCAI.h"
#include "AI/NormalPlayout.h"
#include "AI/SoftmaxPolicyPlayout.h"
#include "AI/UCTPlayer.h"
#include "AI/UCTBoundCalculator.h"
#include "ExecuteParameter.h"

#include "Gtp/GtpCore.h"

#include "Go/GoBook.h"

#include "generated_version.h"

using namespace std;
using namespace Common;

namespace Gtp {

  static Board *board = NULL;
  static shared_ptr<AI::PlayerBase> computer_player;
  static shared_ptr<Go::GoBook> opening_book;

  static AI::Parameters ai_params;

  typedef void (*CommandFunc)(const std::vector<string> &commands, Gtp::GtpCore &gtp);
  
  typedef std::map<string, CommandFunc> CommandMap;
  CommandMap command_map;

  void command_boardsize(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp("");

    if (commands.size() >= 2) {
      int size = atoi(commands[1].c_str());
      if (board && board->getSize() == size) {
        return;
      }
      if (size > 0) {
        stringstream ss;
        ss << "Recreating a board with size " << size << endl;
        gtp.writeConsoleLn(ss.str());
        delete board;
        board = new Board(size);
      }
    }
    return;
  }

  void command_genmove(const std::vector<string> &commands, Gtp::GtpCore &gtp) {

    if (commands.size()<2) {
      gtp.sendGtp("PASS");
      gtp.writeConsoleLn("Invalid Command!!");
      return;
    }
    Common::Color turn = (commands[1] == "w" || commands[1] == "W") ? WHITE : BLACK;

    Rules::setPutEyeAllowed(false);
    Common::Point p = POINT_NULL;
    if (opening_book) {
      vector<Point> moves = opening_book->getMovesInBook(*board);
      if (moves.size()>0) {
        p = moves[MTRandom::getInstance()()%moves.size()];
        gtp.writeConsoleLn("Used the opening book");
      }
    }
    if (p == POINT_NULL) {
      p = computer_player->selectBestMove(turn);
      if (computer_player->shouldResign()) {
        gtp.sendGtp("resign");
      }
      if (p == POINT_NULL) {
        gtp.sendGtp("nogame"); // this is not common GTP command
      }
    }

    if (p == PASS) {
      gtp.sendGtp("PASS");
    } else if (p != POINT_NULL) {
      std::pair<int,int> point = board->pointToXY(p);
      string send;
      if (point.first >= 8) {
        send += 'A' + point.first + 1; // ABCDEFGHJK... <= I is ignored
      } else {
        send += 'A' + point.first;
      }
      if (point.second < 9) {
        send += '1' + point.second;
      } else {
        send += '0' + (point.second+1)/10;
        send += '0' + (point.second+1)%10;
      }
      gtp.sendGtp(send);
    }
    board->put(p,turn);
  }

  void command_clear(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    if (board) {
      board->clear();
      // player is also created
      // default is UCTPlayer based on UCB_RAVE
      //AI::UCTPlayer<AI::NormalPlayout, AI::UCB_RAVE_Calculator> *player = new AI::UCTPlayer<AI::NormalPlayout, AI::UCB_RAVE_Calculator>(board, AI::NormalPlayout());
      // AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator> *player = new AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator>(board, AI::SoftmaxPolicyPlayout(AI::Parameters::getStandardFeatureWeightFile()));

      computer_player = ai_params.createPlayerAccordingToKind(board, 1);
      if (computer_player == NULL) {
        cerr << "Unsupported Player Kind !!" << endl;
        exit(-1);
      }
      if (ai_params.getUseOpeningBook()) {
        opening_book.reset(new Go::GoBook());
        if (!opening_book->readFromFile(ai_params.getOpeningBookFilename())) {
          opening_book = shared_ptr<Go::GoBook>();
        } else {
          cerr << "Use the opening book: " << ai_params.getOpeningBookFilename() << endl;
        }
      } else {
        opening_book = shared_ptr<Go::GoBook>();
      }

      // switch (ai_params.getPlayerKind()) {
      // case AI::Parameters::PLAYER_KIND::PURE_MC:
      //   cerr << "Unsupported Player Kind !!" << endl;
      //   exit(-1);
      //   break;
      // case AI::Parameters::PLAYER_KIND::UCT_PLAIN:
      //   {
      //     AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator> *player = new AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator>(board, AI::NormalPlayout(), ai_params);
      //     //player->setPlayoutLimit(ai_params.getPlayoutLimit());
      //     player->setDebugLevel(1);
      //     computer_player.reset(player);
      //   }
      //   break;
      // case AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES:
      //   {
      //     AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator> *player = new AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator>(board, AI::SoftmaxPolicyPlayout(ai_params.getStandardFeatureWeightFile(), MTRandom::getInstance()), ai_params);
      //     //player->setPlayoutLimit(ai_params.getPlayoutLimit());
      //     player->setDebugLevel(1);
      //     computer_player.reset(player);
      //   }
      //   break;

                            
      // default:
      //   cerr << "Unsupported Player Kind !!" << endl;
      //   exit(-1);
      // }

      // set to "recommended parameters"
      // This is may festival mode
      // this is a 2nd dim curv to fix (9, 10000) and (19, 1000)
      // int playout = (board->getSize() - 464) * (board->getSize() - 464) - 197025;
      // player->setPlayoutLimit(playout);
      stringstream ss;
      ss << "Playout Limit is Set to " << ai_params.getPlayoutLimit() << endl;
      cerr << ss.str() << endl << endl;
      gtp.writeConsoleLn(ss.str());
    }
    gtp.sendGtp("");
  }

  void command_play(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp("");
    if (commands.size() < 3) {
      gtp.writeConsoleLn("Invalid Command!!");
      return;
    }
    gtp.writeConsoleLn("opp played " + commands[1] + " " + commands[2]);

    Common::Color turn = (commands[1] == "w" || commands[1] == "W") ? WHITE : BLACK;
    string pos_str = commands[2];
    Common::Point p;
    stringstream ss;
    ss << "Put To ";
    if (pos_str == "PASS") {
      p = PASS;
      ss << "PASS";
    } else {
      int x = pos_str[0]-'A';
      if (x > 8) x--; // ABCDEFGHJK... <= I is ignored
      int y;
      if (pos_str.size() == 2) {
        // 9 or less
        y = pos_str[1]-'1';
      } else {
        y = (pos_str[1] - '0')*10 + (pos_str[2]-'0') - 1;
      }
      p = board->xyToPoint(x,y);
      ss << x << "," << y;
    }
    ss << " turn = " << turn;
    gtp.writeConsoleLn(ss.str());
    Rules::setPutEyeAllowed(true);
    if (Board::PUT_LEGAL != board->put(p, turn)) {
      gtp.writeConsoleLn("Invalid Move: " + pos_str);
    }
    Rules::setPutEyeAllowed(false);
  }

  void command_name(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp("itamochi_test");
  }

  void command_protocol_version(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp("2");
  }

  void command_version(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp(current_version);
  }

  void command_commandlist(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    CommandMap::iterator it;
    std::string ret = "";
    for (it = command_map.begin(); it!=command_map.end(); it++) {
      ret += it->first + "\n";
    }
  
    gtp.sendGtp(ret.substr(0,ret.size()-1));
  }

  void command_printBoard(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    if (board) {
      board->printInvertedBoardToErr();
    }
    gtp.sendGtp("");
  }

  void command_quit(const std::vector<string> &commands, Gtp::GtpCore &gtp) {
    gtp.sendGtp("");

    exit(0);
  }

  void gtp_main() {

    ai_params = AI::Parameters();
    if (ExecuteParameter::gtpParamFile == "" ||
        !ai_params.readParametersFromFile(ExecuteParameter::gtpParamFile)) {
      cerr << "Failed to read a parameter file for execution!! Program will finish." << endl;
      exit(-1);
    }

    vector<string> commands;
    Gtp::GtpCore &gtp = Gtp::GtpCore::getInstance();

    cerr << "Expand Threshold: " << ai_params.getThreshold_Expand() << endl;

    command_map["boardsize"] = command_boardsize;
    command_map["name"] = command_name;
    command_map["protocol_version"] = command_protocol_version;
    command_map["version"] = command_version;
    command_map["clear_board"] = command_clear;
    command_map["genmove"] = command_genmove;
    command_map["play"] = command_play;
    command_map["list_commands"] = command_commandlist;
    command_map["print_board"] = command_printBoard;
    command_map["showboard"] = command_printBoard;
    command_map["quit"] = command_quit;

    while (gtp.getCommand(commands)) {
      if (commands.size() == 0) {
        gtp.writeConsoleLn("empty command!");
        continue;
      }
      const string &command_head = commands[0];

      CommandMap::iterator it = command_map.find(command_head);
      if (it != command_map.end()) {
        it->second(commands, gtp);
      } else {
        gtp.sendGtp("");
        gtp.writeConsoleLn("unhandled command: " + command_head);
      }
    }
  }
}
