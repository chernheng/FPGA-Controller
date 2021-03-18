#include "../include/screen.h"
#include "../include/player.h"
#include "../include/connection.h"
#include "../include/map.h"
#include "../include/game_loop.h"

using namespace std;

int main(int argc, char **argv) {

  // start ncurses
  start_ncurses();
  menu_screen(); // connect to server

  // read map
  readmap("maps/map1.txt");
  print_map_to_screen(map_screen);
  wrefresh(map_screen);

  // info panel
  // TODO

  copy_stations_to_map();

  // game loop
  game_loop();


  getch();
  TaskStation t1 = TaskStation();
  vector<int> x = t1.x_stn;
  vector<int> y = t1.y_stn;

  player p1;
  char ch;
  


  getch();

  // end ncurses
  endwin();
}