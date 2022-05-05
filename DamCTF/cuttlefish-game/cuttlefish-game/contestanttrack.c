#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/list.h>

#define GET_GAME 0x1337
#define SET_NOTES 0x1338
#define RELEASE_GAME 0x1339
#define EXE_GAME 0x133a
#define MARK_ALIVE 0x133b
#define MARK_DEAD 0x133c
#define GET_STATUS 0x133d

#define MAX_GAMES 5

MODULE_AUTHOR("Aqcurate");
MODULE_DESCRIPTION("Contestant Tracker");
MODULE_LICENSE("GPL"); 

typedef struct __attribute__((packed))
{
    char notes[135];
    uint8_t contestants[57];
} state_t;

typedef struct __attribute__((packed))
{
    unsigned long (*execute_game1) (unsigned long);
    unsigned long (*execute_game2) (unsigned long);
    unsigned long (*execute_game3) (unsigned long);
    unsigned long (*execute_game4) (unsigned long);
    unsigned long (*execute_game5) (unsigned long);
    unsigned long (*execute_game6) (unsigned long);
    unsigned long (*fallthrough_game) (unsigned long);
} gameops_t;

typedef struct
{
    refcount_t count;
    char name[16];
    char desc[135];
    state_t* state;
    gameops_t* ops;
    struct list_head glist;
} game_t;

typedef struct __attribute__((packed))
{
    uint32_t serial;
    game_t* game;
    struct list_head slist;
} serial_t;

typedef struct
{
    char buf[135];
    uint32_t serial;
    long gamenum;
    long args;
    long contestant;
} request_t;

static long tracker_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
// 0xffffffffc00007a4
static long tracker_get_game(char* name);
// 0xffffffffc00008bb
static long tracker_set_notes(uint32_t serial, char* notes);
// 0xffffffffc0000960
static long tracker_release_game(uint32_t serial);
// 0xffffffffc0000a32
static long tracker_execute_game(uint32_t serial, long gamenum, long args);
// 0xffffffffc0000bf8
static long tracker_mark(uint32_t serial, uint64_t contestant, uint32_t status);
// 0xffffffffc0000d30
static long tracker_status(uint32_t serial, int64_t contestant);
// 0xffffffffc0000dfe
static game_t* tracker_alloc_game(char* name);
// 0xffffffffc0000f08
static long tracker_create_serial(game_t* game);
// 0xffffffffc0000fb7
static serial_t* tracker_search_serial(uint32_t serial);
// 0xffffffffc0001026
static unsigned long dummy(unsigned long args);

static DEFINE_MUTEX(game_lock);
static LIST_HEAD(games);
static LIST_HEAD(serials);

static gameops_t* dummy_gops; 

static long tracker_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    request_t r;

    /*
     * ffffffff81039f30 t is_copy_from_user
     * ffffffff8117ae50 T copy_from_user_nofault
     * ffffffff813f30f0 t kfifo_copy_from_user
     * ffffffff813f5460 T _copy_from_user
     * ffffffff81439090 T csum_and_copy_from_user
     * ffffffff8143b460 T copy_from_user_nmi
     * ffffffff81860fe0 T copy_from_user_toio
     * ffffffff81914660 t ethtool_rxnfc_copy_from_user
     */
    if (copy_from_user((void *) &r, (void *) arg, sizeof(request_t)))
    {   
        return -1; 
    }

    if (cmd == GET_GAME)
    {
        return tracker_get_game(r.buf);
    }
    else if (cmd == SET_NOTES)
    {
        return tracker_set_notes(r.serial, r.buf);
    }
    else if (cmd == RELEASE_GAME)
    {
        return tracker_release_game(r.serial);
    }
    else if (cmd == EXE_GAME)
    {
        return tracker_execute_game(r.serial, r.gamenum, r.args);
    }
    else if (cmd == MARK_ALIVE)
    {
        return tracker_mark(r.serial, r.contestant, 1);
    } 
    else if (cmd == MARK_DEAD) 
    {
        return tracker_mark(r.serial, r.contestant, 0);
    } 
    else if (cmd == GET_STATUS) 
    {
        return tracker_status(r.serial, r.contestant); 
    }

    return -1;
}; 

// only place where something different than 0, 1 and -1 is returned
static long tracker_get_game(char* name)
{
    game_t* g;
    long ret = -1;

    mutex_lock(&game_lock);

    list_for_each_entry(g, &games, glist)
    {
        /*
         * ffffffff81430130 T strcmp
         */
        if (!strcmp(g->name, name))
        {
            // new serial is being created
            ret = tracker_create_serial(g);
            // there are no checks regarding too big value
            /*
             * ffffffffc0000394 t __refcount_inc	[contestanttrack]
             * ffffffffc00003bd t refcount_inc	[contestanttrack]
             */
            refcount_inc(&(g->count));

            mutex_unlock(&game_lock);
            return ret;
        }
    }

    g = tracker_alloc_game(name);
    /*
     * ffffffffc000005d t __list_add	[contestanttrack]
     * ffffffffc00000cb t list_add	[contestanttrack]
     */
    list_add(&g->glist, &games);
    ret = tracker_create_serial(g);

    mutex_unlock(&game_lock);
    return ret;
}

static long tracker_set_notes(uint32_t serial, char* notes) {
    serial_t* s;
    state_t* state;
    long ret = -1;
    mutex_lock(&game_lock);

    s = tracker_search_serial(serial);
    if (!s) 
    {
        mutex_unlock(&game_lock);
        return ret;
    }
    state = s->game->state;
    /*
     * ffffffff81b910b0 W memcpy
     */
    // this is probably good place for arbitrary write, I would have to make
    // sure the state->notes address is pointing to the place I want to write
    memcpy(state->notes, notes, sizeof(state->notes));
    state->notes[sizeof(state->notes)-1] = 0;
    ret = 0;

    mutex_unlock(&game_lock);
    return ret;
}

static long tracker_release_game(uint32_t serial)
{
    serial_t* s;
    game_t* g;
    long ret = -1;
    mutex_lock(&game_lock);

    s = tracker_search_serial(serial);
    if (!s) 
    {
        mutex_unlock(&game_lock);
        return ret;
    }

    // not sure if serial can't be found even if it's deleted
    /*
     * ffffffffc0000162 t list_del	[contestanttrack]
     */
    list_del(&s->slist);
    /*
     * ffffffff811e3240 T kfree
     */
    kfree(s);

    g = s->game;
    // this condition is true if refcount == 0
    if (refcount_dec_and_test(&g->count))
    {
        /*
         * ffffffffc0000162 t list_del	[contestanttrack]
         */
        list_del(&g->glist);
        /*
         * ffffffff811e3240 T kfree
         */
        kfree(g->state);
        kfree(g);
    }

    mutex_unlock(&game_lock);
    return ret;
}

static long tracker_execute_game(uint32_t serial, long gamenum, long args)
{
    serial_t* s;
    game_t* g;
    long ret = -1;
    mutex_lock(&game_lock);

    s = tracker_search_serial(serial);
    if (!s) 
    {
        mutex_unlock(&game_lock);
        return ret;
    }

    g = s->game;

    switch (gamenum)
    {
        case 1:    
            ret = g->ops->execute_game1(args);
            break;
        case 2:
            ret = g->ops->execute_game2(args);
            break;
        case 3:
            ret = g->ops->execute_game3(args);
            break;
        case 4:
            ret = g->ops->execute_game4(args);
            break;
        case 5:
            ret = g->ops->execute_game5(args);
            break;
        case 6:
            ret = g->ops->execute_game6(args);
            break;
        default:
            ret = g->ops->fallthrough_game(args);
            break;
    }

    mutex_unlock(&game_lock);
    return ret;
}

static long tracker_mark(uint32_t serial, uint64_t contestant, uint32_t status)
{
    serial_t* s;
    state_t* state;
    uint64_t num_contestants;
    long ret = -1;
    mutex_lock(&game_lock);

    s = tracker_search_serial(serial);
    if (!s) 
    {
        mutex_unlock(&game_lock);
        return ret;
    }
    state = s->game->state;

    // here is probably wrong as contestants[num_contestants] if OOB
    // probably negative index will work
    num_contestants = sizeof(state->contestants)<<3;
    if (contestant <= num_contestants)
    {
        if (status)
        {
            state->contestants[contestant>>3] |= 1 << (contestant&7);
        } 
        else 
        {
            state->contestants[contestant>>3] &= ~(1 << (contestant&7));
        }
        ret = 0;
    }
    mutex_unlock(&game_lock);
    return ret;
}

static long tracker_status(uint32_t serial, int64_t contestant) {
    serial_t* s;
    state_t* state;
    long ret = -1;
    int64_t num_contestants;
    mutex_lock(&game_lock);

    s = tracker_search_serial(serial);
    if (!s) 
    {
        mutex_unlock(&game_lock);
        return ret;
    }
    
    num_contestants = sizeof(state->contestants)<<3;
    if (num_contestants >= contestant)
    {
        state = s->game->state;
        ret = (state->contestants[contestant>>3] & (1 << (contestant&7))) >> (contestant&7);
    }

    mutex_unlock(&game_lock);
    return ret;
}

static game_t* tracker_alloc_game(char* name)
{
    game_t* game;
    state_t* state;

    game = (game_t *) kmalloc(sizeof(game_t), GFP_KERNEL);
    strlcpy(game->name, name, sizeof(game->name));
    strlcpy(game->desc, "Hundreds of flag-strapped players accept.", sizeof(game->desc));
    refcount_set(&(game->count), 1);

    state = (state_t *) kmalloc(sizeof(state_t), GFP_KERNEL);
    strlcpy(state->notes, "Default note.", sizeof(state->notes));
    memset(state->contestants, '\xff', sizeof(state->contestants)); 

    game->state = state;
	game->ops = dummy_gops;

    return game;
}

static long tracker_create_serial(game_t* g) 
{
    serial_t* serial = (serial_t *) kmalloc(sizeof(serial_t), GFP_KERNEL);
    get_random_bytes(&serial->serial, sizeof(serial->serial));
    serial->serial >>= 1;
    serial->game = g;

    for (;;) 
    {
        if (tracker_search_serial(serial->serial))
        {
            serial->serial += 1;
        } 
        else 
        {
            break;
        }
    }

    list_add(&serial->slist, &serials);
    return serial->serial;
}

static serial_t* tracker_search_serial(uint32_t serial)
{
    serial_t* s;
    list_for_each_entry(s, &serials, slist)
    {
        if (s->serial == serial)
        {
            return s;
        }
    }

    return NULL;
}

/*
 * ffffffffc0001026 t dummy	[contestanttrack]
 */
static unsigned long dummy(unsigned long args)
{
	return args+1;
}

static struct proc_ops fops = {
  	.proc_ioctl = tracker_ioctl,
};

struct proc_dir_entry *proc_entry = NULL;

static int __init tracker_init(void)
{
    mutex_init(&game_lock);

    // TODO: Add real games...
    dummy_gops = (gameops_t *) kmalloc(sizeof(gameops_t), GFP_KERNEL);
    dummy_gops->execute_game1 = dummy;
    /*
     * ffffffff810dc690 T clock_t_to_jiffies
     */
    dummy_gops->execute_game2 = clock_t_to_jiffies;
    dummy_gops->execute_game3 = dummy;
    dummy_gops->execute_game4 = dummy;
    dummy_gops->execute_game5 = dummy;
    dummy_gops->execute_game6 = dummy;
  	dummy_gops->fallthrough_game = dummy;

    proc_entry = proc_create("tracker", 0666, NULL, &fops);
    printk(KERN_INFO "Loaded Contestant Tracking Module...\n");
  	return 0;
}

static void __exit tracker_cleanup(void)
{
    mutex_destroy(&game_lock);
	if (proc_entry) proc_remove(proc_entry);
    printk(KERN_INFO "Removed Contestant Tracking Module...\n");
}

module_init(tracker_init);
module_exit(tracker_cleanup);
