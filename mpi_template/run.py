from __future__ import print_function
import time
import spynnaker_mpi
import spynnaker_acp
import spynnaker_acp.types
import subprocess

subprocess.call("ybug spin5.polito.it -bmp bmp-spin5.polito.it < reboot-spin5.txt", shell=True)
print("ACP:", spynnaker_acp.__path__)
print("MPI:", spynnaker_mpi.__path__)

runtime = spynnaker_mpi.MPIRuntime.create("spin5.polito.it", "bmp-spin5.polito.it")
context = spynnaker_mpi.MPIContext(runtime, 7, 16)
app = spynnaker_mpi.MPIApp(runtime, "mpi_template.aplx")

nodes = context.cores_number
print("Nodes: {}".format(nodes))
if nodes > 20:
    (x,y,p) = context.get_processor(20)
    print("Node {} in chip {}".format(20, (x,y,p)))

me1000 = spynnaker_acp.RemoteMemoryEntity(runtime, 1000)
me1001 = spynnaker_acp.RemoteMemoryEntity(runtime, 1001)
uint8_acp = spynnaker_acp.types.ACPIntegerType(8, False)
uint32_acp = spynnaker_acp.types.ACPIntegerType(32, False)
uint8_acp.value = 0
uint32_acp.value = 0

# Init
app.init(context)

# Start
app.run()

# --- Program

if nodes > 20:
    # Sync SpiNNaker application
    me1001.update(x, y, p, uint8_acp, sync=True, verbose=True)
    # Read ME with memory pointer
    me1000.read(x, y, p, uint32_acp, verbose=True)
    print("Received: 0x{:08x}".format(uint32_acp.value))
    # Sync SpiNNaker application
    me1001.update(x, y, p, uint8_acp, sync=True, verbose=True)

# ---

# Wait Stop
buffers = app.stop(get_buffers=True)
print("DONE")

# Buffers elaboration
strings = spynnaker_acp.Utils.get_io_buffers(buffers)
count = 0
count_max = 100
for string in strings:
    print(string)
    count += 1
    if count == count_max:
        break

#print("Received: 0x{:08x}".format(uint31_acp.value))
