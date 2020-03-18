#include "aalc.h"

#define expected(type) fatal("Expected a '%s' but found a '%s'\n", TOKEN_TYPE_DEBUG[type], TOKEN_TYPE_DEBUG[t.type]);

static vec_Token *tks;
static vec_int *stack;
static Token t;
static int tki = -1;

// Gets the next token
static inline Token next()
{ t = tks->data[++tki]; return t; }

// Reverts the token index if we backtrack
static inline void unwind()
{ tki = vec_int_pop(stack); }

// Marks our token index when we enter a production for backtracking
static inline void enter()
{ vec_int_push(stack, tki); }

// Make sure that the next token is of the right type
static inline void match(int type)
{ if(next().type != type) expected(type); }

static ASTnode* statement()
{

}

static ASTnode* compound_statement()
{
	ASTnode *tree, *left = NULL;

	for (;;)
	{
		tree = statement();

		if (tree != NULL)
		{
			if (left == NULL)
				left = tree;
			else
				left = astnode(A_COMB, 0, left, tree);
		}
		if (tks->data[tki+1].type == T_BRC)
		{
			next();
			return left;
		}
	}
}

static ASTnode* function()
{
	ASTnode *tree = astleaf(A_FN, 0);
	enter();
	
	match(T_FN);
	match(T_IDNT);
	// add symbol table shit
	match(T_PRO);
	//tree->lhs = parameter_list();
	match(T_PRC);
	//tree->rhs = compound_statement();

	return tree;
}

static ASTnode* unit()
{
	ASTnode *tree, *left = NULL;

	for (;;)
	{
		tree = function();

		if (tree != NULL)
		{
			if (left == NULL)
				left = tree;
			else
				left = astnode(A_COMB, 0, left, tree);
		}
		else
		{
			unwind();
			tree = statement();

			if (left == NULL)
				left = tree;
			else
				left = astnode(A_COMB, 0, left, tree);
		}
		if (tks->data[tki+1].type == T_EOF)
			return left;
	}

	return tree;
}

// Creates an AST from our token vector
ASTnode *parse(vec_Token *tokens)
{
	// Set our static variables
	stack = vec_int_new();
	tks = tokens;
	
	// Parse our program
	ASTnode *tree = unit();

	// Free backtrack stack, tokens and return tree

	return tree;
}