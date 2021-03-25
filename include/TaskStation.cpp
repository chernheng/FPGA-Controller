#include "map.h"

TaskStation::TaskStation(int n /* = 0 */) {
  srand((unsigned) time(0)+n);
  // add compulsory stations
  int i = 0;
  for (auto it : mp::map_stations) {
  
    x_stn.push_back(std::get<0>(it));
    y_stn.push_back(std::get<1>(it));
    task.push_back(std::get<2>(it));
    i++;
  }
  
  int rand_x;
  int rand_y;
  int rand_task;
  
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

void TaskStation::newTask(std::vector<int> x, std::vector<int> y, std::vector<int> t){
  x_stn = x;
  y_stn = y;
  task = t;
}
