#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>



/**
 * (0, 0)   ...   (200, 0)
 *    :
 *    :
 * (0, 50)  ...   (200, 50)
 */

using namespace std;
// global vector to store the map
namespace map {
  extern vector<vector<char>> map_array;
  extern int map_height;
  extern int map_width;

  extern int info_screen_height;
  extern int info_screen_width;

  extern int player_start_x;
  extern int player_start_y;
  extern int stations;

  extern int playerID;
}
enum direction {UP_DIR, DN_DIR, LF_DIR, RT_DIR};
enum {WALL_CLR=1, EMPTY_CLR, P1_CLR, STNS_CLR};



// map utils
bool can_occupy(int x, int y);
char get_map_char(int x, int y);
void readmap(std::string mapfile);



 

class TaskStation {
  public:
  vector<int> task;
  vector<int> x_stn;
  vector<int> y_stn;

  TaskStation();
};
