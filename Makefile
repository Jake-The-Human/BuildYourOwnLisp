repl: REPL.o
	cc -std=c99 -Wall REPL.c -ledit -o repl
