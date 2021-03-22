#include <string>
#include <vector>
#include "player.h"
#include "connection.h"
#include "screen.h"


namespace game {
  int player_index=0;
  std::string player_name;
  player players[6];
  int current_task=0;
  bool doing_task=false;
  std::vector<std::pair<int, int>> stations;
}
using namespace std;

void game_loop() {
    
    if (game::doing_task) {
      if (game::current_task==1) {
        // if just finished a task, remove station from map
        mp::map_array[game::players[game::player_index].old_y_coord][game::players[game::player_index].old_x_coord]=' ';
        game::doing_task = false;
      }
    }

    // if doing a task, display task on screen
    if (game::current_task > 1) {
      game::doing_task = true;
      // TODO Display task on screen
    }

    // update player vision
    update_player_pos(game::players[game::player_index], map_screen);

    // update other player positions
    update_other_player_pos();

    
    wrefresh(map_screen);
  
}

void copy_stations_to_map() {
  for (auto it : game::stations) {
    mp::map_array[it.second][it.first] = 'S';
  }
}
