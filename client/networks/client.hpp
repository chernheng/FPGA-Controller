#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <cstdint>
#include <string.h> 
#include <string>
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
    uint8_t packet_type;
    //TODO: add on what is needed

};

int create_connection_socket(string server_ip);

int connection_request_packet(client_server_pkt* buffer_send);

int player_ready_packet(client_server_pkt* buffer_send);

int game_input_packet(client_server_pkt* buffer_send);

int process_packet(char* buffer_recv);

int process_acknowledgement(char *buffer_recv, int buffer_size);

void process_game_start(char* buffer_recv_game_start, int buffer_size);

void process_game(char* buffer_recv_game, int buffer_size);

void close_game();



