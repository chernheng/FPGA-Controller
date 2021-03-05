#pragma once
#include <iostream>
#include <vector>
#include <ncurses.h>




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

  extern int player_start_x;
  extern int player_start_y;
}
enum direction {UP_DIR, DN_DIR, LF_DIR, RT_DIR};
enum {WALL_CLR=1, EMPTY_CLR, P1_CLR};

// screen utils
WINDOW *create_map_screen(int startx, int starty);
void print_map_to_screen(WINDOW * screen);
void print_char_to_screen(WINDOW * screen, int x_pos, int y_pos, char c);
void init_color_pairs();

// map utils
bool can_occupy(int x, int y);
char get_map_char(int x, int y);
void readmap(std::string mapfile);


class player {
  public:
  int x_coord;
  int y_coord;

  int old_x_coord;
  int old_y_coord;

  player();

  void move(direction dir, int steps); 
};

void update_player_pos(const player &p, WINDOW * screen);             