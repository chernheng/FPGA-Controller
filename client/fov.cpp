#include "map.h"
#include <vector>


/*

Recursive Shadowcasting Algorithm: http://www.roguebasin.com/index.php?title=FOV_using_recursive_shadowcasting

         Shared
             edge by
  Shared     1 & 2      Shared
  edge by\      |      /edge by
  1 & 8   \     |     / 2 & 3
           \1111|2222/
           8\111|222/3
           88\11|22/33
           888\1|2/333
  Shared   8888\|/3333  Shared
  edge by-------@-------edge by
  7 & 8    7777/|\4444  3 & 4
           777/6|5\444
           77/66|55\44
           7/666|555\4
           /6666|5555\
  Shared  /     |     \ Shared
  edge by/      |      \edge by
  6 & 7      Shared     4 & 5
             edge by 
             5 & 6

*/

using namespace std;

namespace map {
  vector<pair<int, int>> visible_cells;
  vector<pair<int, int>> prev_visible_cells;
}

inline int get_x_offset(const int &octant, const int &j) {
  switch (octant)
  {
  case 1: return j;
  case 6: return j;
    
  }
}

inline int get_y_offset(const int &octant, const int &i) {
  switch (octant)
  {
  case 1: return -i;
  case 6: return i;
  

  }
}

inline pair<float, float> calculate_slope(const int &x, const int &y, const int &player_x, const int &player_y, const int &octant) {
  float l_slope, r_slope;
  switch (octant)
  {
  case 1:
    l_slope = ((float) player_x - ((float) x - 0.5)) / ((float) player_y - ((float) y + 0.5));
    r_slope = ((float) player_x - ((float) x + 0.5)) / ((float) player_y - ((float) y - 0.5));
    return make_pair(l_slope, r_slope);
    break;
  
  case 6:
    l_slope = ((float) player_x - ((float) x - 0.5)) / abs((float) player_y - ((float) y - 0.5));
    r_slope = ((float) player_x - ((float) x + 0.5)) / abs((float) player_y - ((float) y + 0.5));
    return make_pair(l_slope, r_slope);
    break;
  
  default:
    break;
  }

}

void recur_shadowcast(float start_slope, float end_slope, int radius, int row, int player_x, int player_y, int octant) {

  if (start_slope < end_slope) {return;}

  float prev_r_slope = start_slope;
  //  go from left to right, then keep moving up
  for (int i=row; i<=radius; i++) {
    bool blocked = false; // indicates whether the previous cell was also blocked
    
    int search_y = player_y + get_y_offset(octant, i);
    // if y pos outside radius, break
    // if (abs(search_y - player_y) > radius) {break;}

    for (int j=-i; j<=0; j++) {
      int search_x = player_x + get_x_offset(octant, j);
      // if x pos outise radius, continue
      // if (abs(search_x - player_x) > radius) {continue;}

      // calculate slope
      pair<float, float> current_slopes = calculate_slope(search_x, search_y, player_x, player_y, octant);
      // if square is too much to the left, continue
      if (current_slopes.second > start_slope) {continue;}
      // if square is too much to the right, break
      if (current_slopes.first < end_slope) {break;}

      // make this square visible
      map::visible_cells.emplace_back(search_x, search_y);

      // check if this square is transparent
      bool cell_is_transparent = transparent_cell(search_x, search_y);
      // if previous cell was blocked, and this isnt, then change the current start slope
      if (blocked) {
        if (cell_is_transparent) {
          blocked = false;
          start_slope = prev_r_slope;
        } else {
          prev_r_slope = current_slopes.second;
        }
      } else if (!cell_is_transparent) {
        prev_r_slope = current_slopes.second;
        blocked = true;
        // need to recusrively call this again, start at leftmost square on next row

        recur_shadowcast(start_slope, current_slopes.first, radius, i+1, player_x, player_y, octant);

        

      }

    }
    // break if last square is blocked
    if (blocked) {break;}
  }

}


void mark_visible_cells(int player_x, int player_y) {
  int radius = 10;
  recur_shadowcast(1.0, 0.0, radius, 0, player_x, player_y, 1);
  recur_shadowcast(1.0, 0.0, radius, 0, player_x, player_y, 6);
}