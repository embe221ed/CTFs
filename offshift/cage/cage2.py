import angr
import claripy

proj = angr.Project('./cage2')
simgr = proj.factory.simgr()
simgr.explore(find=lambda s: b"wrong" not in s.posix.dumps(1))
s = simgr.found[0]
print(s.posix.dumps(0))
