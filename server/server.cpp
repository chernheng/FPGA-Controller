#include "server.hpp"


using namespace std;

//initialising sock variable
int server_fd = 0;
int udp_fd = 0;
int new_socket = 0;
struct sockaddr_in address;
struct sockaddr_in serv_address;
int addrlen = sizeof(address);
clients_info clients;
vector<string> player_names;
vector<int> FPGA_index; //how many FPGA and their index on the socket descriptor, used to just send to FPGA
vector<int> client_index; // how many clients and their index, used to just send to client
std::map<int,std::string> client_mac; // maps FPGA to mac address
std::map<std::string,int> mac_FPGA;// maps mac address to client index
char all_names[6][15];

//empty function so makefile doesnt complain
int create_connection_socket(string server_ip)
{ 
    return 0; 
} 

int create_udp_connection_socket(string server_ip){
    return 0; 
}


//function to set up socket connection
int create_connection_socket()
{
    // int server_fd, new_socket, valread;
    //struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

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
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
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
    return 1;
}

int create_udp_connection_socket()
{
    //int udp_fd; 
    //char buffer[MAXLINE]; 
    //char *hello = "Hello from server"; 
    // struct sockaddr_in servaddr, cliaddr; 
    int opt =1;
      
    // Creating socket file descriptor 
    if ( (udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&serv_address, 0, sizeof(serv_address)); 
    // memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    serv_address.sin_family    = AF_INET; // IPv4 
    serv_address.sin_addr.s_addr = INADDR_ANY; 
    serv_address.sin_port = htons(PORT); 

    // Forcefully attaching socket to the port 8080
    if (setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
      
    // Bind the socket with the server address 
    if ( bind(udp_fd, (const struct sockaddr *)&serv_address,  
            sizeof(serv_address)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    // int n;
    // socklen_t len; 
  
    // len = sizeof(cliaddr);  //len is value/resuslt 
  
    // n = recvfrom(udp_fd, (char *)buffer, MAXLINE,  
    //             MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
    //             &len); 
    // buffer[n] = '\0'; 
    // printf("Client : %s\n", buffer); 
    // sendto(udp_fd, (const char *)hello, strlen(hello),  
    //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
    //         len); 
    // printf("Hello message sent.\n");  
      
    return 0; 
}

int AcceptClient(int server_socket)
{
    fd_set readfds;
    int max_sd;
    int sd;
    int iResult;
    int client_number = 1;
    int retVal = 0;

    while (1)
    {  
        char * empty_buffer = new char[MAX_COUNT_BYTES];
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;
        //add child sockets to set
        for (int i = 0; i < clients.socket_descriptor.size(); i++)
        {
            sd = clients.socket_descriptor[i];
            FD_SET(sd, &readfds);
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        printf("Waiting for select function...\n");
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        iResult = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        if (iResult < 0)
        {
            printf("Socket error in accepting new connection\n");
            break;
        }
        else if (iResult == 0)
        {
            printf("No additional clients connecting\n");
            printf("====================================\n\n");
            break;
        }
        else if (iResult > 0)
        {
            //If something happened on the master socket , then its an incoming connection
            if (FD_ISSET(server_socket, &readfds))
            {
                if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                {
                    perror("accept");
                    // exit(EXIT_FAILURE);
                    break;
                }
                //read in clients's ip address?
                clients.address.push_back(address);
                clients.address_len.push_back(addrlen);
            }
        }

        client_number++;
        printf("Attempting TCP connection with client %d\n", client_number);
        clients.socket_descriptor.push_back(new_socket);
        printf("TCP connection established with client %d\n", client_number);
        clients.buffer_conn_req.push_back(empty_buffer);

        if ((read(clients.socket_descriptor[client_number - 1], clients.buffer_conn_req[client_number - 1], MAX_COUNT_BYTES)) < 0)
        {
            printf("Error in reading received bytes\n");
        }
        printf("Finished reading into buffer\n");
        retVal = 1;
        //until server detects max number of clients/players
        if (client_number == 10)
        {
            break;
        }

        // memset(empty_buffer,0,MAX_COUNT_BYTES);
    }

    return retVal;
}

int acknowledgement_packet(client_server_pkt *buffer_send)
{
    int buffer_send_size = 0;
    //to set-up packet_fields to be sent to client to acknowledge that client is connect/no connected to a game
    client_server_pkt send_packet;
    send_packet.packet_type = CONNECTION_ACK_PKT;

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}
char user_id = 0;

int game_start_packet(client_server_pkt *buffer_send, TaskStation ts)
{
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_START_PKT;
    send_packet.ch = user_id;
    user_id++;
    for (int i = 0; i< 4;i++){
        send_packet.ts_x[i] = ts.x_stn[i];
        send_packet.ts_y[i] = ts.y_stn[i];
        send_packet.task[i] = ts.task[i];
    }
    send_packet.total_players = client_index.size();
    for (int i = 0; i<client_index.size();i++){
        for (int j = 0;j<15;j++){
            send_packet.all_names[i][j] = all_names[i][j];
        }
    }


    //to set-up packet_fields

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}


int game_process_packet(client_server_pkt *buffer_send, player *players, int id, bool move, int8_t x_value, int8_t y_value, int task)
{
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = GAME_PROCESS_PKT;
    send_packet.task_number = 1;


    //to set-up packet_fields
    if (move){
        // while (x_value || y_value) {
            if (x_value <0){
                players[id].move(RT_DIR, 1);
                // x_value++;
            } else if(x_value>0){
                players[id].move(LF_DIR, 1);
                // x_value--;
            }
            if (y_value <0){
                players[id].move(UP_DIR, 1);
                // y_value++;
            } else if(y_value>0){
                players[id].move(DN_DIR, 1);
                // y_value--;
            }
        // }
        send_packet.task_number = task;
    }
    for (int i =0;i<client_index.size();i++){
        send_packet.x_coord[i] = players[i].x_coord;
        send_packet.y_coord[i] = players[i].y_coord;
    }

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int reject_packet(client_server_pkt *buffer_send)
{
    int buffer_send_size = 0;
    client_server_pkt send_packet;
    send_packet.packet_type = REJECT_PKT;

    //to set-up packet_fields

    *buffer_send = send_packet;
    buffer_send_size = sizeof(send_packet);
    return buffer_send_size;
}

int process_packet(char *buffer_recv)
{
    int pkt_type;
    client_server_pkt *pkt_received = (client_server_pkt *)buffer_recv;
    pkt_type = pkt_received->packet_type;
    if (pkt_type == 1 || pkt_type == 3 || pkt_type == 5){
        printf("Packet received: %d\n", pkt_type);
    //printf("Client's MAC address: %s\n", pkt_received->client_mac_address);
    char mac[18];
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
            (unsigned char)pkt_received->client_mac_address[0],
            (unsigned char)pkt_received->client_mac_address[1],
            (unsigned char)pkt_received->client_mac_address[2],
            (unsigned char)pkt_received->client_mac_address[3],
            (unsigned char)pkt_received->client_mac_address[4],
            (unsigned char)pkt_received->client_mac_address[5]);
    printf("mac address: %s\n", mac);
    } else {
        printf("Packet sent: %d\n", pkt_type);
    }
    return pkt_type;
}

int process_packet_header(char *buffer_recv){
    int8_t pkt_type;
    // client_server_pkt *pkt_received = (client_server_pkt *)buffer_recv;
    pkt_type = (*buffer_recv);
    if (pkt_type==1){
        //fpga packet received
        printf("Fpga packet received.\n");
    }else{
        printf("Host packet received; packet type: %d\n", pkt_type);
    }
    return pkt_type;
}

int process_fpga_coord(char *buffer_recv, fpga_server_pkt* fpga_pkt){
    
    fpga_server_pkt fpga_pkt_received;
    fpga_pkt_received.fpga_or_host = buffer_recv[0];
    
    for (int i=0; i<6; i++){
        fpga_pkt_received.mac_addr[i] = buffer_recv[i+1];
    }
    fpga_pkt_received.x_coord = buffer_recv[7];
    fpga_pkt_received.y_coord = buffer_recv[8];
    fpga_pkt_received.task_complete = buffer_recv[9];


    
    printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned char)fpga_pkt_received.mac_addr[0],
            (unsigned char)fpga_pkt_received.mac_addr[1],
            (unsigned char)fpga_pkt_received.mac_addr[2],
            (unsigned char)fpga_pkt_received.mac_addr[3],
            (unsigned char)fpga_pkt_received.mac_addr[4],
            (unsigned char)fpga_pkt_received.mac_addr[5]);

    printf("x-coord: %d\n", fpga_pkt_received.x_coord);
    printf("y-coord: %d\n",fpga_pkt_received.y_coord);
    printf("task_complete: %d\n",fpga_pkt_received.task_complete);

    *fpga_pkt = fpga_pkt_received;
    return 1;
}

int process_connection_request_fpga(int id, fpga_server_pkt* fpga_pkt_recv)
{
    //to check a global map that stores current clients? - if size>=2, then will send ack, else will wait for another player to connect
    //if after timeout fn and no other player connects - error msg - dont ack player
    //timeout fn to be set in either main fn or ack packet
    //clients_connected.push_back(address);
    //fpga packet received
    char mac_fpga[18];
    sprintf(mac_fpga, "%02x:%02x:%02x:%02x:%02x:%02x", 
        (unsigned char)fpga_pkt_recv->mac_addr[0],
        (unsigned char)fpga_pkt_recv->mac_addr[1],
        (unsigned char)fpga_pkt_recv->mac_addr[2],
        (unsigned char)fpga_pkt_recv->mac_addr[3],
        (unsigned char)fpga_pkt_recv->mac_addr[4],
        (unsigned char)fpga_pkt_recv->mac_addr[5]);
        printf("Fpga packet received.\n");

    string mac_addr(mac_fpga);
    mac_FPGA.insert(pair<std::string,int>(mac_addr,id));
    FPGA_index.push_back(id);
    
    if (clients.socket_descriptor.size() < 2)
    {
        printf("Waiting for other players to connect...\n");
        return 0;
    }
    printf("Minimum number of players reached\n");
    return 1;
}

int process_connection_request_client(char *buffer_conn_req, int id)
{
    //to check a global map that stores current clients? - if size>=2, then will send ack, else will wait for another player to connect
    //if after timeout fn and no other player connects - error msg - dont ack player
    //timeout fn to be set in either main fn or ack packet
    //clients_connected.push_back(address);
    client_server_pkt *pkt_received = (client_server_pkt *)buffer_conn_req;
    std::string name;
    for(int i = 0; i<15;i++){
        name[i] = pkt_received->name[i];
        all_names[id][i] = pkt_received->name[i];
    }
    printf("%s has joined!\n", &(pkt_received->name[0]));

    char mac[18];
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
            (unsigned char)pkt_received->client_mac_address[0],
            (unsigned char)pkt_received->client_mac_address[1],
            (unsigned char)pkt_received->client_mac_address[2],
            (unsigned char)pkt_received->client_mac_address[3],
            (unsigned char)pkt_received->client_mac_address[4],
            (unsigned char)pkt_received->client_mac_address[5]);
            

    string mac_addr(mac);
    client_mac.insert(pair<int,std::string>(id,mac_addr));
    client_index.push_back(id);
    printf("client pushback done\n");
    
    if (clients.socket_descriptor.size() < 2)
    {
        printf("Waiting for other players to connect...\n");
        return 0;
    }
    printf("Minimum number of players reached\n");
    return 1;
}

int process_ready(char *buffer_recv, int buffer_size)
{
    //process the packet sent by client - whether client has indicated they are ready before timeout fn
    //else, remove them from the global vector of connected clients

    //include into vectors of clients that are currently in game
    //push back address of client - to get the address from param?
    clients_in_game.push_back(address);
    return 0;
}

int process_usr_input(char *buffer_usr_input, int buffer_size)
{
    return 1;
}


void process_game(char *buffer_recv_game, int buffer_size)
{
}

void close_game()
{
    //
}

int main()
{
    printf("Setting up TCP socket\n");
    printf("====================================\n");

    if (create_connection_socket() != 1)
    {
        //socket is not set-up properly - TODO: what to respond with?
    }

    printf("\nSetting up UDP socket\n");
    printf("====================================\n");
    if (create_udp_connection_socket() != 1)
        {
            //socket is not set-up properly - TODO: what to respond with?
        }

    while (1)
    {
        //when a connection arrives, open a new socket to communicate with it
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("\nFinished setting up socket.\n");
        printf("====================================\n\n");
        //read in clients's ip address?
        clients.address.push_back(address);
        clients.address_len.push_back(addrlen);


        //initialising buffer for read function to store received bytes into (able to read/store a max of max_count_bytes)
        char empty_buffer[MAX_COUNT_BYTES] = {0};
        //storing client 1's socket info
        clients.socket_descriptor.push_back(new_socket);
        clients.buffer_conn_req.push_back(empty_buffer);

        //error returns -1
        if ((read(clients.socket_descriptor[0], clients.buffer_conn_req[0], MAX_COUNT_BYTES)) < 0)
        {
            printf("Error in reading received bytes\n");
        }

        //check if packet is of fpga packet
        int pkt_header_type;
        pkt_header_type = process_packet_header(clients.buffer_conn_req[0]);

        // //check if received packet is of connection req type
        int pkt_type;
        
        // pkt_type = process_packet(clients.buffer_conn_req[0]);
        // if (pkt_type != CONNECTION_REQ_PKT)
        // {
        //     printf("Detected wrong packet response from client.\n");
        // }

        if (pkt_header_type == 0){
        //when there is not enough players - wait for another client's connection
            if (process_connection_request_client(clients.buffer_conn_req[0], 0) != 1)
            {
                //int timeout=10;
                printf("Attempting to detect second client\n");

                int iResult = AcceptClient(server_fd);
                if (iResult == 0)
                {
                    //some error occured with the sockets
                }
            }
        } else if (pkt_header_type == 1) {
            fpga_server_pkt fpga_pkt;
            int process = process_fpga_coord(clients.buffer_conn_req[0], &fpga_pkt);
            if (process_connection_request_fpga(0, &fpga_pkt) != 1)
            {
                //int timeout=10;
                printf("Attempting to detect second client\n");

                int iResult = AcceptClient(server_fd);
                if (iResult == 0)
                {
                    //some error occured with the sockets
                }
            }
             
        }
        printf("socket_descriptor size: %d, buffercon size: %d \n", (int) clients.socket_descriptor.size(),(int) clients.buffer_conn_req.size());
        //for each client connection:
        for (int i = 1; i < clients.socket_descriptor.size(); i++)
        {
            printf("Processing client number: %d\n", i);
            pkt_header_type = process_packet_header(clients.buffer_conn_req[i]);
            if (pkt_header_type == 0){
            //when there is not enough players - wait for another client's connection
                if (process_connection_request_client(clients.buffer_conn_req[i], i) != 1)
                {
                    //int timeout=10;
                    printf("connection req failed\n");
                }
            } else if (pkt_header_type == 1) {
                fpga_server_pkt fpga_pkt;
                int process = process_fpga_coord(clients.buffer_conn_req[i], &fpga_pkt);
                if (process_connection_request_fpga(i, &fpga_pkt) != 1)
                {
                    //int timeout=10;
                    printf("connection req failed\n");
                }        
            }
        }

        // char buffer_send_ack;
        // int buffer_send_ack_size;
        // char buffer_send_game_start;
        // int buffer_send_game_start_size;
        char buffer_send_game;
        int buffer_send_game_size;

        for (int i = 0; i < client_index.size(); i++)
        {
            printf("Processing client number: %d\n", i);
            client_server_pkt buffer_send_ack;
            int buffer_send_ack_size;
            // clients.buffer_send_ack.push_back(&buffer_send_ack);
            // clients.buffer_send_ack_size.push_back(buffer_send_ack_size);

            buffer_send_ack_size = acknowledgement_packet(&buffer_send_ack);

            //send ack packet to client
            send(clients.socket_descriptor[client_index[i]], (char *)&buffer_send_ack, buffer_send_ack_size, 0);
            printf("Acknowledge packet sent\n");
        }
        for (int i = 0; i < client_index.size(); i++)
        {
            //receive play ready packet from client
            //clients.buffer_ready.push_back(empty_buffer);
            char recv_buffer_ready[1024];
            if ((read(clients.socket_descriptor[client_index[i]], recv_buffer_ready, MAX_COUNT_BYTES)) < 0)
            {
                printf("Error in reading received bytes\n");
            }

            //TODO: process the packet sent by client - whether client has indicated they are ready before timeout fn
            //else, remove them from the global vector of connected clients
            pkt_type = process_packet(recv_buffer_ready);
            if (pkt_type == PLAYER_READY_PKT)
            {
                if (process_ready(recv_buffer_ready, 1024) != 1)
                {
                    //if client times-out - goes back to start of loop? (maybe case structure)
                }
            }
            else
            {
                printf("Detected wrong packet response from client.\n");
            }

        }

        for (int i = 0; i < FPGA_index.size(); i++)
        {
            char sendchar = '0';
            printf("Sending char to fpga\n");
            send(clients.socket_descriptor[FPGA_index[i]], &sendchar, sizeof(sendchar), 0);     

            char recv_data[MAX_COUNT_BYTES] = {0};
            int iBytesReceived;
            //error returns -1
            if ((iBytesReceived = read(clients.socket_descriptor[FPGA_index[i]], recv_data, MAX_COUNT_BYTES)) < 0)
            {
                printf("Error in reading received bytes\n");
            }
            printf("FPGA %d ready\n", i);

        }






        readmap("maps/map1.txt");
        TaskStation ts[client_index.size()];
        for (int i = 0; i<client_index.size();i++){
            ts[i] = TaskStation(i);
        }
        int sent_pkt_type;
        for (int i = 0; i < client_index.size(); i++)
        {
            pkt_header_type = process_packet_header(clients.buffer_conn_req[i]);
            if (pkt_header_type == 0){
                //send game start packet
                //set-up connection game-start coord. and game start display
                client_server_pkt buffer_send_game_start;
                int buffer_send_game_start_size;
                // clients.buffer_send_game_start.push_back(&buffer_send_game_start);
                // clients.buffer_send_game_start_size.push_back(buffer_send_game_start_size);
                buffer_send_game_start_size = game_start_packet(&buffer_send_game_start, ts[i]);
                //send game start coord and display to client
                send(clients.socket_descriptor[client_index[i]], (char *)&buffer_send_game_start, buffer_send_game_start_size, 0);
                printf("Game start coordinates and game display sent\n");
                sent_pkt_type = process_packet((char *)&buffer_send_game_start);
            }
        }
        // sendto(server_fd, clients.buffer_send_game_start[i], clients.buffer_send_game_start_size[i], MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
        //         len);

        


        struct sockaddr_in cliaddr; 
        memset(&cliaddr, 0, sizeof(cliaddr)); 
        socklen_t len = sizeof(cliaddr);
        vector<sockaddr_in> vec_cliaddr;
        vector<socklen_t> vec_cliaddr_len;

        player players[client_index.size()];
        for (int i = 0; i<client_index.size();i++){
            players[i] = player();
        }

        while (sent_pkt_type != GAME_END_PKT)
        {
            for (int i = 0; i < client_index.size(); i++)
            {
                clients.buffer_usr_input.push_back(empty_buffer);
                // if ((read(clients.socket_descriptor[i], clients.buffer_usr_input[i], MAX_COUNT_BYTES)) < 0)
                // {
                //     printf("Error in reading received bytes\n");
                // }
                printf("Attempting to receive udp game input packets from client...\n");
                int n;
                vec_cliaddr.push_back(cliaddr);
                vec_cliaddr_len.push_back(sizeof(cliaddr));
                if(n = recvfrom(udp_fd, clients.buffer_usr_input[i], MAX_COUNT_BYTES, MSG_WAITALL, (struct sockaddr *)&vec_cliaddr[client_index[i]], &vec_cliaddr_len[client_index[i]])<0){
                    printf("Error in udp bytes received\n");
                }
                //clients.buffer_usr_input[i][n] = '\0';
                // if(n<0){
                    
                // }else if (n==0){
                //     printf("No udp bytes received\n");
                // }
                printf("Receive udp game input packets from client\n");
                clients.buffer_usr_input[i][n]='\0';
                for (int i=0; i<client_index.size(); i++){
                    if(clients.address[client_index[i]].sin_addr.s_addr==cliaddr.sin_addr.s_addr){
                        printf("Processing user input\n");
                        if (process_usr_input(clients.buffer_usr_input[i], 1024) != 1)
                        {
                            //if client times-out - goes back to start of loop? (maybe case structure)
                        }
                    }
                }
                // printf("%s\n", inet_ntoa(clients.address[i].sin_addr));
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

                // if (process_usr_input(clients.buffer_usr_input[i], 1024) != 1)
                // {
                //     //if client times-out - goes back to start of loop? (maybe case structure)
                // }
            }
            while(1){
                for (int i = 0; i < client_index.size(); i++)
                {
                    // vector<int> x = ts[i].x_stn;
                    // vector<int> y = ts[i].y_stn;
                    bool move = true;
                    //sends game display during game
                    vector<int>::iterator it_x = find(ts[i].x_stn.begin(),ts[i].x_stn.end(),players[i].x_coord);
                    vector<int>::iterator it_y = find(ts[i].y_stn.begin(),ts[i].y_stn.end(),players[i].y_coord);
                    if ((it_x - ts[i].x_stn.begin()) == (it_y - ts[i].y_stn.begin()) && it_x!=ts[i].x_stn.end() && it_y!=ts[i].y_stn.end()) {
                        move = false;
                    }
                    std::map<int,std::string>::iterator client_it;
                    std::map<std::string,int>::iterator FPGA_it;
                    client_it = client_mac.find(client_index[i]);
                    FPGA_it = mac_FPGA.find(client_it->second);
                    printf("FPGA_index: %d\n",FPGA_it->second);
                    fpga_server_pkt fpga_pkt;
                    int8_t x_value;
                    int8_t y_value;
                    int8_t task_complete;
                    int task;


                    if(move){
                        char sendchar = '1';
                        printf("Sending char to fpga\n");
                        send(clients.socket_descriptor[FPGA_it->second], &sendchar, sizeof(sendchar), 0);

                        char recv_data[MAX_COUNT_BYTES] = {0};
                        int iBytesReceived;
                        //error returns -1
                        if ((iBytesReceived = read(clients.socket_descriptor[FPGA_it->second], recv_data, MAX_COUNT_BYTES)) < 0)
                        {
                            printf("Error in reading received bytes\n");
                        }
                        int process = process_fpga_coord(recv_data, &fpga_pkt);
                        x_value = fpga_pkt.x_coord;
                        y_value = fpga_pkt.y_coord;
                    }else if(move ==false) {
                        int index =(it_x - ts[i].x_stn.begin());
                        task = ts[i].task[index];
                        task = task+2;
                        char ta[2];
                        sprintf(ta,"%d",task);
                        char sendchar = ta[0];
                        printf("Sending char to fpga\n");
                        send(clients.socket_descriptor[FPGA_it->second], &sendchar, sizeof(sendchar), 0);

                        char recv_data[MAX_COUNT_BYTES] = {0};
                        int iBytesReceived;
                        //error returns -1
                        if ((iBytesReceived = read(clients.socket_descriptor[FPGA_it->second], recv_data, MAX_COUNT_BYTES)) < 0)
                        {
                            printf("Error in reading received bytes\n");
                        }
                        int process = process_fpga_coord(recv_data, &fpga_pkt);
                        x_value = fpga_pkt.x_coord;
                        y_value = fpga_pkt.y_coord;
                        task_complete = fpga_pkt.task_complete;
                        if(task_complete = 1){
                            ts[i].x_stn.erase(it_x);
                            ts[i].y_stn.erase(it_y);
                            ts[i].task.erase(ts[i].task.begin()+index);
                        }
                    }

                    client_server_pkt buffer_send_game;
                    int buffer_send_game_size;

                    //clients.buffer_send_game.push_back(&buffer_send_game);
                    //clients.buffer_send_game_size.push_back(buffer_send_game_size);
                    buffer_send_game_size = game_process_packet(&buffer_send_game, players, i,move, x_value, y_value, task);
                    //send game in process corrd and display to client
                    //send(clients.socket_descriptor[i], clients.buffer_send_game[i], clients.buffer_send_game_size[i], 0);
                    
                    if(sendto(udp_fd, (const char *)&buffer_send_game, buffer_send_game_size, MSG_CONFIRM, (const struct sockaddr *) &vec_cliaddr[client_index[i]], vec_cliaddr_len[client_index[i]])<0){
                        perror("sending udp bytes failed"); 
                        exit(EXIT_FAILURE); 
                        //printf("Error in udp bytes sent\n");
                    }
                    
                    sent_pkt_type = process_packet((char *)&buffer_send_game);
                    printf("loop number: %d\n", i);
                    
                }
                // for (int i = 0; i < clients.socket_descriptor.size(); i++)
                // {
                // }
            }
        }

        for (int i = 0; i < clients.socket_descriptor.size(); i++)
        {
            close(clients.socket_descriptor[i]);
        }

	clients = {};
    }
}