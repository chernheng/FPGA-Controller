#include "map.h"


player::player() {
  x_coord = old_x_coord = map::player_start_x;
  y_coord = old_y_coord = map::player_start_y;
}

void player::move(direction dir, int steps) {


    int y_coord_change=0;
    int x_coord_change=0;
    switch (dir)
    {
    case UP_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord, y_coord-i)) {
          break;
        } else {
          y_coord_change=-i;
        }
      }
      break;

    case RT_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord+i, y_coord)) {
          break;
        } else {
          x_coord_change=+i;
        }
      }
      break;
    case DN_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord, y_coord+i)) {
          break;
        } else {
          y_coord_change=i;
        }
      }
      break;
    case LF_DIR:
      for(int i=1; i<=steps; i++) {
        if(!can_occupy(x_coord-i, y_coord)) {
          break;
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