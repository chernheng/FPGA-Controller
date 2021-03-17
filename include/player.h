#pragma once


#include "map.h"

class player {
  public:
  int x_coord;
  int y_coord;
  std::string name;
  int tasks_left;
  bool is_used=false;

  int old_x_coord;
  int old_y_coord;

  player();

  void move(direction dir, int steps); 
  void newCoord(int x, int y);
};

 
