# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Sample Makefile that you can quickly adapt to your needs  #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Set compiler options here
CFLAGS=-g -Wall -g3

# Set the name of the executable file to compile here
program=demo

# List object files needed to compile the program
objects=Main.o HP.o HT.o SHT.o

# List library files needed to compile the program
libs=BF_64.a -no-pie

# This makes the program. It's the first rule so it will be executed if you give just 'make'

$(program) : $(objects)
		gcc $(CFLAGS) $(objects) $(libs) -o $(program) 

# List .h dependencies for each .o (dependency on .c is automatically set by make!)
Main.o : HP.h HT.h SHT.h
HP.o : HP.h BF.h Record.h
HT.o : HT.h BF.h Record.h
SHT.o : SHT.h BF.h Record.h

# This allows to quickly remove all generated files with 'make clean'
clean:
		rm $(program) $(objects)

# Run the program with 'make run'. Sometimes useful to avoid rewriting long commands.
run: $(program)
		./$(program)

# Opens the file "records1K.txt" and creates a heap file to save the records
run_heap_1K: $(program)
		./$(program) -f records1K.txt -t heap

# Opens the file "records1K.txt" and creates a hash file with 7 buckets to save the records
run_hash_1K: $(program)
		./$(program) -f records1K.txt -t hash -b 7

# Opens the file "records1K.txt" and creates two hash files with 7 buckets to save the records
run_+sht_1K: $(program)
		./$(program) -f records1K.txt -t +sht -b 7

# Opens the file "records5K.txt" and creates a heap file to save the records
run_heap_5K: $(program)
		./$(program) -f records5K.txt -t heap

# Opens the file "records5K.txt" and creates a hash file with 59 buckets to save the records
run_hash_5K: $(program)
		./$(program) -f records5K.txt -t hash -b 59

# Opens the file "records5K.txt" and creates two hash files with 59 buckets to save the records
run_+sht_5K: $(program)
		./$(program) -f records5K.txt -t +sht -b 59

# Opens the file "records10K.txt" and creates a heap file to save the records
run_heap_10K: $(program)
		./$(program) -f records10K.txt -t heap

# Opens the file "records10K.txt" and creates a hash file with 113 buckets to save the records
run_hash_10K: $(program)
		./$(program) -f records10K.txt -t hash -b 113

# Opens the file "records10K.txt" and creates two hash files with 113 buckets to save the records
run_+sht_10K: $(program)
		./$(program) -f records10K.txt -t +sht -b 113

# Opens the file "records15K.txt" and creates a heap file to save the records
run_heap_15K: $(program)
		./$(program) -f records15K.txt -t heap

# Opens the file "records15K.txt" and creates a hash file with 181 buckets to save the records
run_hash_15K: $(program)
		./$(program) -f records15K.txt -t hash -b 181

# Opens the file "records15K.txt" and creates two hash files with 181 buckets to save the records
run_+sht_15K: $(program)
		./$(program) -f records15K.txt -t +sht -b 181

# Run Valgrind for this program to check possible memory leaks.
run_valgrind: $(program)
		valgrind --track-origins=yes ./$(program) -f records1K.txt -t +sht -b 7