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
  int vision_radius=2;
}

inline int get_x_offset(const int &octant, const int &j, const int &player_x, const int &player_y) {
  switch (octant)
  {
  case 1: return player_x+j;
  case 6: return player_x+j;
  case 2: return player_x-j;
  case 5: return player_x-j;

  case 3: return player_y+j;
  case 8: return player_y+j;
  case 7: return player_y-j;
  case 4: return player_y-j;
    
  }
}

inline int get_y_offset(const int &octant, const int &i, const int &player_x, const int &player_y) {
  switch (octant)
  {
  case 1: return player_y-i;
  case 6: return player_y+i;
  case 2: return player_y-i;
  case 5: return player_y+i;

  case 3: return player_x+i;
  case 8: return player_x-i;
  case 7: return player_x-i;
  case 4: return player_x+i;
  

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

  case 2:
    r_slope = abs((float) player_x - ((float) x - 0.5)) / ((float) player_y - ((float) y - 0.5));
    l_slope = abs((float) player_x - ((float) x + 0.5)) / ((float) player_y - ((float) y + 0.5));
    return make_pair(l_slope, r_slope);
    break;
  
  case 5:
    r_slope = abs((float) player_x - ((float) x - 0.5)) / abs((float) player_y - ((float) y + 0.5));
    l_slope = abs((float) player_x - ((float) x + 0.5)) / abs((float) player_y - ((float) y - 0.5));
    return make_pair(l_slope, r_slope);
    break;

  case 3:
    l_slope = ((float) player_y - ((float) y - 0.5)) / -((float) player_x - ((float) x - 0.5));
    r_slope = ((float) player_y - ((float) y + 0.5)) / -((float) player_x - ((float) x + 0.5));
    return make_pair(l_slope, r_slope);
    break;

  case 8:
    l_slope = ((float) player_y - ((float) y - 0.5)) / ((float) player_x - ((float) x + 0.5));
    r_slope = ((float) player_y - ((float) y + 0.5)) / ((float) player_x - ((float) x - 0.5));
    return make_pair(l_slope, r_slope);
    break;

  case 7:
    l_slope = -((float) player_y - ((float) y + 0.5)) / ((float) player_x - ((float) x + 0.5));
    r_slope = -((float) player_y - ((float) y - 0.5)) / ((float) player_x - ((float) x - 0.5));
    return make_pair(l_slope, r_slope);
    break;
  
  case 4:
    l_slope = -((float) player_y - ((float) y + 0.5)) / -((float) player_x - ((float) x - 0.5));
    r_slope = -((float) player_y - ((float) y - 0.5)) / -((float) player_x - ((float) x + 0.5));
    return make_pair(l_slope, r_slope);
    break;
  
  default:
    break;
  }

}

void recur_shadowcast(float start_slope, float end_slope, int radius, int row, int player_x, int player_y, int octant) {

  if (start_slope < end_slope) {return;}

  float prev_r_slope = start_slope;
  int search_x, search_y;
  //  go from left to right, then keep moving up
  for (int i=row; i<=radius; i++) {
    bool blocked = false; // indicates whether the previous cell was also blocked
    
    int search_y_tmp = get_y_offset(octant, i, player_x, player_y);

    for (int j=-i; j<=0; j++) {
      int search_x_tmp = get_x_offset(octant, j, player_x, player_y);

      // swap if this is vertical octants
      if ((octant==3) || (octant==8) || (octant==7) || (octant==4)) {
        search_y = search_x_tmp; search_x = search_y_tmp;
      } else {
        search_y = search_y_tmp; search_x = search_x_tmp;
      }


      if ((search_x<0) || (search_x>=map::map_width) || (search_y<0) || (search_y>=map::map_height)) {continue;}

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
  for (int i=1; i<=8; i++) {
    recur_shadowcast(1.0, 0.0, map::vision_radius, 0, player_x, player_y, i);
  }
}