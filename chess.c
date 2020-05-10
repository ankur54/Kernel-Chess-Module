#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DRIVER_FILE "/dev/chess"
#define MAX_SIZE 1024

#define UNKN_ERR    -1
#define OK           0
#define ILLMOVE      1
#define PWIN         2
#define CPUWIN       3
#define TIE          4

int8_t respbuf[MAX_SIZE];
int fd, moves, player;

static void send_cmd(int fd, char *cmd, size_t len) {
    if (write(fd, cmd, len + 1) != len + 1) {
        printf("Couldn't write command to /dev/tic-tac-toe device: %s\n", strerror(errno));
        close(fd);
        exit(1);
    }
    
    if (read(fd, respbuf, 1024) < 0) {
        printf("Couldn't read response from /dev/tic-tac-toe device: %s\n", strerror(errno));
        close(fd);
        exit(1);
    }

    if (strcmp(respbuf, "OK") == 0 || strcmp(respbuf, "ILLMOV") == 0 || strcmp(respbuf, "CHECK") == 0 ||
        strcmp(respbuf, "INVFMT") == 0 ||strcmp(respbuf, "OOT") == 0 || strcmp(respbuf, "UNKCMD") == 0 || 
        strcmp(respbuf, "MATE") == 0 || strcmp(respbuf, "NOGAME") == 0) {
        
        printf("< %s\n", respbuf);
    } 
    else {
        printf("  ");
        for (int i = 'a'; i <= 'h'; i++)
            printf("%c  ", i);
        printf("\n");

        for (int i = 0, c = 0; i < 8; i++) {
            printf("%d ", i + 1);
            for (int j = 0; j < 8; j++, c += 2)
                printf("%c%c ", respbuf[c], respbuf[c+1]);
            printf("\n");
        }
    }
}

int main(int argc, char* argv[]) {
    int fd;
    char cmd[MAX_SIZE];

    if((fd = open(DRIVER_FILE, O_RDWR)) < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }

    while(1) {
        printf("> ");
        fgets(cmd, MAX_SIZE, stdin); 
        // printf("%ld\n", strlen(cmd));
        
        // if ((strcmp(cmd, "00 X\n") == 0) || (strcmp(cmd, "00 O\n") == 0)) {
            // send_cmd(fd, cmd, strlen(cmd));
            // close(fd);
        // }
        
        // else if (strcmp(cmd, "01\n") == 0) {
            // fd = open("/dev/test_game", O_RDWR);
            // send_cmd(fd, cmd, strlen(cmd));
            // close(fd);
        // }

        // else if (strlen(cmd) == 7 && cmd[0] == '0' && cmd[1] == '2' && cmd[2] == ' ' && cmd[4] == ' ') {
            // fd = open("/dev/test_game", O_RDWR);
            // send_cmd(fd, cmd, strlen(cmd));
            // close(fd);
        // }

        // else if (strcmp(cmd, "03\n") == 0) {
            // fd = open("/dev/test_game", O_RDWR);
            // send_cmd(fd, cmd, strlen(cmd));
            // close(fd);
        // }

        if (strcmp(cmd, "quit\n") == 0) {
            close(fd);
            printf("Exiting...!!!\n");
            break;
        }

        send_cmd(fd, cmd, strlen(cmd)); 

        // else {
            // printf("UNKCMD\n")
        // }
    }
    close(fd);
    return 0;
}