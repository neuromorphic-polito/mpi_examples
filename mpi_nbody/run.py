from __future__ import print_function
import time
import spynnaker_mpi
import spynnaker_acp

print("ACP:", spynnaker_acp.__path__)
print("MPI:", spynnaker_mpi.__path__)

runtime = spynnaker_mpi.MPIRuntime.create("spin5.polito.it", "bmp-spin5.polito.it")
context = spynnaker_mpi.MPIContext(runtime, 4, 4)
app = spynnaker_mpi.MPIApp(runtime, "mpi_nbody.aplx")

# Init
app.init(context)
time.sleep(1)

# Start
app.run()
time.sleep(1)

# Stop
buffers = app.stop(get_buffers=True)

# Buffers elaboration
strings = spynnaker_acp.Utils.get_io_buffers(buffers)

for string in strings:
    print(string)
