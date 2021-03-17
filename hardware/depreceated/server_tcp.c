// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <ctype.h>

#define PORT 8080 
int main(int argc, char const *argv[]) {
    /* Socket parameters */
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
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
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    /* IMPORTANT CODE HERE */

    // Get an int back
    #define RECV_LEN 3
    int recv_data[RECV_LEN] = {0};
    valread = read( new_socket , &recv_data, sizeof(recv_data) );
    // Data is sent as bytes
    for(unsigned i=0; i<RECV_LEN; i++){
        recv_data[i] = ntohl(recv_data[i]);     // Remember to do translation back to host endinanness
    }
    printf("X: %d | Y: %d | Task_Complete: %d | Valread:%d\n", recv_data[0], recv_data[1], recv_data[2], valread);

    char *ack_msg = "Recieved."; 
    send(new_socket , ack_msg , strlen(ack_msg) , 0 );  // This will be substituted to be the STATE for the player
    printf("Acknowledge message sent\n");
    return 0; 
}
