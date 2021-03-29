#include "../client/client.hpp"
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
struct timeval _time;
int total_no_players;


//function to set up socket connection
//1. socket creation
//2. socket connection to server's address and port
int create_connection_socket(string server_ip)
{ 
    
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

    // Creating socket file descriptor 
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    } 
    _time.tv_sec = 20;
  
    memset(&serv_addr_udp, 0, sizeof(serv_addr_udp)); 
    serv_addr_udp.sin_family = AF_INET; 
    serv_addr_udp.sin_port = htons(PORT); 
    setsockopt(udp_sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&_time,sizeof(struct timeval));
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, server_ip.c_str(), &serv_addr_udp.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
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

    }
    return 0;
}

int connection_request_packet(client_server_pkt* buffer_send, char *mac_address){
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to server for connection request
    client_server_pkt send_packet;

    for (int i=0; i<14; i++){
        send_packet.client_mac_address[i] = *(mac_address+i);
    }
    for (int i =0;i<15;i++){
        send_packet.name[i] = 0;
    }
    for (int i =0; i<game::player_name.size();i++){
        send_packet.name[i] = game::player_name[i];
    }

    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif

    send_packet.packet_type = CONNECTION_REQ_PKT;
    send_packet.fpga_or_host = 0;

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

    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif

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

    #ifdef DEBUG
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)send_packet.client_mac_address[0],
            (unsigned char)send_packet.client_mac_address[1],
            (unsigned char)send_packet.client_mac_address[2],
            (unsigned char)send_packet.client_mac_address[3],
            (unsigned char)send_packet.client_mac_address[4],
            (unsigned char)send_packet.client_mac_address[5]);
    #endif

    *buffer_send = send_packet;
    
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int process_packet(char* buffer_recv){
    int pkt_type;
    client_server_pkt* pkt_received = (client_server_pkt*)buffer_recv;
    pkt_type = pkt_received->packet_type;

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
    for (int i = 0; i<5;i++){
        game::stations.emplace_back(x[i],y[i]);
    }
    total_no_players = pkt_received->total_players;
    for (int i =0; i< total_no_players;i++){
        game::players[i].name = "               ";
        game::players[i].is_used = true;
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
    for(int i = 0 ; i<total_no_players;i++){
        players[i].newCoord(x_loc[i],y_loc[i]);
    }
    game::current_task = pkt_received->task_number;
}



void close_game(){
    //
}

int main(int argc, char* argv[]){

    start_ncurses();
    readmap("maps/splash1.txt");
    print_splash_screen(map_screen);
    menu_screen();
    clear_map();
    char mac_address[14];
    if (get_mac_address(mac_address)!=0){
        //error handling
    }
    //set-up connection request packet fields
    client_server_pkt buffer_send;
    int buffer_send_size = connection_request_packet(&buffer_send, mac_address);
    //send connection request to server
    send(sock , (char*)&buffer_send , buffer_send_size , 0 ); 
    #ifdef DEBUG
    printf("Connection request sent\n");
    #endif
    client_server_pkt *test_pkt_1 = (client_server_pkt*)&buffer_send;
    #ifdef DEBUG
    printf("Checking packet number sent: %d\n", test_pkt_1->packet_type);
    #endif

    //receive acknowledgement packet from server - writes to buffer_recv
    char buffer_recv[MAX_COUNT_BYTES] = {0}; 
    int valread = read( sock , buffer_recv, MAX_COUNT_BYTES); 

    //if connected client to a new game: sends over display for player to click a button to indicate you are ready
    int pkt_type = process_packet(buffer_recv);
    if (pkt_type==CONNECTION_ACK_PKT){
        if (process_acknowledgement(buffer_recv, MAX_COUNT_BYTES)!=1){
            //
        }
    }else{
        printf("Detected wrong packet response from server.\n");
    }

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
    readmap(MAP_FILE);
    char buffer_recv_game_start[MAX_COUNT_BYTES] = {0}; 
    int valread_game_start = read( sock , buffer_recv_game_start, MAX_COUNT_BYTES); 
    int user_id;
    pkt_type = process_packet(buffer_recv_game_start);
    if (pkt_type==GAME_START_PKT){
        user_id = process_game_start(buffer_recv_game_start, MAX_COUNT_BYTES);
    }else{
        printf("Detected wrong packet response from server.\n");
    }

    //receive game ongoing coord. and game display from server - writes to buffer_recv

    char buffer_recv_game[MAX_COUNT_BYTES] = {0}; 
    int valread_game;
    client_server_pkt buffer_send_input;
    int buffer_send_input_size;

    readmap("maps/map1.txt");
    print_map_to_screen(map_screen);
    wrefresh(map_screen);
    copy_stations_to_map();

    while(pkt_type!=GAME_END_PKT){

        //send game input to server
        buffer_send_input_size = game_input_packet(&buffer_send_input, mac_address);
        
        if(send(sock , (char *)&buffer_send_input , buffer_send_input_size , 0 )<0){
            printf("Error in sending udp packet\n");
            cout << strerror(errno) << '\n';
            endwin();
            exit(EXIT_FAILURE);
        }

        socklen_t len = sizeof(serv_addr_udp);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        if (read( sock , (char *)buffer_recv_game, MAX_COUNT_BYTES)<0 ){
            printf("Error in receiving udp packet\n");
            cout << strerror(errno) << '\n';
            endwin();
            exit(EXIT_FAILURE);
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
        // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << "[ns]" << std::endl;

        init_info_panel();
        copy_stations_to_map();
        pkt_type = process_packet(buffer_recv_game);
        if (pkt_type==GAME_PROCESS_PKT){
            process_game(buffer_recv_game, MAX_COUNT_BYTES, game::players); 
            game::player_index = user_id;
            game_loop();
            wrefresh(map_screen);
            while(1) {
                if (send(sock, (char *)&buffer_send_input, buffer_send_input_size, 0) < 0)
                {
                    printf("Error in sending udp packet\n");
                    cout << strerror(errno) << '\n';
                    endwin();
                    exit(EXIT_FAILURE);
                }

                game_loop();
               
                wrefresh(map_screen);
                
                if (read( sock , (char *)buffer_recv_game, MAX_COUNT_BYTES)<0 ){
                    // printf("Error in receiving udp packet\n");
                    // endwin();
                }
                pkt_type = process_packet(buffer_recv_game);
                process_game(buffer_recv_game, MAX_COUNT_BYTES, game::players);  //update player coordinates
                if (pkt_type==GAME_END_PKT){
                    endwin();
                    break;
                }
            }
        }else if(pkt_type==GAME_END_PKT){
            //end game?
            clear_map();
            readmap("maps/splash1.txt");
            print_splash_screen(map_screen);
            getch();
            endwin();
        }else{
            printf("Detected wrong packet response from server. Packet type: %d\n", pkt_type);
            exit(1);
        }
        //to clear prev mem of packet received
        memset(buffer_recv_game,'0',MAX_COUNT_BYTES);
    }    
    exit(1);
}
    
     