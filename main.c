#include "aalc.h"

// This should all be pretty basic
int main (int argc, char *argv[])
{

	if (argc < 2)
		fatal(USAGE);

	char *source;
	{
		FILE *f = fopen (argv[1], "r");
		if (f == NULL)
			fatal("Error opening source file: %s\n", strerror(errno));
		fseek(f, 0, SEEK_END); size_t length = ftell(f); fseek(f, 0, SEEK_SET);
		source = malloc(length);
		fread(source, 1, length, f);
		fclose(f);
		source[length] = 0;
	}

	vec_Token *tokens = lex(source);
	
	#ifdef debug
		Token t;
		for(int i = 0; i < tokens->len; i++)
			printf("%s\n", TOKEN_TYPE_DEBUG[tokens->data[i].type]);
	#endif

	ASTnode *ast = parse(tokens);
	sema(ast);
}