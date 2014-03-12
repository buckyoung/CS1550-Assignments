#!/usr/bin/env python

# Dirty bit: if this memory has been Written to (and thus, it must write to disk upon eviction)
# Referenced bit: if this memory has been read or written to (if it's been referenced at all)

# Four algorithms: Optimal, Clock (w/ circular queue enhancement of second chance algo.),
#					NotRecentlyUsed (variable of what "recent means" / use D and R bits), Random

# Implement a page table for a 32-bit address, all pages are 4kb's, number of frames are a cmdline param

"""
	Program:
		1) Get cmd-line args
		2) Run through file
		3) Display Action taken for each address:
			- Hit
			- Page Fault (no eviction)
			- Page Fault (evict clean page)
			- Page Fault (evict dirty page -- write to disk)
		4) Print Summary Stats
			- Number of frames / total memory accesses / total number of page faults / total writes to disk
"""

# File has memory address followed by (R)ead or (W)rite

# Note: play around with NRU to determine the best refresh rate

import sys

#Heuristics
mem_accesses = 0
page_faults = 0
writes_disk = 0
#

#Function Declarations:
def exit():
	print("Usage: ./vmsim -n <numframes> -a <opt|clock|nru|rand> [-r <NRUrefresh>] <tracefile>")
	sys.exit(-1)

def set_args():
	#check length
	if not((len(sys.argv) == 6) or (len(sys.argv) == 8)):
		exit()

	#check and set -n -a and -r
	if "-n" in sys.argv:
		global num_frames
		i = sys.argv.index("-n")
		i += 1
		num_frames = sys.argv[i]
	else: #if -n is not included in the cmdline args
		exit()

	if "-a" in sys.argv:
		global algorithm
		i = sys.argv.index("-a")
		i += 1
		algorithm = sys.argv[i]
	else: #if -a is not included in the cmdline args
		exit() 

	if "nru" in sys.argv:
		if "-r" in sys.argv:
			global nru_refresh
			i = sys.argv.index("-r")
			i += 1
			nru_refresh = sys.argv[i]
		else:
			exit()

	#set filename
	global filename
	filename = sys.argv[-1]


# START MAIN:

# Set the global's from the cmd-line args
set_args()

# Create RAM with user-defined number of frames
# Note: each frame in RAM is initialized to -1
RAM = [-1 for i in range(int(num_frames))] # This is called a comprehension
FRAME_COUNTER = 0 #init 

# Create page table dictionary structure
PAGE_TABLE = {} #init

while(True):
	print("PAGE_TABLE = %s" % PAGE_TABLE)
	# TODO: Open file and reading
	# DEBUG: Read line from keyboard
	line = raw_input("DEBUG: Enter line from file: ")# DEBUG

	# Split line based on whitespace
	result = line.split(" ")

	# Create the page number and operation
	memory_address = result[0] #in hex
	page_number = memory_address[:5] #ignore the offset!
	operation = result[1] #R or W

	# Check for page number in the page table
	try:
		pt_entry = PAGE_TABLE[page_number]
		# Exists in page table!:

		# Check for page in RAM
		frame_number = pt_entry[7:]
		ram_entry = RAM[int(frame_number)]

		if(page_number == ram_entry[2:7]): #HIT!
			#TODO
			print("Hit!")
		else: #PAGE FAULT -- RUN EVICTION ALGO!
			#TODO -- run eviction algorithm
			print("Page Fault: Evict")

	except KeyError:
		# Does not exist in page table yet:

		# Create page table entry
		D_bit = False #init
		R_bit = False #init
		if (operation.upper() == "R"):
			R_bit = True
		if (operation.upper() == "W"):
			D_bit = True
			R_bit = True


		if (int(FRAME_COUNTER) < int(num_frames)): #PAGE FAULT - NO EVICTION!
			print("Page Fault: no eviction")
			frame_number = FRAME_COUNTER
			FRAME_COUNTER += 1
		else: #PAGE FAULT -- RUN EVICTION ALGO!
			#DO THE eviction ALGORITHM
			print("Page Fault: Evict (frames full)")
			frame_number = -1
		page_table_entry = "%i%i%s%d" % (D_bit, R_bit, page_number, frame_number)
		#Insert the entry into the page table
		PAGE_TABLE[page_number] = page_table_entry
		#Insert the entry into RAM
		RAM[frame_number] = page_table_entry



# END MAIN

#DEBUG
print("num_frames = %s" % num_frames)
print("algorithm = %s" % algorithm)
try:
	nru_refresh
	print("nru_refresh = %s" % nru_refresh)
except NameError:
	pass
print("filename = %s" % filename)
print("RAM Len = %d" % len(RAM))
print("Line = %s" % line)
print("Memory Address = %s" % memory_address)
print("Operation = %s" % operation)
print("Page Number = %s" % page_number)
print("PAGE_TABLE = %s" % PAGE_TABLE)
print("Page table entry = %s" % page_table_entry)