from __future__ import print_function
import os
import time
import click
import spynnaker_mpi
import spynnaker_acp
import subprocess



def do(rings, cores, n_chunks, n_patterns):
	CHUNK_BYTES = 256
	PATTERN_BYTES = 32

	chips = [1, 4, 9, 16, 24, 32, 40, 48]
	#rings = 0
	#cores = 2
	#n_chunks = 1000
	#n_patterns = 120

	mpi_workers = cores*chips[rings]-1

	#if mpi_workers > n_patterns:
	#	exit()

	## ----- GET BINARY FILES FOR FED ----- ##
	data_path = "./ecoli"
	with open(os.path.join(data_path, 'ecoli_text.bin'), 'rb') as text_bin:
		text_bytes = bytearray(text_bin.read()[:(n_chunks*CHUNK_BYTES)])
	print("Read " + str(len(text_bytes)) + " bytes from text binary")

	with open(os.path.join(data_path, 'ecoli_32bp_patterns.bin'), 'rb') as patterns_bin:
		pattern_bytes = bytearray(patterns_bin.read()[:(n_patterns*PATTERN_BYTES*mpi_workers)])
	print("Read " + str(len(pattern_bytes)) + " bytes from pattern binary")

	## ------------------------------------ ##

	subprocess.call("ybug spin5.polito.it -bmp bmp-spin5.polito.it < reboot-spin5.txt", shell=True)

	print("ACP:", spynnaker_acp.__path__)
	print("MPI:", spynnaker_mpi.__path__)

	runtime = spynnaker_mpi.MPIRuntime.create("spin5.polito.it", "bmp-spin5.polito.it")
	context = spynnaker_mpi.MPIContext(runtime, rings, cores)
	app = spynnaker_mpi.MPIApp(runtime, "mpi_template.aplx")

	# This sleep is necessary because for large number of cores it takes a while to create the context.
	# Without this sleep, it will crash
	# Context works up to 7x16 if <= 75000 patterns
	time.sleep(2)

	## ----- HOST-BOARD COMMUNICATION ----- ##
	(x,y,p) = context.get_processor(0)
	me1000 = spynnaker_acp.RemoteMemoryEntity(runtime, 1000)
	me1001 = spynnaker_acp.RemoteMemoryEntity(runtime, 1001)
	me1002 = spynnaker_acp.RemoteMemoryEntity(runtime, 1002)
	me1003 = spynnaker_acp.RemoteMemoryEntity(runtime, 1003)
	uint8_acp = spynnaker_acp.types.ACPIntegerType(8, False)
	uint32_acp = spynnaker_acp.types.ACPIntegerType(32, False)
	uint8_acp.value = 0
	uint32_acp.value = 0
	## ------------------------------------ ##

	# Init
	app.init(context)
	#time.sleep(1)

	# Start
	app.run()

	## ----- HOST-BOARD COMMUNICATION ----- ##
	time.sleep(1)
	uint32_acp.value = n_chunks
	me1002.update(x, y, p, uint32_acp)

	time.sleep(1)
	uint32_acp.value = n_patterns
	me1003.update(x, y, p, uint32_acp)

	time.sleep(1)
	# Read ME with memory pointer
	me1000.read(x, y, p, uint32_acp)
	print("Received: 0x{:08x}".format(uint32_acp.value))

	# SCP write to SDRAM
	print('Writing TEXT buffers to SDRAM...')

	tc = runtime._transceiver

	tc.write_memory(0, 0, uint32_acp.value, text_bytes)

	print('Unlocking application (1) ...')

	# Unlock spinnaker application
	me1001.update(x, y, p, uint8_acp)
	## ------------------------------------ ##

	## ----- HOST-BOARD COMMUNICATION ----- ##
	time.sleep(1)

	# Read ME with memory pointer
	me1000.read(x, y, p, uint32_acp)
	print("Received: 0x{:08x}".format(uint32_acp.value))

	# SCP write to SDRAM
	print('Writing PATTERN buffers to SDRAM...')
	tc.write_memory(0, 0, uint32_acp.value, pattern_bytes)

	time.sleep(1)
	print('Unlocking application (2) ...')
	# Unlock spinnaker application

	me1001.update(x, y, p, uint8_acp)
	## ------------------------------------ ##

	print("Running ...")

	# Stop
	buffers = app.stop(get_buffers=True)

	# Buffers elaboration
	strings = spynnaker_acp.Utils.get_io_buffers(buffers)

#	text_file = open("output.txt", "w")
#	csv_file = open("output.csv", "a")

	for string in strings:
		print(string)
	#	text_file.write("{0}\n".format(string))
		if "FED + pattern time:" in string:
			overall = int(string.split()[7])
		if "Pattern scattering time:" in string:
			conf = int(string.split()[6])
		if "FED time:" in string:
			match = int(string.split()[5])


	#text_file.close()

	#csv_file.write(",{0},{1},{2},{3},{4},,{5},,{6},{7}\n".format(mpi_workers,n_chunks,n_patterns,overall,conf,match,rings,cores))
	#csv_file.close()


@click.command()
@click.option('--rings', default=0, help='Number of rings')
@click.option('--cores', default=2, help='Number of cores per chip')
@click.option('--chunks', default=10, help='Number of text chunks to read')
@click.option('--patterns', default=120, help='Number of patterns per MPI worker')
def main(rings, cores, chunks, patterns):
	#print("{} {} {} {}".format(rings, cores, chunks, patterns))
	print("Options:")
	print("	--rings {}".format(rings))
	print("	--cores {}".format(cores))
	print("	--chunks {}".format(chunks))
	print("	--patterns {}".format(patterns))
	do(rings, cores, chunks, patterns)


if __name__ == "__main__":
    main()
