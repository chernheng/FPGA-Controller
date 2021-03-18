#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <map>



/**
 * (0, 0)   ...   (200, 0)
 *    :
 *    :
 * (0, 50)  ...   (200, 50)
 */


// global vector to store the map
namespace mp {
  extern std::vector<std::vector<char>> map_array;
  extern std::vector<std::pair<int, int>> visible_cells;
  extern std::vector<std::pair<int, int>> prev_visible_cells;
  extern int vision_radius;
  extern int map_height;
  extern int map_width;

  extern int info_screen_height;
  extern int info_screen_width;

  extern int player_start_x;
  extern int player_start_y;
  extern int stations;
  extern std::vector<std::pair<int, int>> map_stations;

  extern std::map<std::pair<int, int>, std::pair<int, int>> teleport_bindings;

}
enum direction {UP_DIR, DN_DIR, LF_DIR, RT_DIR};
enum {WALL_CLR=1, EMPTY_CLR, STNS_CLR, VISIBLE_CLR, FLOOR_CLR, WINDOW_CLR, \
      LANTERN_CLR, P0_CLR, P1_CLR, P2_CLR, P3_CLR, P4_CLR, P5_CLR,         \
      TELEPORT_CLR, WATER_CLR, };



// map utils
bool can_occupy(const int &x, const int &y);
bool transparent_cell(const int &x, const int &y);
char get_map_char(const int &x, const int &y);
void readmap(std::string mapfile);



 

class TaskStation {
  public:
  std::vector<int> task;
  std::vector<int> x_stn;
  std::vector<int> y_stn;
  TaskStation(int n=0);
};
