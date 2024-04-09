test: compiler.out
	@for name in ./testcode/*.c; do \
		./compiler.out $$name; \
	done
	@rm compiler.out

compiler.out: lexical_analyzer.c syntactical_analyzer.c compiler.c
	@gcc -Wall -o compiler.out lexical_analyzer.c semantic_analyzer_fragment.c syntactical_analyzer.c compiler.c