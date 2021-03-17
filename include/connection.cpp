#include <string>
#include "map.h"
#include "game_loop.h"
#include <chrono>
#include <thread>

using namespace std;

bool connect_to_server(string ip, string player_name) {
  char buffer[11];
  // TODO copy player name to buffer
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); // delay for testing
  // code for connecting to server goes here

  // wait for reply, and get player ID
  game::player_index=0;

  // player names, if supported
  game::players[0].name = "";
  game::players[0].is_used = true;

  /**
   * Record down player game station coordinates
   * 
   * game::stations.emplace_back(2, 3);
   * ...
   */

  // return true if ok, else return false
  return true;
}

void check_game_packets() {
  // check if any packet has arrived

  // update vars

  game::players[0].x_coord = 0;
  game::players[0].y_coord = 0;
  game::players[1].x_coord = 0;
  game::players[1].y_coord = 0;
  /* ... and so on */

  // tasks remaining, if supported
  game::players[0].tasks_left = 0;
  

  // update the task_no
  game::current_task = 0;
}
