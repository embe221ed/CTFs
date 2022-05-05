target remote localhost:1234
file contestanttrack.ko
directory /home/h4ck1t/Desktop/CTFs/DamCTF/cuttlefish-game/cuttlefish-game/
# tracker_get_game
# b *0xffffffffc00007a4
# tracker_set_notes
# b *0xffffffffc00008bb
# tracker_release_game
# b *0xffffffffc0000960
# tracker_release_game interesting point where in RAX is address of the game_t
b *0xffffffffc00009d2
# tracker_execute_game
# b *0xffffffffc0000a32
# tracker_mark
# b *0xffffffffc0000bf8
# tracker_status
# b *0xffffffffc0000d30
# tracker_create_serial
# b *0xffffffffc0000f08
# tracker_alloc_game
# b *0xffffffffc0000dfe
# tracker_search_serial
# b *0xffffffffc0000fb7
# tracker_cleanup
# b *0xffffffffc000103b
