#include <string>
#include "map.h"
#include <chrono>
#include <thread>

using namespace std;

bool connect_to_server(string ip, string player_name) {
  std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // delay for testing
  // code for connecting to server goes here

  // wait for reply, and get player ID
  map::playerID=0;

  // return true if ok, else return false
  return true;
}
