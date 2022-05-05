import os
import json
import r2pipe


def print_registers():
    j = r2.cmd("drj")
    d = json.loads(j)
    for key in d:
        d[key] = hex(d[key])
    print(json.dumps(d, indent=2))


r2 = r2pipe.open("./masochistic_snake")
r2.cmd("e dbg.profile=profile.rr2")
r2.cmd("ood")
base = 0x555555554000
breakpoint = base + 0x1560
r2.cmd("db main")
print(r2.cmd("db {}".format(hex(breakpoint))))
r2.cmd("dc")
print_registers()
r2.cmd("dc")
print_registers()
print(r2.cmd("x/16xg @0x7fffffff9180"))
