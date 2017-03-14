repl: REPL.c mpc.c
	cc -std=c99 -Wall REPL.c mpc.c -ledit -lm -o repl
clean:
	rm repl
