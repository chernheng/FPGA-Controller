#include "map.h"
TaskStation::TaskStation(){
  srand((unsigned) time(0));
  int rand_x;
  int rand_y;
  int rand_task;
  int i = 0;
  while(i<map::stations){
    rand_x = (rand() % 150) +4;
    rand_y = (rand() % 35)+2;
    rand_task = rand() % 5;
    if (map::map_array[rand_y][rand_x]==' ') {
      x_stn.push_back(rand_x);
      y_stn.push_back(rand_y);
      task.push_back(rand_task);
    } else {
      continue;
    }
    i++;
  }
}
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
    if (map::map_array[rand_y][rand_x]==' ') {
      x_stn.push_back(rand_x);
      y_stn.push_back(rand_y);
      task.push_back(rand_task);
    } else {
      continue;
    }
    i++;
  }
}

void TaskStation::newTask(std::vector<int> x, std::vector<int> y){
  x_stn = x;
  y_stn = y;
}
