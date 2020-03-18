#ifndef _AALC_H
#define _AALC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_IDENT_LENGTH (256)

#define USAGE "USAGE: aalc <input>\n"

#define fatal(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define warning(...) {fprintf(stderr, __VA_ARGS__);}
#define info(...) {fprintf(stdout, __VA_ARGS__);}

enum TOKEN_TYPE
{
	// + - * / : :: ; ( ) { } ->
	T_ADD = 0, T_SUB, T_MUL, T_DIV, T_ASN, T_EQL, T_END, T_PRO, T_PRC, T_BRO, T_BRC, T_RTY,
	// fn if else while ret
	T_FN, T_IF, T_ELS, T_WHL, T_RET,
	// int float char str
	T_INT, T_FLT, T_CHR, T_STR,
	// literals
	T_INTL, T_FLTL, T_CHRL, T_STRL,
	// identifier
	T_IDNT, T_EOF
};

#ifdef debug
static const char *TOKEN_TYPE_DEBUG[] = {
	"+", "-", "*", "/", ":", "::", ";", "(", ")", "{", "}", "->",
	"fn", "if", "else", "while", "ret",
	"int", "float", "char", "str",
	"int literal", "float literal", "char literal", "str literal",
	"identifier", "EOF"
};
#endif

typedef struct Token
{
	int type;
	union
	{
		long int integer;
		double decimal;
		char* string;
	};
} Token;

static const Token EOF_TOKEN = {T_EOF, 0};

#define VECTOR_TYPE Token
#include <srxk_vector.h>

#define VECTOR_TYPE int
#include <srxk_vector.h>

enum AST_TYPE
{
	A_FN, A_UNIT, A_COMB
};

typedef struct ASTnode
{
	int op;
	int value;
	struct ASTnode *lhs;
	struct ASTnode *rhs;
} ASTnode;

ASTnode* astnode(int op, int value, ASTnode *lhs, ASTnode *rhs);
ASTnode* astleaf(int op, int value);
ASTnode* astunary(int op, int value, ASTnode *lhs);

vec_Token* lex (char* source);
ASTnode *parse(vec_Token *tokens);
void sema(ASTnode *root);

#endif // _AALC_H