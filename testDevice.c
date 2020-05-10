#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>

#define BUFF_SIZE 1024
#define game "chess"

dev_t dev = 0;
struct class *dev_class;
struct cdev test_cdev;
static uint8_t *kernel_buffer;
static uint8_t board[8][8][2];
static char player = '0';
static int moves = 0;
static int over = 0;
static int check = 0;
int status;
int init_x, init_y, fin_x, fin_y;
int king_x, king_y;
int test_x, test_y;
int left_dia = 0, right_dia = 0, straight = 0, side = 0;
int i, j, done;




// To hold he file operations performed on this device

static int test_open (struct inode * pinode, struct file * pfile) {
    if((kernel_buffer = kmalloc(BUFF_SIZE , GFP_KERNEL)) == 0){
        printk(KERN_INFO "Cannot allocate memory in kernel\n");
        return -1;
    }
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    return 0;
}

static ssize_t test_read (struct file * pfile, char __user * buffer, size_t length, loff_t * offset) {
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    printk(KERN_ALERT "Kernel read: %s\n", kernel_buffer);
    copy_to_user(buffer, kernel_buffer, length);
    memset(kernel_buffer, 0, strlen(kernel_buffer));
    return 0;
}

static ssize_t test_write (struct file * pfile, const char __user * buffer, size_t length, loff_t * offset) {
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    copy_from_user(kernel_buffer, buffer, length);
    printk(KERN_NOTICE "Kernel: %s", kernel_buffer);
    
    if (kernel_buffer[0] == '0' && (kernel_buffer[1] == '0' || kernel_buffer[1] == '1' || kernel_buffer[1] == '2' || kernel_buffer[1] == '3' || kernel_buffer[1] == '4')) {
        if (kernel_buffer[1] == '0') {
            int i, j;
            for (i = 0; i < 8; i++) {
                if (i > 1 &&i < 6) {
                    for (j = 0; j < 8; j++) 
                        strcpy(board[i][j], "**");
                    // board[i][8][0] = '\0';
                }
                else {
                    if (i == 1 || i == 6) {
                        for (j = 0; j < 8; j++) 
                            board[i][j][0] = (i == 1 ? 'W' : 'B'),
                            board[i][j][1] = 'P';
                        // board[i][8][0] = '\0';
                    } else {
                        for (j = 0; j < 8; j++) {
                            board[i][j][0] = (i == 0 ? 'W' : 'B');
                            if (j == 0)
                                board[i][j][1] = 'R';
                            else if (j == 1)
                                board[i][j][1] = 'N';
                            else if (j == 2)
                                board[i][j][1] = 'B';
                            else if (j == 3)
                                board[i][j][1] = 'Q';
                            else if (j == 4)
                                board[i][j][1] = 'K';
                            else if (j == 5)
                                board[i][j][1] = 'B';
                            else if (j == 6)
                                board[i][j][1] = 'N';
                            else if (j == 7)
                                board[i][j][1] = 'R';  
                        }
                    }
                }
            }
            over = 0;
            moves = 0;
            if (kernel_buffer[2] == 'W' || kernel_buffer[2] == 'B') 
                strcpy(kernel_buffer, "INVFMT");
            else if (kernel_buffer[2] == ' ' && (kernel_buffer[3] == 'W' || kernel_buffer[3] == 'B')){
                player = kernel_buffer[3];
                
                strcpy(kernel_buffer, "OK\0");
            }
            else strcpy(kernel_buffer, "UNKCMD");
        }

        else if (kernel_buffer[1] == '1') {
            memcpy(kernel_buffer, board, sizeof(board));
        }

        else if (!over) {
            if (kernel_buffer[1] == '2') {
                if (kernel_buffer[2] == 'W' && kernel_buffer[2] == 'B') 
                    strcpy(kernel_buffer, "INVFMT");

                else if (kernel_buffer[2] == ' ' && (kernel_buffer[3] == 'W' || kernel_buffer[3] == 'B')) {
                    
                    if (player == kernel_buffer[3] && ((player == 'B' && moves%2 == 1) || (player == 'W' && moves%2 == 0))) {

                        if (kernel_buffer[4] == 'P' || kernel_buffer[4] == 'K' || kernel_buffer[4] == 'Q' || kernel_buffer[4] == 'N' || kernel_buffer[4] == 'R' || kernel_buffer[4] == 'B') {

                            if ((kernel_buffer[5] >= 'a' && kernel_buffer[5] <= 'h') && (kernel_buffer[6] >= '1' && kernel_buffer[6] <= '8')) {

                                if (kernel_buffer[7] == '-') {

                                    if ((kernel_buffer[8] >= 'a' && kernel_buffer[8] <= 'h') && (kernel_buffer[9] >= '1' && kernel_buffer[9] <= '8')) {

                                        init_x = kernel_buffer[5] - 'a';
                                        init_y = kernel_buffer[6] - '1';
                                        fin_x = kernel_buffer[8] - 'a';
                                        fin_y = kernel_buffer[9] - '1';

                                        // printk(KERN_NOTICE "%d %d %d %d", init_x, init_y, fin_x, fin_y);

                                        if (board[init_y][init_x][0] == kernel_buffer[3] && board[init_y][init_x][1] == kernel_buffer[4]) {

                                            // check move according to the piece

                                            if (kernel_buffer[4] == 'P') {
                                                
                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if (init_x == fin_x) {

                                                        printk(KERN_INFO "testing: %c%c", board[fin_y][fin_x][0], board[fin_y][fin_x][1]);

                                                        if (player == 'W' && ((init_y + 1 == fin_y) || ((init_y == 1) && (init_y + 2 == fin_y))) && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                            board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                            board[fin_y][fin_x][0] = kernel_buffer[3];
                                                            board[fin_y][fin_x][1] = kernel_buffer[4];
                                                            strcpy(kernel_buffer, "OK");
                                                            moves++;
                                                        
                                                        }
                                                        else if (player == 'B' && ((init_y - 1 == fin_y) || ((init_y == 6) && (init_y - 2 == fin_y))) && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                            board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                            board[fin_y][fin_x][0] = kernel_buffer[3];
                                                            board[fin_y][fin_x][1] = kernel_buffer[4];
                                                            strcpy(kernel_buffer, "OK");
                                                            moves++;

                                                        } else strcpy(kernel_buffer, "ILLMOV");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if (init_x == fin_x - 1 || init_x == fin_x + 1) {

                                                        if (player == 'W' && kernel_buffer[11] == 'B' && init_y + 1 == fin_y && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                            if (kernel_buffer[13] == '\n' || kernel_buffer[10] == '\0') {

                                                                board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                                board[fin_y][fin_x][0] = kernel_buffer[3];
                                                                board[fin_y][fin_x][1] = kernel_buffer[4];
                                                                moves++;
                                                                strcpy(kernel_buffer, "OK");

                                                            }
                                                            else if (kernel_buffer[13] == 'y') {

                                                                if (kernel_buffer[14] == 'W' && (kernel_buffer[15] == 'Q' || kernel_buffer[15] == 'N' || kernel_buffer[15] == 'B' || kernel_buffer[15] == 'R')) {

                                                                    board[fin_y][fin_x][0] = kernel_buffer[3];
                                                                    board[fin_y][fin_x][1] = kernel_buffer[4];
                                                                    moves++;
                                                                    board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                                    strcpy(kernel_buffer, "OK");
                                                                    moves++;
                                                                
                                                                } else strcpy(kernel_buffer, "ILLMOV");

                                                            }
                                                            else strcpy(kernel_buffer, "UNKCMD");
                                                            
                                                        }
                                                        else if (player == 'B' && kernel_buffer[11] == 'W' && init_y - 1 == fin_y && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                            if (kernel_buffer[13] == '\n' || kernel_buffer[10] == '\0') {

                                                                board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                                board[fin_y][fin_x][0] = kernel_buffer[3];
                                                                board[fin_y][fin_x][1] = kernel_buffer[4];
                                                                moves++;
                                                                strcpy(kernel_buffer, "OK");

                                                            }
                                                            else if (kernel_buffer[13] == 'y') {

                                                                if (kernel_buffer[14] == 'B' && (kernel_buffer[15] == 'Q' || kernel_buffer[15] == 'N' || kernel_buffer[15] == 'B' || kernel_buffer[15] == 'R')) {

                                                                    board[fin_y][fin_x][0] = kernel_buffer[3];
                                                                    board[fin_y][fin_x][1] = kernel_buffer[4];
                                                                    moves++;
                                                                    board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                                    strcpy(kernel_buffer, "OK");
                                                                
                                                                } else strcpy(kernel_buffer, "ILLMOV");

                                                            }
                                                            else strcpy(kernel_buffer, "UNKCMD");

                                                        } else strcpy(kernel_buffer, "ILLMOV");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'y') {

                                                    if (init_x == fin_x) {

                                                        if (player == 'W' && init_y + 1 == fin_y && fin_y == 7 && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {
                                                            
                                                            if (kernel_buffer[11] == 'W' && (kernel_buffer[12] == 'Q' || kernel_buffer[12] == 'N' || kernel_buffer[12] == 'B' || kernel_buffer[12] == 'R')) {

                                                                board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                                board[fin_y][fin_x][0] = kernel_buffer[3];
                                                                board[fin_y][fin_x][1] = kernel_buffer[4];
                                                                moves++;
                                                                strcpy(kernel_buffer, "OK");
                                                            
                                                            }
                                                        
                                                        }
                                                        else if (player == 'B' && init_y - 1 == fin_y && fin_y == 0 && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                            board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                            board[fin_y][fin_x][0] = kernel_buffer[3];
                                                            board[fin_y][fin_x][1] = kernel_buffer[4];
                                                            moves++;
                                                            strcpy(kernel_buffer, "OK");

                                                        } else strcpy(kernel_buffer, "ILLMOV");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "ILLMOV");

                                            }

                                            else if (kernel_buffer[4] == 'R') {

                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if ((init_x == fin_x || init_y == fin_y) && player == kernel_buffer[3] && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if ((init_x == fin_x || init_y == fin_y) && player == kernel_buffer[3] && 
                                                        player == kernel_buffer[3] && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "UNKCMD");

                                            }

                                            else if (kernel_buffer[4] == 'N') {

                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if (((init_x + 1 == fin_x && init_y + 2 == fin_y) || (init_x - 1 == fin_x && init_y + 2 == fin_y) ||
                                                        (init_x + 1 == fin_x && init_y - 2 == fin_y) || (init_x - 1 == fin_x && init_y - 2 == fin_y) ||
                                                        (init_x + 2 == fin_x && init_y + 1 == fin_y) || (init_x - 2 == fin_x && init_y + 1 == fin_y) ||
                                                        (init_x + 2 == fin_x && init_y - 1 == fin_y) || (init_x - 2 == fin_x && init_y - 1 == fin_y)) && 
                                                        player == kernel_buffer[3] && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if (((init_x + 1 == fin_x && init_y + 2 == fin_y) || (init_x - 1 == fin_x && init_y + 2 == fin_y) ||
                                                        (init_x + 1 == fin_x && init_y - 2 == fin_y) || (init_x - 1 == fin_x && init_y - 2 == fin_y) ||
                                                        (init_x + 2 == fin_x && init_y + 1 == fin_y) || (init_x - 2 == fin_x && init_y + 1 == fin_y) ||
                                                        (init_x + 2 == fin_x && init_y - 1 == fin_y) || (init_x - 2 == fin_x && init_y - 1 == fin_y)) && 
                                                        player == kernel_buffer[3] && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {
                                                        
                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "UNKCMD");

                                            }


                                            else if (kernel_buffer[4] == 'B') {
                                                printk(KERN_INFO "testing: %c%c", board[fin_y][fin_x][0], board[init_y][init_x][1]);
                                                
                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if (((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) && 
                                                        player == kernel_buffer[3] && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if (((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) && 
                                                        player == kernel_buffer[3] && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "UNKCMD");

                                            }

                                            else if (kernel_buffer[4] == 'K') {

                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if (((((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) && (init_y - fin_y == 1 || init_y - fin_y == -1)) || 
                                                        (init_x == fin_x && (init_y + 1 == fin_y || init_y - 1 == fin_y)) || (init_y == fin_y && (init_x + 1 == fin_x || init_x - 1 == fin_x))) && 
                                                        player == kernel_buffer[3] && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if (((((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) && (init_y - fin_y == 1 || init_y - fin_y == -1)) || 
                                                        (init_x == fin_x && (init_y + 1 == fin_y || init_y - 1 == fin_y)) || (init_y == fin_y && (init_x + 1 == fin_x || init_x - 1 == fin_x))) &&
                                                        player == kernel_buffer[3] && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "UNKCMD");

                                            }

                                            else if (kernel_buffer[4] == 'Q') {

                                                if (kernel_buffer[10] == '\n' || kernel_buffer[10] == '\0') {

                                                    if ((((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) || 
                                                        (init_x == fin_x) || (init_y == fin_y)) && 
                                                        player == kernel_buffer[3] && (board[fin_y][fin_x][0] == '*' && board[fin_y][fin_x][1] == '*')) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else if (kernel_buffer[10] == 'x') {

                                                    if (kernel_buffer[11] != player && kernel_buffer[12] == 'K') {
                                                        over = 1;
                                                        strcpy(kernel_buffer, "MATE");
                                                    }

                                                    else if ((((init_x - fin_x == init_y - fin_y) || (init_x - fin_x == fin_y - init_y)) || 
                                                        (init_x == fin_x) || (init_y == fin_y)) &&
                                                        player == kernel_buffer[3] && board[fin_y][fin_x][0] != board[init_y][init_x][0]) {

                                                        board[fin_y][fin_x][0] = kernel_buffer[3];
                                                        board[fin_y][fin_x][1] = kernel_buffer[4];
                                                        moves++;
                                                        board[init_y][init_x][0] = '*'; board[init_y][init_x][1] = '*';
                                                        strcpy(kernel_buffer, "OK");

                                                    } else strcpy(kernel_buffer, "ILLMOV");

                                                }
                                                else strcpy(kernel_buffer, "UNKCMD");

                                            } else strcpy(kernel_buffer, "UNKCMD");

                                        } else strcpy(kernel_buffer, "ILLMOV");

                                    } else strcpy(kernel_buffer, "UNKCMD");

                                } 

                                else if (kernel_buffer[7] == ' ' || kernel_buffer[7] == 'W' || kernel_buffer[7] == 'B')
                                    strcpy(kernel_buffer, "INVFMT");

                                else strcpy(kernel_buffer, "UNKCMD");

                            } else strcpy(kernel_buffer, "UNKCMD");

                        } else strcpy(kernel_buffer, "UNKCMD");


                    } else strcpy(kernel_buffer, "OOT");   

                }
                else strcpy(kernel_buffer, "UNKCMD");

                if (!over) {
                    // Check if their is any check in opposite king:
                    for (king_x = 0, done = 0; king_x < 8; king_x++) {
                        for (king_y = 0; king_y < 8; king_y++) {
                            if (board[king_x][king_y][0] != player && board[king_x][king_y][1] == 'K') {
                                done = 1;
                                break;
                            }
                        }
                        if (done) break;
                    }

                    printk(KERN_INFO "King: %d %d", king_x, king_y);
                    printk(KERN_INFO "King: %c%c", board[king_x][king_y][0], board[king_x][king_y][1]);

                    // Check in diagonals

                    for (test_x = king_x-1, test_y = king_y-1; test_x >= 0 && test_y >= 0; test_x--, test_y--) {
                        if (board[test_x][test_y][0] != '*') {
                            if (board[test_x][test_y][0] == player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                check = 1;
                                left_dia = 1;
                                printk(KERN_ALERT "Bishop gave check");
                            }
                            break;
                        }
                    }

                    for (test_x = king_x-1, test_y = king_y+1; test_x >= 0 && test_y < 8; test_x--, test_y++) {
                        if (board[test_x][test_y][0] != '*') {
                            if (board[test_x][test_y][0] == player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                check = 1;
                                right_dia = 1;
                                printk(KERN_ALERT "Bishop gave check");
                            }
                            break;
                        }
                    }

                    for (test_x = king_x+1, test_y = king_y-1; test_x < 8 && test_y >= 0; test_x++, test_y--) {
                        if (board[test_x][test_y][0] != '*') {
                            if (board[test_x][test_y][0] == player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                check = 1;
                                right_dia = 1;
                                printk(KERN_ALERT "Bishop gave check");
                            }
                            break;
                        }
                    }

                    for (test_x = king_x+1, test_y = king_y+1; test_x < 8 && test_y < 8; test_x++, test_y++) {
                        if (board[test_x][test_y][0] != '*') {
                            if (board[test_x][test_y][0] == player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                check = 1;
                                left_dia = 1;
                                printk(KERN_ALERT "Bishop gave check");
                            }
                            break;
                        }
                    }

                    // Check same files

                    if (!check) {

                        for (test_x = king_x-1; test_x >= 0; test_x--)
                            if (board[test_x][king_y][0] != '*') {
                                if (board[test_x][king_y][0] == player && (board[test_x][king_y][1] == 'R' || board[test_x][king_y][1] == 'Q')) {
                                    check = 1;
                                    straight = 1;
                                    printk(KERN_ALERT "Rook gave check");
                                }
                                break;
                            }
                        
                        for (test_x = king_x+1; test_x < 8; test_x++)
                            if (board[test_x][king_y][0] != '*') {
                                if (board[test_x][king_y][0] == player && (board[test_x][king_y][1] == 'R' || board[test_x][king_y][1] == 'Q')) {
                                    check = 1;
                                    straight = 1;
                                    printk(KERN_ALERT "Rook gave check");
                                }
                                break;
                            }

                        for (test_y = king_y-1; test_y >= 0; test_y--)
                            if (board[king_x][test_y][0] != '*') {
                                if (board[king_x][test_y][0] == player && (board[king_x][test_y][1] == 'B' || board[king_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    side = 1;
                                    printk(KERN_ALERT "Rook gave check");
                                }
                                break;
                            }

                        for (test_y = king_y+1; test_y < 8; test_y++)
                            if (board[king_x][test_y][0] != '*') {
                                if (board[king_x][test_y][0] == player && (board[king_x][test_y][1] == 'B' || board[king_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    side = 1;
                                    printk(KERN_ALERT "Rook gave check");
                                }
                                break;
                            }

                        // Check by knight
                        if (!check) {
                            if (king_x + 2 < 8 && king_y + 1 < 8 && board[king_x + 2][king_y + 1][0] == player && board[king_x + 2][king_y + 1][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x + 2 < 8 && king_y - 1 >= 0 && board[king_x + 2][king_y - 1][0] == player && board[king_x + 2][king_y - 1][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x - 2 >= 0 && king_y + 1 < 8 && board[king_x - 2][king_y + 1][0] == player && board[king_x - 2][king_y + 1][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x - 2 >= 0 && king_y - 1 >= 0 && board[king_x - 2][king_y - 1][0] == player && board[king_x - 2][king_y - 1][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x + 1 < 8 && king_y + 2 < 8 && board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x + 1 < 8 && king_y - 2 >= 0 && board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x - 1 >= 0 && king_y + 2 < 8 && board[king_x - 1][king_y + 2][0] == player && board[king_x - 1][king_y + 2][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }
                            else if (king_x - 1 >= 0 && king_y - 2 >= 0 && board[king_x - 1][king_y - 2][0] == player && board[king_x - 1][king_y - 2][1] == 'N') {
                                printk(KERN_INFO "Knight gave check");
                                check = 1;
                            }

                            // Check by pawns
                            if (!check) {

                                if ((board[king_x - 1][king_y - 1][0] == player && board[king_x - 1][king_y - 1][1] == 'P') || 
                                    (board[king_x - 1][king_y + 1][0] == player && board[king_x - 1][king_y + 1][1] == 'P')) {
                                    printk(KERN_INFO "Check by pawn");
                                    check = 1;
                                }

                            }

                        }
                    
                    }

                    if (check) 
                        strcpy(kernel_buffer, "CHECK");

                        // check checkmates in the board
                        // if ((left_dia || board[king_x - 1][king_y - 1][0] != player || board[king_x + 1][king_y + 1][0] != player) && 
                        //     (right_dia || board[king_x - 1][king_y + 1][0] != player || board[king_x + 1][king_y - 1][0] != player) &&
                        //     (straight || board[king_x + 1][king_y][0] != player || board[king_x - 1][king_y][0] != player) &&
                        //     (side || board[king_x][king_y + 1][0] != player || board[king_x][king_y - 1][0] != player)) 
                        //         over = 1;

                        // Check for blocking piece case:
                        // if (board[king_x - 1][king_y - 1][0] != player && board[king_x + 1][king_y + 1][0] != player &&
                        //     board[king_x - 1][king_y + 1][0] != player && board[king_x + 1][king_y - 1][0] != player &&
                        //     board[king_x - 1][king_y][0] != player && board[king_x + 1][king_y][0] != player &&
                        //     board[king_x][king_y - 1][0] != player && board[king_x][king_y + 1][0] != player) {

                        //     if (board[king_x - 2][king_y - 1][0] == player && board[king_x - 2][king_y - 1][1] == 'N') {
                        //         if ((board[king_x - 2][king_y + 1][0] == player && board[king_x - 2][king_y + 1][1] == 'N') || 
                        //             (board[king_x + 2][king_y - 1][0] == player && board[king_x + 2][king_y - 1][1] == 'N') ||
                        //             (board[king_x + 2][king_y + 1][0] == player && board[king_x + 2][king_y + 1][1] == 'N') ||
                        //             (board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') || 
                        //             (board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x - 2][king_y + 1][0] == player && board[king_x - 2][king_y + 1][1] == 'N') {
                        //         if ((board[king_x + 2][king_y - 1][0] == player && board[king_x + 2][king_y - 1][1] == 'N') ||
                        //             (board[king_x + 2][king_y + 1][0] == player && board[king_x + 2][king_y + 1][1] == 'N') ||
                        //             (board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') || 
                        //             (board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x + 2][king_y - 1][0] == player && board[king_x + 2][king_y - 1][1] == 'N') {
                        //         if ((board[king_x + 2][king_y + 1][0] == player && board[king_x + 2][king_y + 1][1] == 'N') ||
                        //             (board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') || 
                        //             (board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x + 2][king_y + 1][0] == player && board[king_x + 2][king_y + 1][1] == 'N') {
                        //         if ((board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') || 
                        //             (board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x + 1][king_y - 2][0] == player && board[king_x + 1][king_y - 2][1] == 'N') {
                        //         if ((board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x + 1][king_y + 2][0] == player && board[king_x + 1][king_y + 2][1] == 'N') {
                        //         if ((board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') ||
                        //             (board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }
                        //     else if (board[king_x - 1][king_y - 2][0] == player && board[king_x - 2][king_y - 2][1] == 'N') {
                        //         if ((board[king_x - 1][king_y + 2][0] == player && board[king_x - 2][king_y + 2][1] == 'N'))
                        //             over = 1;
                        //         else over = 0;
                        //     }

                        // }
                }

            }
            else if (kernel_buffer[1] == '3') {
                if ((player == 'W' && moves%2 == 1) || (player == 'B' && moves%2 == 0)) {
                    moves++;
                    for (i = 0, done = 0; i < 8; i++) {
                        for (j = 0; j < 8; j++) {
                            if (board[i][j][0] != player) {

                                if (board[i][j][1] == 'P') {

                                    if (player == 'W' && board[i-1][j][0] == '*') {
                                        board[i-1][j][0] = 'B', board[i-1][j][1] = 'P';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        done = 1;
                                        strcpy(kernel_buffer, "OK");
                                        break;
                                    }
                                    else if (player == 'B' && board[i+1][j][0] == '*') {
                                        board[i+1][j][0] = 'W', board[i+1][j][1] = 'P';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                                else if (board[i][j][1] == 'N') {

                                    if (i-2 >= 0 && j + 1 < 8 && board[i - 2][j + 1][0] == '*') {
                                        board[i-2][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-2][j+1][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-2 >= 0 && j-1 >= 0 && board[i - 2][j - 1][0] == '*') {
                                        board[i-2][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-2][j-1][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+2 < 8 && j-1 >= 0 && board[i + 2][j - 1][0] == '*') {
                                        board[i+2][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+2][j-1][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+2 >= 0 && j + 1 < 8 && board[i + 2][j + 1][0] == '*') {
                                        board[i+2][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+2][j+1][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+1 < 8 && j-2 >= 0 && board[i + 1][j - 2][0] == '*') {
                                        board[i+1][j-2][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j-2][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+1 < 8 && j + 2 < 8 && board[i + 1][j + 2][0] == '*') {
                                        board[i+1][j+2][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j+2][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-1 >= 0 && j-2 >= 0 && board[i - 1][j - 2][0] == '*') {
                                        board[i-1][j-2][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j-2][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-1 >= 0 && j + 2 < 8 && board[i - 1][j + 2][0] == '*') {
                                        board[i-1][j+2][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j+2][1] = 'N';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                                else if (board[i][j][1] == 'Q') {

                                    if (i+1 < 8 && j+1 < 8 && board[i+1][j+1][0] == '*') {
                                        board[i+1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j+1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i+1 < 8 && j-1 >= 0 && board[i+1][j-1][0] == '*') {
                                        board[i+1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j-1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if(i-1 >= 0 && j+1 < 8 && board[i-1][j+1][0] == '*') {
                                        board[i-1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j+1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-1 >= 0 && j-1 >= 0 && board[i-1][j-1][0] == '*') {
                                        board[i-1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j-1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+1 < 8 && board[i+1][j][0] == '*') {
                                        board[i+1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i-1 >= 0 && board[i-1][j][0] == '*') {
                                        board[i-1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j+1 < 8 && board[i][j+1][0] == '*') {
                                        board[i][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j+1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j-1 >= 0 && board[i][j-1][0] == '*') {
                                        board[i][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j-1][1] = 'Q';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                                else if (board[i][j][1] == 'K') {

                                    if (i+1 < 8 && j+1 < 8 && board[i+1][j+1][0] == '*') {
                                        board[i+1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i+1 < 8 && j-1 >= 0 && board[i+1][j-1][0] == '*') {
                                        board[i+1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if(i-1 >= 0 && j+1 < 8 && board[i-1][j+1][0] == '*') {
                                        board[i-1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-1 >= 0 && j-1 >= 0 && board[i-1][j-1][0] == '*') {
                                        board[i-1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i+1 < 8 && board[i+1][j][0] == '*') {
                                        board[i+1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i-1 >= 0 && board[i-1][j][0] == '*') {
                                        board[i-1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j+1 < 8 && board[i][j+1][0] == '*') {
                                        board[i][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j-1 >= 0 && board[i][j-1][0] == '*') {
                                        board[i][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                                else if (board[i][j][1] == 'R') {

                                    if (i+1 < 8 && board[i+1][j][0] == '*') {
                                        board[i+1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i-1 >= 0 && board[i-1][j][0] == '*') {
                                        board[i-1][j][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j+1 < 8 && board[i][j+1][0] == '*') {
                                        board[i][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (j-1 >= 0 && board[i][j-1][0] == '*') {
                                        board[i][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                                else if (board[i][j][1] == 'B') {

                                    if (i+1 < 8 && j+1 < 8 && board[i+1][j+1][0] == '*') {
                                        board[i+1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    } 
                                    else if (i+1 < 8 && j-1 >= 0 && board[i+1][j-1][0] == '*') {
                                        board[i+1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i+1][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if(i-1 >= 0 && j+1 < 8 && board[i-1][j+1][0] == '*') {
                                        board[i-1][j+1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j+1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }
                                    else if (i-1 >= 0 && j-1 >= 0 && board[i-1][j-1][0] == '*') {
                                        board[i-1][j-1][0] = (player == 'W' ? 'B' : 'W');
                                        board[i-1][j-1][1] = 'K';
                                        board[i][j][0] = '*', board[i][j][1] = '*';
                                        strcpy(kernel_buffer, "OK");
                                        done = 1;
                                        break;
                                    }

                                }

                            }
                        }
                        if (done) break;
                    }

                    // Verify if player is at check

                    if (!over) {
                        // Check if their is any check in player's king:
                        for (king_x = 0, done = 0; king_x < 8; king_x++) {
                            for (king_y = 0; king_y < 8; king_y++) {
                                if (board[king_x][king_y][0] == player && board[king_x][king_y][1] == 'K') {
                                    done = 1;
                                    break;
                                }
                            }
                            if (done) break;
                        }

                        printk(KERN_INFO "King: %d %d", king_x, king_y);
                        printk(KERN_INFO "King: %c%c", board[king_x][king_y][0], board[king_x][king_y][1]);

                        // Check in diagonals

                        for (test_x = king_x-1, test_y = king_y-1; test_x >= 0 && test_y >= 0; test_x--, test_y--) {
                            if (board[test_x][test_y][0] != '*') {
                                if (board[test_x][test_y][0] != player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    left_dia = 1;
                                    printk(KERN_ALERT "Bishop gave check");
                                }
                                break;
                            }
                        }

                        for (test_x = king_x-1, test_y = king_y+1; test_x >= 0 && test_y < 8; test_x--, test_y++) {
                            if (board[test_x][test_y][0] != '*') {
                                if (board[test_x][test_y][0] != player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    right_dia = 1;
                                    printk(KERN_ALERT "Bishop gave check");
                                }
                                break;
                            }
                        }

                        for (test_x = king_x+1, test_y = king_y-1; test_x < 8 && test_y >= 0; test_x++, test_y--) {
                            if (board[test_x][test_y][0] != '*') {
                                if (board[test_x][test_y][0] != player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    right_dia = 1;
                                    printk(KERN_ALERT "Bishop gave check");
                                }
                                break;
                            }
                        }

                        for (test_x = king_x+1, test_y = king_y+1; test_x < 8 && test_y < 8; test_x++, test_y++) {
                            if (board[test_x][test_y][0] != '*') {
                                if (board[test_x][test_y][0] != player && (board[test_x][test_y][1] == 'B' || board[test_x][test_y][1] == 'Q')) {
                                    check = 1;
                                    left_dia = 1;
                                    printk(KERN_ALERT "Bishop gave check");
                                }
                                break;
                            }
                        }

                        // Check same files

                        if (!check) {

                            for (test_x = king_x-1; test_x >= 0; test_x--)
                                if (board[test_x][king_y][0] != '*') {
                                    if (board[test_x][king_y][0] != player && (board[test_x][king_y][1] == 'R' || board[test_x][king_y][1] == 'Q')) {
                                        check = 1;
                                        straight = 1;
                                        printk(KERN_ALERT "Rook gave check");
                                    }
                                    break;
                                }
                            
                            for (test_x = king_x+1; test_x < 8; test_x++)
                                if (board[test_x][king_y][0] != '*') {
                                    if (board[test_x][king_y][0] != player && (board[test_x][king_y][1] == 'R' || board[test_x][king_y][1] == 'Q')) {
                                        check = 1;
                                        straight = 1;
                                        printk(KERN_ALERT "Rook gave check");
                                    }
                                    break;
                                }

                            for (test_y = king_y-1; test_y >= 0; test_y--)
                                if (board[king_x][test_y][0] != '*') {
                                    if (board[king_x][test_y][0] != player && (board[king_x][test_y][1] == 'B' || board[king_x][test_y][1] == 'Q')) {
                                        check = 1;
                                        side = 1;
                                        printk(KERN_ALERT "Rook gave check");
                                    }
                                    break;
                                }

                            for (test_y = king_y+1; test_y < 8; test_y++)
                                if (board[king_x][test_y][0] != '*') {
                                    if (board[king_x][test_y][0] != player && (board[king_x][test_y][1] == 'B' || board[king_x][test_y][1] == 'Q')) {
                                        check = 1;
                                        side = 1;
                                        printk(KERN_ALERT "Rook gave check");
                                    }
                                    break;
                                }

                            // Check by knight
                            if (!check) {
                                if (king_x + 2 < 8 && king_y + 1 < 8 && board[king_x + 2][king_y + 1][0] != player && board[king_x + 2][king_y + 1][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x + 2 < 8 && king_y - 1 >= 0 && board[king_x + 2][king_y - 1][0] != player && board[king_x + 2][king_y - 1][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x - 2 >= 0 && king_y + 1 < 8 && board[king_x - 2][king_y + 1][0] != player && board[king_x - 2][king_y + 1][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x - 2 >= 0 && king_y - 1 >= 0 && board[king_x - 2][king_y - 1][0] != player && board[king_x - 2][king_y - 1][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x + 1 < 8 && king_y + 2 < 8 && board[king_x + 1][king_y + 2][0] != player && board[king_x + 1][king_y + 2][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x + 1 < 8 && king_y - 2 >= 0 && board[king_x + 1][king_y - 2][0] != player && board[king_x + 1][king_y - 2][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x - 1 >= 0 && king_y + 2 < 8 && board[king_x - 1][king_y + 2][0] != player && board[king_x - 1][king_y + 2][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }
                                else if (king_x - 1 >= 0 && king_y - 2 >= 0 && board[king_x - 1][king_y - 2][0] != player && board[king_x - 1][king_y - 2][1] == 'N') {
                                    printk(KERN_INFO "Knight gave check");
                                    check = 1;
                                }

                                // Check by pawns
                                if (!check) {

                                    if ((board[king_x - 1][king_y - 1][0] != player && board[king_x - 1][king_y - 1][1] == 'P') || 
                                        (board[king_x - 1][king_y + 1][0] != player && board[king_x - 1][king_y + 1][1] == 'P')) {
                                        printk(KERN_INFO "Check by pawn");
                                        check = 1;
                                    }

                                }

                            }
                        
                        }

                        if (check) 
                            strcpy(kernel_buffer, "CHECK");
                    
                    } 

                }
                else strcpy(kernel_buffer, "OOT");

            } 
            else if (kernel_buffer[1] == '4') {
                over = 1;
                strcpy(kernel_buffer, "OK");
            }
            else if (kernel_buffer[1] == ' ') 
                strcpy(kernel_buffer, "INVFMT");
            
            else strcpy(kernel_buffer, "UNKCMD");

        }
        else strcpy(kernel_buffer, "NOGAME");
    }
    else strcpy(kernel_buffer, "UNKCMD");
    
    printk(KERN_NOTICE "KERNEL: %s\n", kernel_buffer);
    printk(KERN_NOTICE "PLAYER: %c\n", player);
    return length;
}


static int test_release (struct inode * pinode, struct file * pfile) {
    kfree(kernel_buffer);
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    return 0;
}

struct file_operations test_file_operation = {
    .owner   = THIS_MODULE,
    .open    = test_open,
    .read    = test_read,
    .write   = test_write,
    .release = test_release
};


static int __init test_init(void) {
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);

    /* Allocating major number */
    if (alloc_chrdev_region(&dev, 0, 1, game) < 0) {
        printk(KERN_ALERT "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&test_cdev, &test_file_operation);

    /* Adding character device to the system */
    if (cdev_add(&test_cdev, dev, 1) < 0) {
        // error message
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }

    /* Creating structure class */
    if ((dev_class = class_create(THIS_MODULE, game)) == NULL) {
        //error meesage
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device (file) */
    if (device_create(dev_class, NULL, dev, NULL, game) == NULL) {
        //error message
        printk(KERN_INFO "Cannot create the Device\n");
        goto r_device;
    }
    printk(KERN_INFO "Inserting device driver done...!!!");

    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}


static void __exit test_exit(void) {
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&test_cdev);
    printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
    unregister_chrdev_region(dev, 1);
}


module_init(test_init);
module_exit(test_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("CHESS LINUX KERNEL");
MODULE_DESCRIPTION("CHESS KERNEL MODULE TO PLAY SINGLE PLAYER CHESS");
MODULE_VERSION("2.0");