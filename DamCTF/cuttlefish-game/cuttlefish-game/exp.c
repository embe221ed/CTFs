#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define GET_GAME 0x1337
#define SET_NOTES 0x1338
#define RELEASE_GAME 0x1339
#define EXE_GAME 0x133a
#define MARK_ALIVE 0x133b
#define MARK_DEAD 0x133c
#define GET_STATUS 0x133d

#define MAX_GAMES 5


typedef struct
{
    char buf[135];
    int serial;
    long gamenum;
    long args;
    long contestant;
} request_t;


int main(void) {
    int fd, result;
    fd = open("/proc/tracker", 2);
    if (fd < 0) {
        printf("Error opening proc...\n");
        return 0;
    }
    request_t request = {
        .buf="aaaa",
        .serial=0,
        .gamenum=0,
        .args=0,
        .contestant=0
    };
    result = ioctl(fd, GET_GAME, request);
    printf("result: %d\n", result);
    close(fd);
}
