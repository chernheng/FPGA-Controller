#pragma once
#include <vector>
#include "player.h"
#include "map.h"
#include "../client/client.hpp"
#include <ncurses.h>

extern WINDOW * map_screen; 
void start_ncurses();
void menu_screen();

// screen utils
void create_map_screen(int startx, int starty);
void create_info_screen(int startx, int starty);
void print_map_to_screen(WINDOW * screen);
void print_char_to_screen(WINDOW * screen, int x_pos, int y_pos, char c);
void init_color_pairs();


void update_player_pos(const player &p, WINDOW * screen);   
void update_other_player_pos();
void print_station(const TaskStation &t, WINDOW * screen);

void mark_visible_cells(int player_x, int player_y);