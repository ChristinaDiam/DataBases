# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Sample Makefile that you can quickly adapt to your needs  #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Set compiler options here
CFLAGS=-g -Wall -g3

# Set the name of the executable file to compile here
program=demo

# List object files needed to compile the program
objects=Main.o HP.o HT.o

# List library files needed to compile the program
libs=BF_64.a -no-pie

# This makes the program. It's the first rule so it will be executed if you give just 'make'

$(program) : $(objects)
		gcc $(CFLAGS) $(objects) $(libs) -o $(program) 

# List .h dependencies for each .o (dependency on .c is automatically set by make!)
Main.o : HP.h HT.h
HP.o : HP.h BF.h Record.h
HT.o : HT.h BF.h Record.h

# This allows to quickly remove all generated files with 'make clean'
clean:
		rm $(program) $(objects)

# Run the program with 'make run'. Sometimes useful to avoid rewriting long commands.
run: $(program)
		./$(program)

# Run Valgrind for this program to check possible memory leaks.
run_valgrind: $(program)
		valgrind ./$(program)