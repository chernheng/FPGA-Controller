#include "map.h"

TaskStation::TaskStation(int n /* = 0 */) {
  srand((unsigned) time(0)+n);
  // add compulsory stations
  for (auto it : mp::map_stations) {
    x_stn.push_back(it.first);
    y_stn.push_back(it.second);
    task.push_back(rand() % 5);
  }
  
  int rand_x;
  int rand_y;
  int rand_task;
  int i = 0;
  while(i<mp::stations){
    rand_x = (rand() % 150) +4;
    rand_y = (rand() % 35)+2;
    rand_task = rand() % 5;
    if (mp::map_array[rand_y][rand_x]==' ') {
      x_stn.push_back(rand_x);
      y_stn.push_back(rand_y);
      task.push_back(rand_task);
    } else {
      continue;
    }
    i++;
  }
}
