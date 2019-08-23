# Unix-Shell

A basic implementation of a unix-like shell using C.


shellwTabCmplete.c is the most updated version of the shell. It includes the following features:

1. I/O Redirection.
2. Command Piping.
3. Quoting.
4. Tab Completion.

Instructions to run:

1. Compile the program that is to be run by the following command on a linux machine.
	
	gcc -o a.out <filename.c>
	
	If the file shellwTabCmplete.c is to be run, execute a different command:
	
	gcc -o a.out shellwTabCmplete.c -lreadline
	
2. Run the binary generated after compilation.

	./a.out
