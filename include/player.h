#pragma once


#include "map.h"

class player {
  public:
  int x_coord;
  int y_coord;

  int old_x_coord;
  int old_y_coord;

  player();

  void move(direction dir, int steps); 
};

 
