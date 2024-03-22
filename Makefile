test: compiler.out
	for name in ./testcode/*.c; do \
		./compiler.out $$name; \
	done

compiler.out: lexical_analyzer.c syntactical_analyzer.c compiler.c
	gcc -o compiler.out lexical_analyzer.c syntactical_analyzer.c compiler.c
