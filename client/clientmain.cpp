#include "../include/screen.h"
#include "../include/player.h"
#include "../include/connection.h"
#include "../include/map.h"

using namespace std;

int main(int argc, char **argv) {

  // start ncurses
  start_ncurses();
  menu_screen(); // connect to server

  

  

  // read map
  readmap("maps/map1.txt");
  print_map_to_screen(map_screen);
  wrefresh(map_screen);

  getch();
  TaskStation t1;
  vector<int> x = t1.x_stn;
  vector<int> y = t1.y_stn;

  player p1;
  char ch;
  while((ch= getch()) != 'q') {
    print_station(t1,map_screen);
    vector<int>::iterator it_x = find(x.begin(),x.end(),p1.x_coord);
    vector<int>::iterator it_y = find(y.begin(),y.end(),p1.y_coord);
    if ((it_x - x.begin()) == (it_y - y.begin()) && it_x!=x.end() && it_y!=y.end()) {
      while((ch = getch())!='p'){
        //execute task
      }
    }
    switch (ch)
    {
    case 'w':
      p1.move(UP_DIR, 1);
      break;
    case 'a':
      p1.move(LF_DIR, 1);
      break;
    case 's':
      p1.move(DN_DIR, 1);
      break;
    case 'd':
      p1.move(RT_DIR, 1);
      break;
    }

    update_player_pos(p1, map_screen);
    wrefresh(map_screen);
  }


  getch();

  // end ncurses
  endwin();
}