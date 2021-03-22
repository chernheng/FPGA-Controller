
#include "player.h"


namespace game {
  extern int player_index;
  extern std::string player_name;
  extern player players[6];
  extern int current_task;
  extern bool doing_task;
  extern std::vector<std::pair<int, int>> stations;
}


void game_loop();
void copy_stations_to_map();
