
#include "map.h"

WINDOW *create_map_screen(int startx, int starty) {
  WINDOW* map_screen = newwin(map::map_height, map::map_width, startx, starty);
  box(map_screen, 0, 0);
  return map_screen;
}

void print_map_to_screen(WINDOW * screen) {

  for (int i=0; i<map::map_height; i++) {

    for (int j=0; j<map::map_width; j++) {
      char c = map::map_array[i][j];
      
      print_char_to_screen(screen, j, i, c);
    }


  }
}

void print_char_to_screen(WINDOW * screen, int x_pos, int y_pos, char c) {
  std::string ch = std::string(1, c);
  switch (c)
  {
  case '#':
    wattron(screen, COLOR_PAIR(WALL_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(WALL_CLR));
    break;
  
  case 'X':
    wattron(screen, COLOR_PAIR(P1_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(P1_CLR));
    break;
  
  default:
    wattron(screen, COLOR_PAIR(EMPTY_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(EMPTY_CLR));
    break;
  } // switch
}

void init_color_pairs() {
  init_pair(WALL_CLR, COLOR_YELLOW, COLOR_BLACK);
  init_pair(EMPTY_CLR, COLOR_YELLOW, COLOR_BLACK);
  init_pair(P1_CLR, COLOR_RED, COLOR_BLACK);
}

void update_player_pos(const player &p, WINDOW * screen) {
  // print map character where player was
  print_char_to_screen(screen, p.old_x_coord, p.old_y_coord, get_map_char(p.old_x_coord, p.old_y_coord));
  // print new position of player
  print_char_to_screen(screen, p.x_coord, p.y_coord, 'X');

}
