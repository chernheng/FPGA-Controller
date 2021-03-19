#include <iostream>
#include <vector>
#include <map>

using namespace std;
// global vector to store the map
namespace mp {
  vector<vector<char>> map_array;
  int map_height=40;
  int map_width=200;

  int info_screen_height=5;
  int info_screen_width=200;

  int player_start_x=10;
  int player_start_y=5;

  int stations = 4;
  std::vector<std::pair<int, int>> map_stations;
  std::map<std::pair<int, int>, std::pair<int, int>> teleport_bindings;

}