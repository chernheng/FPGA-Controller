#include "server.hpp"


using namespace std;

//initialising sock variable
int server_fd=0;
int new_socket=0;
struct sockaddr_in address; 
//int addrlen = sizeof(address); 
clients_info clients;


//function to set up socket connection
int create_connection_socket()
{ 
    // int server_fd, new_socket, valread; 
    //struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    // char buffer[1024] = {0}; 
    // char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    printf("socket object initialised\n");
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    printf("Attached socket to port\n"); 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("Succesfully bind socket to port.\n");
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    printf("Listening on port\n");
    //when a connection arrives, open a new socket to communicate with it
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    printf("Finished setting up socket.\n");
    return 0; 
}

int AcceptClient(int server_socket, int timeout)
{
   int iResult;
   struct timeval tv;
   fd_set rfds;
   FD_ZERO(&rfds);
   FD_SET(server_socket, &rfds);

   tv.tv_sec = (long)timeout;
   tv.tv_usec = 0;

   iResult = select(server_socket+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);
   if(iResult > 0)
   {
      return accept(server_socket, NULL, NULL);
   }
   else if (iResult == 0){
       printf("No additional clients connecting\n");
   }else{
        printf("Socket error in accepting new connection\n");
   }
   return 0;
}

int acknowledgement_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to client to acknowledge that client is connect/no connected to a game
    client_server_pkt send_packet;
    send_packet.packet_type = CONNECTION_ACK_PKT;

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}


int game_start_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_START_PKT;

    //to set-up packet_fields

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

//for testing
int game_loop = 0;

int game_process_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_PROCESS_PKT;
	
    
    //to set-up packet_fields
    //testing
    game_loop++;
    if (game_loop>=5){
	    send_packet.packet_type = GAME_END_PKT;
    }

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int reject_packet(client_server_pkt* buffer_send){
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = REJECT_PKT;

    //to set-up packet_fields

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

int process_connection_request(char* buffer_conn_req){
    //to check a global map that stores current clients? - if size>=2, then will send ack, else will wait for another player to connect
    //if after timeout fn and no other player connects - error msg - dont ack player
    //timeout fn to be set in either main fn or ack packet
    //clients_connected.push_back(address);
    if(clients.socket_descriptor.size()<2){
        printf("Waiting for other players to connect...\n");
        return 0;
    }
    printf("Minimum number of players reached\n");
    return 1;
}

int process_ready(char* buffer_recv, int buffer_size){
    //process the packet sent by client - whether client has indicated they are ready before timeout fn
    //else, remove them from the global vector of connected clients

    //include into vectors of clients that are currently in game
    //push back address of client - to get the address from param?
    clients_in_game.push_back(address);
    return 0;
}


int process_usr_input(char* buffer_usr_input, int buffer_size){
    return 0;
}


void process_game_start(char* buffer_recv_game_start, int buffer_size){
    //to process display and player's coord.
}

void process_game(char* buffer_recv_game, int buffer_size){

}

void close_game(){
    //
}

int main(){
    printf("setting up socket\n");
    
    if (create_connection_socket()!=0){
        //socket is not set-up properly - TODO: what to respond with?
    }
    
    // clients_info clients;
    //initialising buffer for read function to store received bytes into (able to read/store a max of max_count_bytes)
    char empty_buffer[MAX_COUNT_BYTES] = {0};
    //storing client 1's socket info
    clients.socket_descriptor.push_back(new_socket);
    clients.buffer_conn_req.push_back(empty_buffer);
    
    //error returns -1
    if((read(clients.socket_descriptor[0], clients.buffer_conn_req[0], MAX_COUNT_BYTES))<0){
        printf("Error in reading received bytes\n");
    }

    //check if received packet is of connection req type
    int pkt_type;
    pkt_type = process_packet(clients.buffer_conn_req[0]);
        if (pkt_type!=CONNECTION_REQ_PKT){
            printf("Detected wrong packet response from client.\n");
        }

    //when there is not enough players - wait for another client's connection
    if (process_connection_request(clients.buffer_conn_req[0])!=1){
        int timeout=10;
        printf("Attempting to detect second client\n");

        clients.socket_descriptor.push_back(AcceptClient(server_fd, timeout));
	    printf("Managed to accept second client\n");

        clients.buffer_conn_req.push_back(empty_buffer);
        if((read(clients.socket_descriptor[1], clients.buffer_conn_req[1], MAX_COUNT_BYTES))<0){
            printf("Error in reading received bytes\n");
        }
        
        //TODO: to continue accepting until timeout or max players in game
    }
    printf("read into second buffer\n");
    
    //for each client connection:
    for (int i=1; i<clients.socket_descriptor.size(); i++){
        printf("Processing client number: %d\n", i);
        pkt_type = process_packet(clients.buffer_conn_req[i]);
        if (pkt_type==CONNECTION_REQ_PKT){
            process_connection_request(clients.buffer_conn_req[i]);
        }else{
            printf("Detected wrong packet response from client.\n");
        }
    }
    
    char buffer_send_ack;
    int buffer_send_ack_size;
    char buffer_send_game_start;
    int buffer_send_game_start_size;
    char buffer_send_game;
    int buffer_send_game_size;
    

    for (int i=0; i<clients.socket_descriptor.size(); i++){
        printf("Processing client number: %d\n", i);
        clients.buffer_send_ack.push_back(&buffer_send_ack);
        clients.buffer_send_ack_size.push_back(buffer_send_ack_size);

        clients.buffer_send_ack_size[i] = acknowledgement_packet((client_server_pkt*)clients.buffer_send_ack[i]);

        //send ack packet to client
        send(clients.socket_descriptor[i] , clients.buffer_send_ack[i] , clients.buffer_send_ack_size[i] , 0 ); 
        printf("Acknowledge packet sent\n");
    }
    for (int i=0; i<clients.socket_descriptor.size(); i++){
        //receive play ready packet from client
        clients.buffer_ready.push_back(empty_buffer);
        if((read(clients.socket_descriptor[i], clients.buffer_ready[i], MAX_COUNT_BYTES))<0){
            printf("Error in reading received bytes\n");
        }

        //TODO: process the packet sent by client - whether client has indicated they are ready before timeout fn
        //else, remove them from the global vector of connected clients
        pkt_type = process_packet(clients.buffer_ready[i]);
        if (pkt_type==PLAYER_READY_PKT){
            if (process_ready(clients.buffer_ready[i], 1024)!=1){
                //if client times-out - goes back to start of loop? (maybe case structure)
            }
        }else{
            printf("Detected wrong packet response from client.\n");
        }
    }
    int sent_pkt_type;
    for (int i=0; i<clients.socket_descriptor.size(); i++){
        //send game start packet
        //set-up connection game-start coord. and game start display
        clients.buffer_send_game_start.push_back(&buffer_send_game_start);
        clients.buffer_send_game_start_size.push_back(buffer_send_game_start_size);
        clients.buffer_send_game_start_size[i] = game_start_packet((client_server_pkt*)clients.buffer_send_game_start[i]);
        //send game start coord and display to client
        send(clients.socket_descriptor[i], clients.buffer_send_game_start[i] , clients.buffer_send_game_start_size[i] , 0 ); 
        printf("Game start coordinates and game display sent\n");    
        sent_pkt_type = process_packet(clients.buffer_send_game_start[i]);

    }


        while(sent_pkt_type!=GAME_END_PKT){
            for (int i=0; i<clients.socket_descriptor.size(); i++){
                clients.buffer_usr_input.push_back(empty_buffer);
                if((read( clients.socket_descriptor[i] , clients.buffer_usr_input[i], MAX_COUNT_BYTES))<0){
                    printf("Error in reading received bytes\n");
                }

            //check whether received packet is from clients that are currently in the game
            //TODO: client_in_game struct - unordered map? <client_address, player number?>
            // int is_in_game = 0;
            // for(int i=0; i<clients_in_game.size(); i++){
            //     if(clients_in_game[i].sin_addr.s_addr==address.sin_addr.s_addr){
            //         is_in_game = 1;
            //     }
            // }
            // if (is_in_game==0){
            //     //client is not in game - send reject msg back to client
            //     char* buffer_send_reject;
            //     int buffer_send_reject_size;
            //     buffer_send_reject_size = reject_packet((client_server_pkt*)&buffer_send_reject);
            //     send(new_socket, (char*)&buffer_send_reject , buffer_send_reject_size , 0 ); 

            // }
        
        
            //TODO: process the packet sent by client - whether client has indicated they are ready before timeout fn
            //else, remove them from the global vector of connected clients
                if (process_usr_input(clients.buffer_usr_input[i], 1024)!=1){
                    //if client times-out - goes back to start of loop? (maybe case structure)
                }
            }
            for (int i=0; i<clients.socket_descriptor.size(); i++){
                //sends game display during game
                clients.buffer_send_game.push_back(&buffer_send_game);
                clients.buffer_send_game_size.push_back(buffer_send_game_size);
                clients.buffer_send_game_size[i] = game_process_packet((client_server_pkt*)clients.buffer_send_game[i]);
                //send game in process corrd and display to client
                send(clients.socket_descriptor[i], clients.buffer_send_game[i] , clients.buffer_send_game_size[i] , 0 ); 
                sent_pkt_type = process_packet(clients.buffer_send_game[i]);
                printf("loop number: %d\n", i);
            }
        }
    
    for (int i=0; i<clients.socket_descriptor.size(); i++){
        close(clients.socket_descriptor[i]); 
    }
}
