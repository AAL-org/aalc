#include "aalc.h"

ASTnode* astnode(int op, int value, ASTnode *lhs, ASTnode *rhs)
{
	ASTnode *n = malloc(sizeof(ASTnode));
	if (n == NULL)
		fatal("Unabled to malloc node");

	n->op = op;
	n->value = value;
	n->lhs = lhs;
	n->rhs = rhs;
}

ASTnode* astleaf(int op, int value)
{
	return astnode(op, value, NULL, NULL);
}

ASTnode* astunary(int op, int value, ASTnode *lhs)
{
	return astnode(op, value, lhs, NULL);
}