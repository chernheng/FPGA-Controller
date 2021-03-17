
#include "player.h"


namespace game {
  extern int player_index;
  extern player players[6];
  extern std::string player_names[6];
  extern int current_task;
  extern bool doing_task;
  extern std::vector<std::pair<int, int>> stations;
}


void game_loop();
