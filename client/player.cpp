#include "map.h"


player::player() {
  x_coord = old_x_coord = map::player_start_x;
  y_coord = old_y_coord = map::player_start_y;
}

void player::move(direction dir, int steps) {

    bool can_move=true;
    int y_coord_change=0;
    int x_coord_change=0;
    switch (dir)
    {
    case UP_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord, y_coord-i)) {
          can_move=false;
        } else {
          y_coord_change=-i;
        }
      }
      break;

    case RT_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord+i, y_coord)) {
          can_move=false;
        } else {
          x_coord_change=+i;
        }
      }
      break;
    case DN_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord, y_coord+i)) {
          can_move=false;
        } else {
          y_coord_change=i;
        }
      }
      break;
    case LF_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord-i, y_coord)) {
          can_move=false;
        } else {
          x_coord_change=-i;
        }
      }
      break;
    
    default:
      break;
    } // switch

    old_x_coord = x_coord;
    old_y_coord = y_coord;
    x_coord += x_coord_change;
    y_coord += y_coord_change;
  
}