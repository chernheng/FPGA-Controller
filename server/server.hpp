#pragma once
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <netinet/in.h> 
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

#define PORT   8080 

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
#define REJECT_PKT             8

vector<sockaddr_in> clients_connected;
vector<sockaddr_in> clients_in_game; 
//unordered_map<string, string>client_connection;

struct clients_info{
    vector<int> socket_descriptor;
    vector<struct sockaddr_in> address;
    vector<socklen_t> address_len;
    vector<char*> buffer_conn_req;
    vector<char*> buffer_send_ack;
    vector<int> buffer_send_ack_size;
    vector<char*> buffer_ready;
    vector<char*> buffer_send_game_start;
    vector<int> buffer_send_game_start_size;
    vector<char*> buffer_usr_input;
    vector<char*> buffer_send_reject;
    vector<int> buffer_send_reject_size;
    vector<char*> buffer_send_game;
    vector<int> buffer_send_game_size;
};

// struct client_server_pkt{
//     char client_mac_address[14];
//     uint8_t packet_type;
//     char name[15];
//     char ch;
//     int ts_x[4];
//     int ts_y[4];
//     int task[4];
//     int x_coord[2];
//     int y_coord[2];
//     //TODO: add on what is needed

// };

int create_connection_socket();

int create_udp_connection_socket();

int AcceptClient(int server_socket);

int acknowledgement_packet(client_server_pkt* buffer_send);

int game_start_packet(client_server_pkt* buffer_send, TaskStation ts);

int game_process_packet(client_server_pkt* buffer_send, player *players,int id, bool move);

int reject_packet(client_server_pkt* buffer_send);


int process_packet(char* buffer_recv);

int process_connection_request(char* buffer_conn_req, int id);

int process_ready(char* buffer_recv, int buffer_size);

int process_usr_input(char* buffer_usr_input, int buffer_size);

void process_game(char* buffer_recv_game, int buffer_size);

void close_game();
