#include "client.hpp"
#define PORT 8080 
// #define DEBUG

using namespace std;

//initialising sock variable
int sock = 0;
int udp_sockfd; 
struct sockaddr_in serv_addr; 
struct sockaddr_in serv_addr_udp; 
//char mac_address[18] = {0};
vector<string> all_player_names;


//function to set up socket connection
//1. socket creation
//2. socket connection to server's address and port
int create_connection_socket(string server_ip)
{ 
    // struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    memset(&serv_addr, 0, sizeof(serv_addr)); 
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

int create_udp_connection_socket(string server_ip){

    char buffer[MAX_COUNT_BYTES]; 
    // char* message = "Hello Server"; 
    // struct sockaddr_in serv_addr; 
  
    //int n, len; 
    // Creating socket file descriptor 
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    } 
  
    memset(&serv_addr_udp, 0, sizeof(serv_addr_udp)); 
    serv_addr_udp.sin_family = AF_INET; 
    serv_addr_udp.sin_port = htons(PORT); 
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, server_ip.c_str(), &serv_addr_udp.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
    // // send hello message to server 
    // sendto(sockfd, (const char*)message, strlen(message), 
    //        0, (const struct sockaddr*)&servaddr, 
    //        sizeof(servaddr)); 
  
    // // receive server's response 
    // printf("Message from server: "); 
    // n = recvfrom(sockfd, (char*)buffer, MAXLINE, 
    //              0, (struct sockaddr*)&servaddr, 
    //              &len); 
    // puts(buffer); 
    // close(sockfd);
    #ifdef DEBUG
    printf("Created udp socket\n"); 
    #endif
    return 0; 
}

int get_mac_address(char *mac_address){
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    // int server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    // if (server_fd == -1) { 
    //     /* handle error*/ 
        
    // };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { 
        /* handle error */ 
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { /* handle error */ }
    }

    // unsigned char mac[6];

    // if (success) memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    // //display mac address
    // //prints out each char as hex of length 2
	// printf("Mac : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // // printf("mac address: %s\n", mac_address);


    // char mac[18]; /* one extra for terminating '\0'; 
    //                  You may want to make it a little larger
    //                  still, just to be sure. */
    if(success==1){
        for(int i=0; i<14; i++){
            *(mac_address+i) = *(ifr.ifr_hwaddr.sa_data+i);
        }
        
        //char mac[18];
        #ifdef DEBUG
        printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        #endif
        //printf("mac address: %s\n", mac);

        //printf("mac address: %s\n", mac);
        //*mac_address = mac;

    }
    return 0;
}

int connection_request_packet(client_server_pkt* buffer_send, char *mac_address){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection request
    client_server_pkt send_packet;
    // string my_mac_address = mac_address;
    for (int i=0; i<14; i++){
        send_packet.client_mac_address[i] = *(mac_address+i);
    }
    for (int i =0;i<15;i++){
        send_packet.name[i] = 0;
    }
    for (int i =0; i<game::player_name.size();i++){
        send_packet.name[i] = game::player_name[i];
    }
    //printf("mac address: %s\n", send_packet.client_mac_address);
    //char mac[18];
    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif
    //printf("mac address: %s\n", mac);
    // send_packet.client_mac_address = mac_address;
    send_packet.packet_type = CONNECTION_REQ_PKT;


    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int player_ready_packet(client_server_pkt* buffer_send, char *mac_address){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection ready signal
    client_server_pkt send_packet;
    
    for (int i=0; i<14; i++){
        send_packet.client_mac_address[i] = *(mac_address+i);
    }
    //char mac[18];
    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif
    //printf("mac address: %s\n", mac);

    send_packet.packet_type = PLAYER_READY_PKT;
    *buffer_send = send_packet;
    #ifdef DEBUG
    printf("Checking packet number sent (in player_ready fn): %d\n", buffer_send->packet_type);
    #endif

    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int game_input_packet(client_server_pkt* buffer_send, char *mac_address){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection ready signal
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_INPUT_PKT;
    for (int i=0; i<14; i++){
        send_packet.client_mac_address[i] = *(mac_address+i);
    }
    //char mac[18];
    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif
    //printf("mac address: %s\n", mac);

    *buffer_send = send_packet;
    
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int process_packet(char* buffer_recv){
    int pkt_type;
    client_server_pkt* pkt_received = (client_server_pkt*)buffer_recv;
    pkt_type = pkt_received->packet_type;
    // printf("Packet received: %d\n", pkt_type);
    return pkt_type;
}

int process_acknowledgement(char *buffer_recv, int buffer_size){
    //
}

char process_game_start(char* buffer_recv_game_start, int buffer_size){
    //to process display and player's coord.
    char ch;
    client_server_pkt* pkt_received = (client_server_pkt*)buffer_recv_game_start;
    ch = pkt_received->ch;
    #ifdef DEBUG
    printf("User_id: %d\n",ch); 
    #endif
    int *x = pkt_received->ts_x;
    int *y = pkt_received->ts_y;
    int *task = pkt_received->task;
    for (int i = 0; i<4;i++){
        game::stations.emplace_back(x[i],y[i]);
    }
    int total_no_players = pkt_received->total_players;
    for (int i =0; i< total_no_players;i++){
        game::players[i].name = "               ";
        for (int j = 0;j<15;j++){
            game::players[i].name[j] = pkt_received->all_names[i][j];
        }
    }
    return ch;
    
}

void process_game(char* buffer_recv_game, int buffer_size, player *players){
    client_server_pkt* pkt_received = (client_server_pkt*)buffer_recv_game;
    int *x_loc = pkt_received->x_coord;
    int *y_loc = pkt_received->y_coord;
    for(int i = 0 ; i<2;i++){
        players[i].newCoord(x_loc[i],y_loc[i]);
    }
}



void close_game(){
    //
}

int main(int argc, char* argv[]){
    // if (argc>2){
    //     printf("Too many arguments. Please provide only the server's IP address.\n ");
    // }
    // if (argc<2){
    //     printf("Please input the server's IP address.\n");
    // }
    // string server_ip = argv[1];
    // //set-up of socket for connecting to server
    // if (create_connection_socket(server_ip)!=0){
    //     //socket is not set-up properly - TODO: what to respond with?
    // }
    start_ncurses();
    menu_screen();
    char mac_address[14];
    if (get_mac_address(mac_address)!=0){
        //error handling
    }
    //set-up connection request packet fields
    client_server_pkt buffer_send;
    int buffer_send_size = connection_request_packet(&buffer_send, mac_address);
    //send connection request to server
    //TODO: check that sock != 0 - ensuring that socket has been set-up properly
    send(sock , (char*)&buffer_send , buffer_send_size , 0 ); 
    #ifdef DEBUG
    printf("Connection request sent\n");
    #endif
    client_server_pkt *test_pkt_1 = (client_server_pkt*)&buffer_send;
    #ifdef DEBUG
    printf("Checking packet number sent: %d\n", test_pkt_1->packet_type);
    #endif

    //receive acknowledgement packet from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    char buffer_recv[MAX_COUNT_BYTES] = {0}; 
    int valread = read( sock , buffer_recv, MAX_COUNT_BYTES); 
    //TODO:error detection
    
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
    client_server_pkt buffer_send_ready;
    int buffer_send_ready_size = player_ready_packet(&buffer_send_ready, mac_address);
    //send connection ready to server
    send(sock , (char *)&buffer_send_ready , buffer_send_ready_size , 0 ); 
    client_server_pkt *test_pkt = (client_server_pkt*)&buffer_send_ready;
    #ifdef DEBUG
    printf("Checking packet number sent: %d\n", test_pkt->packet_type);
    printf("Player ready sent\n");
    #endif

    //receive game start coord. and game display from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    readmap("maps/map1.txt");
    char buffer_recv_game_start[MAX_COUNT_BYTES] = {0}; 
    int valread_game_start = read( sock , buffer_recv_game_start, MAX_COUNT_BYTES); 
    int user_id;
    pkt_type = process_packet(buffer_recv_game_start);
    if (pkt_type==GAME_START_PKT){
        user_id = process_game_start(buffer_recv_game_start, MAX_COUNT_BYTES);
    }else{
        printf("Detected wrong packet response from server.\n");
    }
    //to test if other clients can connect while players are in game
    usleep(3*1000000);

    //receive game ongoing coord. and game display from server - writes to buffer_recv
    //TODO: add timeout fn here - may be trying to connect to server when it is down
    char buffer_recv_game[MAX_COUNT_BYTES] = {0}; 
    int valread_game;
    client_server_pkt buffer_send_input;
    int buffer_send_input_size;
    // = read( sock , buffer_recv_game, MAX_COUNT_BYTES); 
    // process_game(buffer_recv_game, MAX_COUNT_BYTES);

    //TODO: while game is ongoing, keep receiving packets - while loop; while the msg received is not the end game packet
    // char* end_game_packet;
    // end_game_packet_struct(end_game_packet);
    // if (create_udp_connection_socket(server_ip)!=0){
    //     //some error in socket creation

    // }
    // start_ncurses();
    // menu_screen();
    // read map
    readmap("maps/map1.txt");
    print_map_to_screen(map_screen);
    wrefresh(map_screen);
    copy_stations_to_map();
    // ts.newTask(r,s,t);
    // TaskStation t1;
    while(pkt_type!=GAME_END_PKT){
        //buffer_recv_game = 0; 
        //send game input to server
        // buffer_send_input_size = game_input_packet(&buffer_send_input, mac_address);
        // printf("Sending packets on udp\n");
        //send(udp_sockfd , (char *)&buffer_send_input , buffer_send_input_size , 0 ); 
        if (sendto(udp_sockfd , (const char *)&buffer_send_input , buffer_send_input_size , MSG_CONFIRM, (const struct sockaddr*)&serv_addr_udp, sizeof(serv_addr_udp))<0){
            printf("Error in sending udp packet\n");
            cout << strerror(errno) << '\n';
            endwin();
            exit(EXIT_FAILURE);
        }
        // printf("Sent udp packet of game input\n");

        socklen_t len = sizeof(serv_addr_udp);
        //receive client's coord. and other players' coord.
        //valread_game = read( sock , buffer_recv_game, MAX_COUNT_BYTES);
        if (recvfrom(udp_sockfd, (char *)buffer_recv_game, MAX_COUNT_BYTES, MSG_WAITALL, (struct sockaddr*)&serv_addr_udp, &len)<0){
            printf("Error in receiving udp packet\n");
            cout << strerror(errno) << '\n';
            endwin();
            exit(EXIT_FAILURE);
        }
        // printf("Received udp packets from server\n");
        game::players[1].is_used = true;
        init_info_panel();
        copy_stations_to_map();
        pkt_type = process_packet(buffer_recv_game);
        if (pkt_type==GAME_PROCESS_PKT){
            process_game(buffer_recv_game, MAX_COUNT_BYTES, game::players); 
            while(1) {
                if (sendto(udp_sockfd , (const char *)&buffer_send_input , buffer_send_input_size , MSG_CONFIRM, (const struct sockaddr*)&serv_addr_udp, sizeof(serv_addr_udp))<0){
                printf("Error in sending udp packet\n");
                cout << strerror(errno) << '\n';
                endwin();
                exit(EXIT_FAILURE);
            }
                // print_station(ts,map_screen);
                game::player_index = user_id;
                game_loop();
                // vector<int>::iterator it_x = find(x.begin(),x.end(),players[0].x_coord);
                // vector<int>::iterator it_y = find(y.begin(),y.end(),players[0].y_coord);
                // if ((it_x - x.begin()) == (it_y - y.begin()) && it_x!=x.end() && it_y!=y.end()) {
                //     while(getch() !='p'){
                //         //execute task
                //         }
                // }

                wrefresh(map_screen);
                if (recvfrom(udp_sockfd, (char *)buffer_recv_game, MAX_COUNT_BYTES, MSG_WAITALL, (struct sockaddr*)&serv_addr_udp, &len)<0){
                    printf("Error in receiving udp packet\n");
                    endwin();
                    cout << strerror(errno) << '\n';
                    exit(EXIT_FAILURE);
                }
                pkt_type = process_packet(buffer_recv_game);
                process_game(buffer_recv_game, MAX_COUNT_BYTES, game::players); 
                if (pkt_type==GAME_END_PKT){
                    endwin();
                    break;
                }
            }
        }else if(pkt_type==GAME_END_PKT){
            //end game?
            endwin();
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
    
     