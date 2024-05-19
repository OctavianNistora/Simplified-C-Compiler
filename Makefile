test: compiler.out
	@for name in ./testcode/*.c; do \
		./compiler.out $$name; \
	done
	@rm compiler.out

compiler.out: lexical_analyzer.c syntactical_analyzer.c compiler.c
	@gcc -Wall -o compiler.out lexical_analyzer.c stack_fragment.c external_functions.c semantic_analyzer_fragment.c syntactical_analyzer.c virtual_machine.c compiler.c