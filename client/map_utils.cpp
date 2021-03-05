#include <iostream>
#include <fstream>
#include <string>
#include "map.h"


void readmap(std::string mapfile) {
  std::ifstream mapin;
  mapin.open(mapfile);
  if (!mapin.is_open()) {
    std::cerr << "Cannot open map file. Exiting" << std::endl;
    exit(1);
  }
  if (mapin.peek()=='\"') {
    mapin.get(); // throw away the " character generated
  }

  for (int i=0; i<map::map_height; i++) {
    std::vector<char> row = {};
    for (int j=0; j<map::map_width; j++) {
      char c = mapin.get();
      row.push_back(c);
    }
    char k;
    while(k=mapin.peek(), (k == '\n') || (k=='\r')) {
      mapin.get();
    }

    map::map_array.push_back(row);
  }
   
  mapin.close();
  

}

char get_map_char(int x, int y) {
  if ((x>map::map_width) || (y>map::map_height) || (x<0) || (y<0)) {
    return '\0';
  }
  return map::map_array[y][x];
}

bool can_occupy(int x, int y) {
  char c = get_map_char(x,y);
  switch (c)
  {
  case '#': return false;
  case '\0': return false;

  
  }
  return true;
}