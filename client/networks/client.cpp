#include "client.hpp"
#define PORT 8080 

using namespace std;

//initialising sock variable
int sock = 0;

//function to set up socket connection
//1. socket creation
//2. socket connection to server's address and port
int create_connection_socket(string server_ip)
{ 
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    return 0; 
} 

int connection_request_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection request
    client_server_pkt send_packet;
    send_packet.packet_type = CONNECTION_REQ_PKT;



    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int player_ready_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection ready signal
    client_server_pkt send_packet;
    send_packet.packet_type = PLAYER_READY_PKT;

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int game_input_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection ready signal
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_INPUT_PKT;

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int process_packet(char* buffer_recv){
    int pkt_type;
    client_server_pkt* pkt_received = (client_server_pkt*)buffer_recv;
    pkt_type = pkt_received->packet_type;
    printf("Packet received: %d\n", pkt_type);
    return pkt_type;
}

int process_acknowledgement(char *buffer_recv, int buffer_size){
    //
}

void process_game_start(char* buffer_recv_game_start, int buffer_size){
    //to process display and player's coord.
    
}

void process_game(char* buffer_recv_game, int buffer_size){

}



void close_game(){
    //
}

int main(int argc, char* argv[]){
    if (argc>2){
        printf("Too many arguments. Please provide only the server's IP address.\n ");
    }
    if (argc<2){
        printf("Please input the server's IP address.\n");
    }
    string server_ip = argv[1];
    //set-up of socket for connecting to server
    if (create_connection_socket(server_ip)!=0){
        //socket is not set-up properly - TODO: what to respond with?
    }
    //set-up connection request packet fields
    char buffer_send;
    int buffer_send_size = connection_request_packet((client_server_pkt*)&buffer_send);

    
    //send connection request to server
    //TODO: check that sock != 0 - ensuring that socket has been set-up properly
    send(sock , (char*)&buffer_send , buffer_send_size , 0 ); 
    printf("Connection request sent\n");

    //receive acknowledgement packet from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    char buffer_recv[MAX_COUNT_BYTES] = {0}; 
    int valread = read( sock , buffer_recv, MAX_COUNT_BYTES); 

    //TODO: process the packet sent by server - whether it says server is busy or connected to a game
    //if connected client to a new game: sends over display for player to click a button to indicate you are ready
    //if server is busy - receives that message
    int pkt_type = process_packet(buffer_recv);
    if (pkt_type==CONNECTION_ACK_PKT){
        if (process_acknowledgement(buffer_recv, MAX_COUNT_BYTES)!=1){
            //TODO: doesnt continue on to send ready signal; display msg to say press some button to try connecting again
        }
    }else{
        printf("Detected wrong packet response from server.\n");
    }
    

    //fpga button signal sent to server
    //model it now as sending signal from client?
    //TODO: timeout fn for client to press button - msg to say response not detected, disconnecting
    //when button is pressed:
    char buffer_send_ready;
    int buffer_send_ready_size = player_ready_packet((client_server_pkt*)&buffer_send_ready);
    //send connection ready to server
    send(sock , (char *)&buffer_send_ready , buffer_send_ready_size , 0 ); 
    printf("Player ready sent\n");

    //receive game start coord. and game display from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    char buffer_recv_game_start[MAX_COUNT_BYTES] = {0}; 
    int valread_game_start = read( sock , buffer_recv_game_start, MAX_COUNT_BYTES); 

    pkt_type = process_packet(buffer_recv_game_start);
    if (pkt_type==GAME_START_PKT){
        process_game_start(buffer_recv_game_start, MAX_COUNT_BYTES);
    }else{
        printf("Detected wrong packet response from server.\n");
    }
    //to test if other clients can connect while players are in game
    usleep(20*1000000);

    //receive game ongoing coord. and game display from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    char buffer_recv_game[MAX_COUNT_BYTES] = {0}; 
    int valread_game;
    char buffer_send_input;
    int buffer_send_input_size;
    // = read( sock , buffer_recv_game, MAX_COUNT_BYTES); 
    // process_game(buffer_recv_game, MAX_COUNT_BYTES);

    //TODO: while game is ongoing, keep receiving packets - while loop; while the msg received is not the end game packet
    // char* end_game_packet;
    // end_game_packet_struct(end_game_packet);

    while(pkt_type!=GAME_END_PKT){
        //buffer_recv_game = 0; 
        //send game input to server
        buffer_send_input_size = game_input_packet((client_server_pkt*)&buffer_send_input);
        send(sock , (char *)&buffer_send_input , buffer_send_input_size , 0 ); 
        
        //receive client's coord. and other players' coord.
        valread_game = read( sock , buffer_recv_game, MAX_COUNT_BYTES);
        pkt_type = process_packet(buffer_recv_game);
        if (pkt_type==GAME_PROCESS_PKT){
            process_game(buffer_recv_game, MAX_COUNT_BYTES); 
        }else if(pkt_type==GAME_END_PKT){
            //end game?
        }else{
            printf("Detected wrong packet response from server. Packet type: %d\n", pkt_type);
            exit(1);
        }
        //to clear prev mem of packet received
        memset(buffer_recv_game,'0',MAX_COUNT_BYTES);
    }    
    exit(1);
    //game ended - close display
    //close_game();


}
    
     

