#include "map.h"
TaskStation::TaskStation(){}
TaskStation::TaskStation(int n) {
  srand((unsigned) time(0)+n);
  int rand_x;
  int rand_y;
  int rand_task;
  int i = 0;
  while(i<map::stations){
    rand_x = (rand() % 150) +4;
    rand_y = (rand() % 35)+2;
    rand_task = rand() % 5;
    if (can_occupy(rand_x, rand_y)) {
      x_stn.push_back(rand_x);
      y_stn.push_back(rand_y);
      task.push_back(rand_task);
    } else {
      continue;
    }
    i++;
  }
}
