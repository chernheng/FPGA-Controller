
#include "map.h"


int main(int argc, char **argv) {

  // start ncurses
  initscr();
  noecho(); // dont display what the user presses
  curs_set(0); // hide cursor
  
  start_color();
  init_color_pairs();
  refresh();

  WINDOW * map_screen = create_map_screen(1, 1);
  wrefresh(map_screen);

  // read map
  readmap("maps/map1.txt");
  print_map_to_screen(map_screen);
  wrefresh(map_screen);
  getch();

  player p1;
  char ch;
  while( (ch= getch()) != 'q') {
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