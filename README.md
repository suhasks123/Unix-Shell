# Unix-Shell

A basic implementation of a unix-like shell using C.


Shell.c in the Unix-Shell directory is the most updated version of the shell. It includes the following features:

1. I/O Redirection.
2. Command Piping.
3. Quoting.
4. Tab Completion.

# Instructions to run:

1. Compile the program that is to be run by the following command on a linux machine:
	
	gcc -o a.out Shell.c -lreadline
	
	
	
2. If an older version is to be run:
	
	gcc -o a.out <filename.c>
	
	
	
3. Run the binary generated after compilation:

	./a.out


# Note:
Incase of errors related to 'readline/readline.h' during compilation, check if libreadline-dev is installed on the machine. If not, it can be installed with the command (On Debian-based machines):

	sudo apt install libreadline-dev
