
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#define MSG_SIZE 12

void init_BLT_sock(int *BLT_client){
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    int s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 22 of the first available
    // local bluetooth adapter

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;

    //Channel for BBB set to 22
    loc_addr.rc_channel = (uint8_t) 22;

    int err1 = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    int err2 = listen(s, 1);

    // accept one connection
    int client = accept(s, (struct sockaddr *)&rem_addr, &opt);
    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    BLT_client[0]= s;
    BLT_client[1] = client;

}
void BLT_end(int *BLT_client){
    close(BLT_client[1]);
    close(BLT_client[0]);

}
void BLT_Recv(int client, char *message){

    // read data from the client
    read(client, message, MSG_SIZE);
/*    if( bytes_read > 0 ) {
        printf("received [%s]\n", message);
    }
*/
}

int main(){
    int BLT_client[2];
    init_BLT_sock(BLT_client);
    char message[1024];
    while(1){
    BLT_Recv(BLT_client[1], message);
    printf("MESSAGE IS %s\n", message);
    }
//    send_instr(message, p_socket);



    BLT_end(BLT_client);
    return 0;
}
