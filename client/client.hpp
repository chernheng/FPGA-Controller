#pragma once
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/ioctl.h>
#include <net/if.h>	//ifreq
#include <string.h> 
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../include/screen.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/connection.h"
#include "../include/game_loop.h"
#define PORT 8080 

using namespace std;
//max count bytes from socket file descriptor
//change this accordingly, corresponding to max paacket size that will be sent
#define MAX_COUNT_BYTES    1024   

#define CONNECTION_REQ_PKT     1
#define CONNECTION_ACK_PKT     2
#define PLAYER_READY_PKT       3
#define GAME_START_PKT         4
#define GAME_INPUT_PKT         5
#define GAME_PROCESS_PKT       6
#define GAME_END_PKT           7

struct client_server_pkt{
    char client_mac_address[14]; //to differentiate btwn different clients on the same network
    uint8_t packet_type;
    char name[15];
    char ch;
    int ts_x[4];
    int ts_y[4];
    int task[4];
    int x_coord[2];
    int y_coord[2];
    //TODO: add on what is needed
};

int create_connection_socket(string server_ip);

int create_udp_connection_socket(string server_ip);

int get_mac_address(char *mac_address);

int connection_request_packet(client_server_pkt* buffer_send, char *mac_address);

int player_ready_packet(client_server_pkt* buffer_send, char *mac_address);

int game_input_packet(client_server_pkt* buffer_send, char *mac_address);

int process_packet(char* buffer_recv);

int process_acknowledgement(char *buffer_recv, int buffer_size);

char process_game_start(char* buffer_recv_game_start, int buffer_size);

void process_game(char* buffer_recv_game, int buffer_size, player *players);

void close_game();



