#include <iostream>
#include <fstream>
#include <string>
#include "map.h"
#include <unordered_map>


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

  // temporary map to hold teleport bindings
  std::unordered_map<char, std::pair<int, int>> temp_teleport_bindings;

  for (int i=0; i<mp::map_height; i++) {
    std::vector<char> row = {};
    for (int j=0; j<mp::map_width; j++) {
      char c = mapin.get();
      char d;
      // if char is '@' consume next character which is the key for teleport bindings
      // if char is 'S' add coordinates to compulsory stations
      switch (c)
      {
      case '@':
        d = mapin.get();
        if (temp_teleport_bindings.count(d)) {
          std::pair<int, int> that_teleport, this_teleport;
          that_teleport = temp_teleport_bindings[d];
          this_teleport = std::make_pair(j, i);
          mp::teleport_bindings[that_teleport] = this_teleport;
          mp::teleport_bindings[this_teleport] = that_teleport;
        } else {
          temp_teleport_bindings[d] = std::make_pair(j, i);
        }
        break;
      case 'S':
        mp::map_stations.emplace_back(j, i);
        break;
      
      }
      row.push_back(c);
    }
    char k;
    while(k=mapin.peek(), (k == '\n') || (k=='\r')) {
      mapin.get();
    }

    mp::map_array.push_back(row);
  }
   
  mapin.close();
  

}

char get_map_char(const int &x, const int &y) {
  if ((x>mp::map_width) || (y>mp::map_height) || (x<0) || (y<0)) {
    return '\0';
  }
  return mp::map_array[y][x];
}

bool can_occupy(const int &x, const int &y) {
  char c = get_map_char(x,y);
  switch (c)
  {
  case '#': return false;
  case '+': return false;
  case '\0': return false;

  
  }
  return true;
}

bool transparent_cell(const int &x, const int &y) {
  char c = get_map_char(x,y);
  switch (c)
  {
  case '#': return false;
  case '\0': return false;

  
  }
  return true;
}