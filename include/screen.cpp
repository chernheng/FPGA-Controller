#include "screen.h"
#include "map.h"
#include <form.h>
#include "connection.h"
#include "game_loop.h"
#include "player.h"


// global vars
WINDOW * map_screen;
WINDOW * info_screen; 


void start_ncurses() {
  // setlocale(LC_ALL, "");
  initscr();
  noecho(); // dont display what the user presses
  curs_set(0); // hide cursor
  int maxx, maxy;
  getmaxyx(stdscr, maxy, maxx);
  if ((maxx < mp::map_width) || (maxy < mp::map_height+mp::info_screen_height)) {
    std::string msg = "Your window is too small! (" + std::to_string(maxx) + " x " + std::to_string(maxy) + "). Needs to be at least (" + \
      std::to_string(mp::map_width) + " x " + std::to_string(mp::map_height+mp::info_screen_height) + "). Press any key to exit";
    printw(msg.c_str());
    getch();
    endwin();
    exit(1);
  }
  start_color();
  init_color_pairs();
  refresh();
  create_map_screen(maxx, maxy);
  create_info_screen(maxx, maxy);
  wrefresh(map_screen);
  wrefresh(info_screen);
  getch();
}

void create_map_screen(int maxx, int maxy) {
  map_screen = newwin(mp::map_height, mp::map_width, 0, (maxx-mp::map_width)/2);
  // box(map_screen, 0, 0);
}

void create_info_screen(int maxx, int maxy) {
  info_screen = newwin(mp::info_screen_height, mp::info_screen_width, mp::map_height, (maxx-mp::info_screen_width)/2);
  // box(info_screen, 0, 0);
}

void print_hidden_char(WINDOW * screen, int x, int y, char c='?') {

  if (c=='?') {
    c = get_map_char(x,y);
  }

  // print the borders
  if ((x==mp::map_width-1) || (x==0) || (y==mp::map_height-1) || (y==0)) {
    print_char_to_screen(screen, x, y, c);
  // print certain characters
  } else if((c=='L')) {
    print_char_to_screen(screen, x, y, c);
  // otherwise print Hidden char
  } else {
    print_char_to_screen(screen, x, y, 'H');
  }
}

void print_map_to_screen(WINDOW * screen) {

  for (int i=0; i<mp::map_height; i++) {

    for (int j=0; j<mp::map_width; j++) {
      print_hidden_char(screen, j, i);
    }


  }
}

void print_splash_screen(WINDOW * screen) {

  for (int i=0; i<mp::map_height; i++) {

    for (int j=0; j<mp::map_width; j++) {
      print_char_to_screen(screen, j, i, get_map_char(j, i));
    }


  }

  wrefresh(screen);
}

void print_char_to_screen(WINDOW * screen, int x_pos, int y_pos, char c) {
  std::string ch = std::string(1, c);
  switch (c)
  {
  case '#':
    wattron(screen, COLOR_PAIR(WALL_CLR));
    mvwprintw(screen, y_pos, x_pos,std::string(1, ' ').c_str());
    // mvwaddch(screen, y_pos, x_pos, ACS_CKBOARD);
    wattroff(screen, COLOR_PAIR(WALL_CLR));
    break;
  
  case 'X':
    wattron(screen, COLOR_PAIR(P1_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(P1_CLR));
    break;
  
  case '0':
    wattron(screen, COLOR_PAIR(P0_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P0_CLR));
    break;
  case '1':
    wattron(screen, COLOR_PAIR(P1_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P1_CLR));
    break;
  case '2':
    wattron(screen, COLOR_PAIR(P2_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P2_CLR));
    break;
  case '3':
    wattron(screen, COLOR_PAIR(P3_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P3_CLR));
    break;
  case '4':
    wattron(screen, COLOR_PAIR(P4_CLR));
    mvwprintw(screen, y_pos, x_pos,std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P4_CLR));
    break;
  case '5':
    wattron(screen, COLOR_PAIR(P5_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, 'X').c_str());
    wattroff(screen, COLOR_PAIR(P5_CLR));
    break;

  case 'S':
    wattron(screen, COLOR_PAIR(STNS_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(STNS_CLR));
    break;
  
  case 'L': // Lantern
    wattron(screen, COLOR_PAIR(LANTERN_CLR));
    mvwaddch(screen, y_pos, x_pos, ACS_LANTERN);
    wattroff(screen, COLOR_PAIR(LANTERN_CLR));
    break;

  case '+': // window
    wattron(screen, COLOR_PAIR(WINDOW_CLR));
    mvwaddch(screen, y_pos, x_pos, ACS_PLUS);
    wattroff(screen, COLOR_PAIR(WINDOW_CLR));
    break;
  
  case 'H': // Hidden
    wattron(screen, COLOR_PAIR(EMPTY_CLR));
    mvwprintw(screen, y_pos, x_pos, std::string(1, ' ').c_str());
    wattroff(screen, COLOR_PAIR(EMPTY_CLR));
    break;
  
  case ' ': // Floor
    wattron(screen, COLOR_PAIR(FLOOR_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(FLOOR_CLR));
    break;
  
  case '~': // Water
    wattron(screen, COLOR_PAIR(WATER_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(WATER_CLR));
    break;

  case '@': // Teleport
    wattron(screen, COLOR_PAIR(TELEPORT_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(TELEPORT_CLR));
    break;
  
  default:
    wattron(screen, COLOR_PAIR(EMPTY_CLR));
    mvwprintw(screen, y_pos, x_pos, ch.c_str());
    wattroff(screen, COLOR_PAIR(EMPTY_CLR));
    break;
  } // switch
}

void init_color_pairs() {
  init_pair(WALL_CLR, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(EMPTY_CLR, COLOR_YELLOW, COLOR_BLACK);
  init_pair(FLOOR_CLR, COLOR_YELLOW, COLOR_WHITE);
  init_pair(P0_CLR, COLOR_RED, COLOR_WHITE);
  init_pair(P1_CLR, COLOR_GREEN, COLOR_WHITE);
  init_pair(P2_CLR, COLOR_MAGENTA, COLOR_WHITE);
  init_pair(P3_CLR, COLOR_CYAN, COLOR_WHITE);
  init_pair(P4_CLR, COLOR_BLUE, COLOR_WHITE);
  init_pair(P5_CLR, COLOR_YELLOW, COLOR_WHITE);
  init_pair(STNS_CLR, COLOR_WHITE, COLOR_GREEN);
  init_pair(VISIBLE_CLR, COLOR_RED, COLOR_RED);
  init_pair(WINDOW_CLR, COLOR_BLACK, COLOR_CYAN);
  init_pair(LANTERN_CLR, COLOR_YELLOW, COLOR_RED);
  init_pair(TELEPORT_CLR, COLOR_YELLOW, COLOR_MAGENTA);
  init_pair(WATER_CLR, COLOR_BLUE, COLOR_WHITE);
  init_pair(INFO_TEXT_CLR, COLOR_BLACK, COLOR_WHITE);
  init_pair(INFO_RED_CLR, COLOR_RED, COLOR_WHITE);
  init_pair(INFO_ALERT_CLR, COLOR_WHITE, COLOR_RED);
}

void update_player_pos(const player &p, WINDOW * screen) {

  // check if player has landed on a lantern
  if(get_map_char(p.x_coord, p.y_coord)=='L') {
    mp::map_array[p.y_coord][p.x_coord] = ' '; // remove lantern
    mp::vision_radius+=2;
    info_panel_update_FOV_radius();
  }
  // calculate FOV
  mark_visible_cells(p.x_coord, p.y_coord);

  // print hidden character where player was
  print_hidden_char(screen, p.old_x_coord, p.old_y_coord);

  // remove previously visible positions
  for (auto it : mp::prev_visible_cells) {
    // print map character where player was
    print_hidden_char(screen, it.first, it.second);
  }
  mp::prev_visible_cells.clear();
  mp::prev_visible_cells = mp::visible_cells;

  // print visible positions
  for (auto it : mp::visible_cells) {
    print_char_to_screen(screen, it.first, it.second, get_map_char(it.first, it.second));
  }
  mp::visible_cells.clear();
  
  // print new position of player
  print_char_to_screen(screen, p.x_coord, p.y_coord, std::to_string(game::player_index)[0]);

}


void update_other_player_pos() {
  for (int i=0; i<6; i++) {
    if (i==game::player_index) {continue;}
    player p = game::players[i];
    if (p.is_used) {
      // show on screen if within FOV
      if (std::find(mp::prev_visible_cells.begin(), mp::prev_visible_cells.end(), std::make_pair(p.x_coord, p.y_coord)) \
          != mp::prev_visible_cells.end() ) {
            print_char_to_screen(map_screen, p.x_coord, p.y_coord, std::to_string(i)[0]);
          }
    }
  }

}

void print_station(const TaskStation &t, WINDOW * screen) {
  for(int i =0; i < mp::stations;i++){
    print_char_to_screen(screen, t.x_stn[i], t.y_stn[i], 'S');
  }
}

bool try_connect_server(FIELD * field[], WINDOW * form_win) {
  mvwprintw(form_win, 3, 5, "Attempting to connect...");
  wrefresh(form_win);
  std::string ip = std::string(field_buffer(field[0], 0));
  std::string name = std::string(field_buffer(field[1], 0));
  for (int i=0; i<ip.size(); i++) {
    // if not . or digit
    if ( ((ip[i] < '0') || (ip[i] > '9')) && (ip[i] != '.') ) {
      ip = ip.substr(0, i);
      break;
    }
  }
  // cerr << endl << endl;
  // for (int i=0; i<ip.size(); i++) {
  //   cerr <<(int) ip[i] << " ";
  // }
  game::player_name = name;
  if (create_connection_socket(ip)!=0){
        //socket is not set-up properly - TODO: what to respond with?
        return false;
  }
  if (create_udp_connection_socket(ip)!=0){
    return false;
        //some error in socket creation
  }
  return connect_to_server(ip, name);
}

void menu_screen() {
  FIELD *field[3];
  FORM *my_form;
  int ch;
  keypad(stdscr, TRUE);
  /* Initialize the fields */
  
  field[0] = new_field(1, 15, 1, 22, 0, 0);
  field[1] = new_field(1, 14, 2, 23, 0, 0);
  field[2] = NULL;
  /* Set field options */
  set_field_back(field[0], A_UNDERLINE); /* Print a line for the option */
  field_opts_off(field[0], O_AUTOSKIP); /* Don't go to next field when this */
  // field_opts_off(field[0], O_STATIC);
  /* Field is filled up */
  set_field_back(field[1], A_UNDERLINE);
  field_opts_off(field[1], O_AUTOSKIP);
  
  /* Create the form and post it */
  my_form = new_form(field);
  int maxx = getmaxx(stdscr);
  
  WINDOW* form_win = newwin(mp::info_screen_height, mp::info_screen_width, mp::map_height, (maxx-mp::info_screen_width)/2);
  set_form_win(my_form, form_win);
  set_form_sub(my_form, derwin(form_win, mp::info_screen_height, mp::info_screen_width, mp::map_height, (maxx-mp::info_screen_width)/2  ));
  box(form_win,0,0);
  post_form(my_form);
  wrefresh(form_win);
  mvwprintw(form_win, 0, 5, "Please enter IP address of server and player name. Press q to quit.");
  mvwprintw(form_win, 1, 10, "IP Address:");
  mvwprintw(form_win, 2, 10, "Player Name:");
  wrefresh(form_win);
  /* Loop through to get user requests */
  bool connected_succesfully = false;
  while((ch = getch()) != 'q') { 
    
    switch(ch)
    { 
      case KEY_DOWN:
        form_driver(my_form, REQ_VALIDATION);
        /* Go to next field */
        form_driver(my_form, REQ_NEXT_FIELD);
        /* Go to the end of the present buffer */
        /* Leaves nicely at the last character */
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_UP:
        form_driver(my_form, REQ_VALIDATION);
        /* Go to previous field */
        form_driver(my_form, REQ_PREV_FIELD);
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_ENTER:
        form_driver(my_form, REQ_VALIDATION);
        if (my_form->current == field[1]) {
          if (try_connect_server(field, form_win)) {
            connected_succesfully = true;
          } else {
            mvwprintw(form_win, 3, 5, "Connection failed       ");
          }
        }
        form_driver(my_form, REQ_NEXT_FIELD);
        /* Go to the end of the present buffer */
        /* Leaves nicely at the last character */
        form_driver(my_form, REQ_END_LINE);
        break;
      case 10: /* Also Enter Key*/
        form_driver(my_form, REQ_VALIDATION);
        if (my_form->current == field[1]) {
          if (try_connect_server(field, form_win)) {
            connected_succesfully = true;
          } else {
            mvwprintw(form_win, 3, 5, "Connection failed       ");
          }
        }
        form_driver(my_form, REQ_NEXT_FIELD);
        /* Go to the end of the present buffer */
        /* Leaves nicely at the last character */
        form_driver(my_form, REQ_END_LINE);
        break;
      case KEY_BACKSPACE:
        form_driver(my_form, REQ_DEL_PREV);
        break;
      default:
        /* If this is a normal character, it gets */
        /* Printed */
        form_driver(my_form, ch);
        break;
    }
    wrefresh(form_win);

    if (connected_succesfully) {
      break;
    }
  }
  if (!connected_succesfully) {
    endwin();
    exit(1);
  }
  /* Un post form and free the memory */
  unpost_form(my_form);
  free_form(my_form);
  free_field(field[0]);
  free_field(field[1]);
  wborder(form_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  wrefresh(form_win);
  delwin(form_win);
  
}


#define TASK_1_TEXT "                                                                "
#define TASK_2_TEXT "Flip up the switches with lights above them!"
#define TASK_3_TEXT "Press the button until the number displayed reaches 0!"
#define TASK_4_TEXT "Use the switches to convert the number shown into binary!"
#define TASK_5_TEXT "Press the button when the moving LED is caught in between the stationary ones!"
#define TASK_6_TEXT "Shake the device up and down!"
#define TASK_7_TEXT "Press any key on the keyboard to quit."


//?  0            13     20           33
//   Tasks Left : N      FOV Radius : N
//?  0               16
//   Complete Task : Task to do
//
//?  0                   20                  40                  60                  80                  100                 120
//   X : Player____Name  X : Player____Name  X : Player____Name  X : Player____Name  X : Player____Name  X : Player____Name
//

void info_panel_update_no_tasks() {
  int hud_y_coord = 0;
  int no_of_tasks_left = game::players[game::player_index].tasks_left;
  wattron(info_screen, COLOR_PAIR(INFO_RED_CLR));
  string txt = to_string(no_of_tasks_left) + " ";
  mvwprintw(info_screen, hud_y_coord, 13, txt.c_str());
  wattroff(info_screen, COLOR_PAIR(INFO_RED_CLR));

  wrefresh(info_screen);
}

void info_panel_update_FOV_radius() {
  int hud_y_coord = 0;

  wattron(info_screen, COLOR_PAIR(INFO_RED_CLR));
  string txt = to_string(mp::vision_radius) + " ";
  mvwprintw(info_screen, hud_y_coord, 33, txt.c_str());
  wattroff(info_screen, COLOR_PAIR(INFO_RED_CLR));

  wrefresh(info_screen);
}

void init_info_panel() {

  // Paint whole screen white
  for (int i=0; i<mp::info_screen_height; i++) {
    for (int j=0; j<mp::info_screen_width; j++) {
      print_char_to_screen(info_screen, j, i, ' ');
    }
  }

  // Print player characters
  int player_name_y_coord = mp::info_screen_height-1;
  for (int i=0; i<6; i++) {
    if (game::players[i].is_used) {
      print_char_to_screen(info_screen, i*20, player_name_y_coord, to_string(i)[0]);
      string txt = ": " + game::players[i].name;
      wattron(info_screen, COLOR_PAIR(INFO_TEXT_CLR));
      mvwprintw(info_screen, player_name_y_coord, (i*20)+2, txt.c_str());
      wattroff(info_screen, COLOR_PAIR(INFO_TEXT_CLR));
    }
  }

  // Print text for "Tasks Left", "FOV"
  int hud_y_coord = 0;
  wattron(info_screen, COLOR_PAIR(INFO_TEXT_CLR));
  mvwprintw(info_screen, hud_y_coord, 0, string("Tasks Left :").c_str());
  mvwprintw(info_screen, hud_y_coord, 20, string("FOV Radius :").c_str());
  wattroff(info_screen, COLOR_PAIR(INFO_TEXT_CLR));

  info_panel_update_FOV_radius();
  info_panel_update_no_tasks();
  wrefresh(info_screen);

}

void info_panel_update_task(int task_id) {
  int task_y_coord = 1;
  switch (task_id)
  {
  case 1:
    for (int j=0; j<mp::info_screen_width; j++) {
      print_char_to_screen(info_screen, j, task_y_coord, ' ');
    }
    break;

  case 2:
    wattron(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Complete Task : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_2_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    break;
  
  case 3:
    wattron(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Complete Task : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_3_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    break;
  
  case 4:
    wattron(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Complete Task : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_4_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    break;
  
  case 5:
    wattron(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Complete Task : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_5_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    break;
  
  case 6:
    wattron(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Complete Task : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_6_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_ALERT_CLR));
    break;
  
  case 7:
    wattron(info_screen, COLOR_PAIR(INFO_TEXT_CLR));
    mvwprintw(info_screen, task_y_coord, 0, string("Game Over : ").c_str());
    mvwprintw(info_screen, task_y_coord, 16, string(TASK_7_TEXT).c_str());
    wattroff(info_screen, COLOR_PAIR(INFO_TEXT_CLR));
    break;
  
  }

  wrefresh(info_screen);
}