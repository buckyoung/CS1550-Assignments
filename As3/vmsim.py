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
from random import randint

#Object Classes:
class PTEntry:
	def __init__(self):
		self.d = 0
		self.r = 0
		self.v = 0
		self.pn = 0
		self.fn = -1 # 11PM addition
	def __repr__(self):
		return "(%d%d%d|%s|%d)" % (self.d, self.r, self.v, self.pn, self.fn)
	def is_valid(self):
		return (self.v != 0 and self.fn != -1) # 11PM addition
	def evicted(self): # 11PM addition
		self.d = 0
		self.r = 0
		self.v = 0
		self.fn = -1 # 11PM addition
	def added(self):
		self.r = 1
		self.v = 1

	def get_frame_number(self):
		return self.fn
	def get_page_number(self):
		return self.pn
	def get_dirty_bit(self):
		return self.d
	def get_ref_bit(self):
		return self.r
	def get_valid_bit(self):
		return self.v
	def get_key(self):
		return self.pn

	def set_dirty_bit(self, bit):
		self.d = bit
	def set_ref_bit(self, bit):
		self.r = bit
	def set_valid_bit(self, bit):
		self.v = bit
	def set_page_num(self, num):
		self.pn = num
	def set_frame_num(self, num):
		self.fn = num

class Ram:
	def __init__(self, numframes):
		self.nf = numframes
		self.array = [PTEntry() for i in range(self.nf)] # This is called a comprehension
		self.fc = 0 #frame counter
		self.clock_hand = 0 #init
	def __repr__(self):
		return "RAM(%d): %s FC(%d)" % (self.nf, self.array, self.fc)
	def add(self, entry):
		self.array[entry.get_frame_number()] = entry
		entry.added()
		self.fc += 1
	def update(self, index, entry):
		self.array[index] = entry
	#def clear(self):
	#	self.array = [PTEntry() for i in range(self.nf)]
	#	self.fc = 0
	def is_full(self):
		return (int(self.fc) >= int(self.nf));

	def evict(self): # 11PM addition
		#Get Entry based on ALGO
		#RAND
		if ("opt" in algorithm): #OPTIMAL
			print("Not Yet Implemented!")
		elif ("clock" in algorithm): #CLOCK EVICT
			replaced = False #init
			while(not replaced):
				#inspect R bit at hand location
				old_entry = self.array[self.clock_hand]
				if(old_entry.get_ref_bit() == 0): #If R == 0, place here
					#evict entry
					evictthis = old_entry.get_frame_number() #return this entry
					replaced = True
				else: #Else, clear R
					old_entry.set_ref_bit(0)
					self.clock_hand += 1
					self.clock_hand = self.clock_hand % self.nf
				#Increment clock hand and try again until replaced


		elif ("nru" in algorithm): #NOT RECENTLY USED EVICT
			print("Not Yet Implemented!")
		else: #RANDOM EVICT
			#Evict whatever the FC is at, modulo # of frames
			evictthis = randint(0,self.nf-1)
			#get whatever was there
			old_entry = self.array[evictthis]
			#empty it
			#self.array[evictthis] = PTEntry() #well... not needed -- it will be overwritten!

		#Determine if we need to write the old one to disk:
		if(old_entry.get_dirty_bit() == 1):
			#print("RAM: Evict Dirty")
			global total_writes_to_disk
			total_writes_to_disk += 1
		#else:
			#print("RAM: Evict Clean")

		old_entry.evicted() #invalidate and reset bits
		return evictthis #return INT -- the frame number to be replaced!
	
	def get_frame_number(self):
		return self.fc
	def get_entry(self, index):
		return self.array[index]


class PageTable:
	def __init__(self):
		self.pt = {}
	def __repr__(self):
		return "PT: %s" % (self.pt)
	def add(self, entry):
		entry.set_valid_bit(1)
		self.pt[entry.get_key()] = entry
	def update(self, entry):
		self.pt[entry.get_key()] = entry

	def get_entry(self, key):
		return self.pt[key]


	def dirty_bit(self, key):
		self.pt[key] = "1%s" % (self.pt[key][1:])



#Function Declarations:
#EXIT
def exit():
	print("Usage: ./vmsim -n <numframes> -a <opt|clock|nru|rand> [-r <NRUrefresh>] <tracefile>")
	sys.exit(-1)
#END EXIT

#SETARGS
def set_args():
	#check length
	if not((len(sys.argv) == 6) or (len(sys.argv) == 8)):
		exit()

	#check and set -n -a and -r
	if "-n" in sys.argv:
		global num_frames
		i = sys.argv.index("-n")
		i += 1
		num_frames = int(sys.argv[i])
	else: #if -n is not included in the cmdline args
		exit()

	if "-a" in sys.argv:
		global algorithm
		i = sys.argv.index("-a")
		i += 1
		algorithm = sys.argv[i]
		algorithm = algorithm.lower()
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
#END SETARGS


# START MAIN:

# Set the global's from the cmd-line args
set_args()
global total_memory_access
total_memory_access = 0 #init
global total_page_faults
total_page_faults = 0 #init
global total_writes_to_disk
total_writes_to_disk = 0 #init

# Create RAM with user-defined number of frames
# Note: each frame in RAM is initialized to -1
RAM = Ram(num_frames) #Create new ram object!
PT = PageTable() #Create new PageTable Object!

#open file
f = open(filename, "r")


for line in f:
	
	total_memory_access += 1
	# TODO: Open file and reading
	# DEBUG: Read line from keyboard
	#line = raw_input("DEBUG: Enter line from file: ")# DEBUG

	# Split line based on whitespace
	result = line.split(" ")

	# Create the page number and operation
	memory_address = result[0] #in hex
	page_number = memory_address[:5] #ignore the offset! First 5
	operation = result[1].rstrip() #R or W, strips the new line

	try: # Check for page number in the page table -- exists in page table
		existing_pt_entry = PT.get_entry(page_number)
		# Exists in page table!:

		# Check for page in RAM
		frame_number = existing_pt_entry.get_frame_number()
		ram_entry = RAM.get_entry(frame_number)

		if(operation.upper() == "W"):
			existing_pt_entry.set_dirty_bit(1)

		if(ram_entry.is_valid() and page_number == ram_entry.get_page_number()): #HIT! in page table and ram!
			"""print("1) Hit!")"""
			#set r bit
			#existing_pt_entry.set_ref_bit(1)
			ram_entry.set_ref_bit(1)
			#Rewrite D-bit from RAM -- in case a write happened a while ago -- could be different than what is kept in the PT
			#existing_pt_entry.set_dirty_bit(ram_entry.get_dirty_bit())
			#Update the existing entry
			#PT.update(page_number, existing_pt_entry)
			#RAM.update(frame_number, existing_pt_entry)
		else: #PAGE FAULT -- RUN EVICTION ALGO! hit in page table but not ram
			#TODO -- check if ram is full, then evict if needed
			if(RAM.is_full()):
				"""print("2) Page Fault: Evict because RAM is full")"""
				total_page_faults += 1
				existing_pt_entry.set_frame_num(RAM.evict()) #returns an empty frame 
				#DEBUG EVICT ALL!
				#RAM.clear()
				#SET THE FRAME NUMBER FROM SOME RETURNING EVICTION FUNCTION!
				#ENDDEBUG
				#Add to RAM
			else: #This type should go away when I implement my algorithms.
				#print("3) Page Fault: Compulsory -- must load page into RAM") 
				print("3)  ---- THIS   SHOULD    NEVER     PRINT     EVER    @(@()(@$*(*$@(*&$%*(&#)%(*&#@jfldsjHF")
				total_page_faults += 1
				#NO evict -- set frame number
				existing_pt_entry.set_frame_num(RAM.get_frame_number())
				#Add to RAM

			RAM.add(existing_pt_entry)

	except KeyError: # ---- Does not exist in page table yet:

		#Start building the new entry
		new_page_table_entry = PTEntry() #create new object!
		new_page_table_entry.set_page_num(page_number)
		#Set R and D based on operation:
		if (operation.upper() == "R"):
			new_page_table_entry.set_ref_bit(1)
		if (operation.upper() == "W"):
			new_page_table_entry.set_ref_bit(1)
			new_page_table_entry.set_dirty_bit(1)

		if (RAM.is_full()): #PAGE FAULT -- RUN EVICTION ALGO!
			#DO THE eviction ALGORITHM
			#DEBUG EVICT ALL!
			#RAM.clear()
			#GET FRAME NUMBER FROM RETURNING EVICTION ALGO
			#ENDDEBUG
			"""print("4) Page Fault: Evict because RAM is full")"""
			total_page_faults += 1
			new_page_table_entry.set_frame_num(RAM.evict()) #evict returns an open frame number!
			#Add new
		else: #PAGE FAULT - NO EVICTION! this will only happen for the first n frames when ram isnt full
			"""print("5) Page Fault: Compulsory -- RAM is not full, must load")"""
			total_page_faults += 1
			#NO evict -- set frame number
			new_page_table_entry.set_frame_num(RAM.get_frame_number())
			#Add new

		#Create Page Table entry and store in RAM!
		PT.add(new_page_table_entry)
		RAM.add(new_page_table_entry)




	#print("%s" % PT)#DEBUG
	#print("%s" % RAM)#DEBUG

#end of file parsing
print("---------------------------------")
print("Number of Frames: %s" % num_frames)
print("Total Memory Accesses: %s" % total_memory_access)
print("Total Page Faults: %s" % total_page_faults)
print("Total Writes to Disk: %s" % total_writes_to_disk)
print("---------------------------------")



# END MAIN