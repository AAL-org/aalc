/*                                                                      
*                          ````..........````                          
*                     ``...-------------------..``                     
*                 ``..----------------------------..``                 
*               `.------------------------------------.`               
*            `.-----------------------------------------..`            
*          `..--------------------------------------------..`          
*         .---------------------SEND HELP-------------------..         
*       `.----------------------------------------------------.`       
*      `.------------------------------------------------------.`      
*     `.--------------------------------------------------------.`     
*    `.----------------------------------------------------------.`    
*    .-------------/yso/-----------------------/oshy:-------------.    
*   .------------..:oshddy+-----------------:shid.+/...------------.   
*   .---------..`      ./hdh+-------------:yddo-       `..---------.   
*  `---------.`    `.`    :hdy-----------/dds.   `--.    `.---------`  
*  `--------.`   -ydddy-   .hdh---------:ddo    +daddy.   `---------`  
*  .--------.    yydaddy    -+/----------//.   .dddddd+   `.--------.  
*  `--------.`   -hdddh:   `.--------------.    /hdddo`   .---------`  
*  `--------:/`    .-.    `.----------------.`    ``     -/:--------`  
*   .-----:////:.      `-:::----------------:::-`     `-/////------.   
*   .----/////////::::ydddddddddhhhhhhhhdddddddddy:://////////:----.   
*    ..-////////////////+oosyyyhhhhhhhhhhyyysso+////////////////-..    
*    `:///////////////:                          ////////////////:`    
*    ./////////////////hs/.                  ./sy/////////////////.    
*    ://///////////////hddddyso//:::::://osyddddh/////////////////:    
*    -/////////////////dddddddddddddddddddddddddh/////////////////-    
*    `:///////////////oddddddddddddddheckddddddddo///////////////:`    
*     `-/////////////--:ohddddddddddddddddddddho:--/////////////-`     
*       `-:///////:--------/oyhddddddddddhyo/--------:///////:-`       
*           `````..--------------::::::---------------.`````           
*                 ``..----------------------------..``                 
*                     ``...------------------...``                     
*                          ````..........````                          
*/                                                                      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define mkleaf(x,y) mknode(x,y,NULL,NULL)
#define mkunary(x,y,z) mknode(x,y,z,NULL)
#define semi() match(T_END)
#define ident() match(T_IDNT)

void fatal (const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	exit(1);
}

enum TOKEN_TYPE
{
	T_ADD = 0, T_SUB, T_MUL, T_DIV, T_ASN, T_SEP, // operators
	T_FN, T_INT, T_STR, T_FLT, // keywords
	T_INTLIT, T_FLTLIT, T_STRLIT, // literals
	T_BKO, T_BKC, T_PRO, T_PRC, T_END, // syntax
	T_IDNT,
};

static const char *TOKEN_TYPE_DEBUG[] = 
{
	"+", "-", "*", "/", ":", ",", // operators
	"fn", "int", "str", "float", // keywords
	"INTEGER", "FLOAT", "STR", // literals
	"{", "}", "(", ")", ";", // syntax
	"IDENTIFIER"
};

enum AST_OP
{
	O_ADD = 0, O_SUB, O_MUL, O_DIV, O_ASN, O_SEP, // operators
	O_FN, O_INT, O_STR, O_FLT, // keywords
	O_INTLIT, O_FLTLIT, O_STRLIT, // literals
	O_BKO, O_BKC, O_PRO, O_PRC, O_END, // syntax
	O_IDNT
};

enum SYMBOL_TYPE
{
	INT = 0,
	FLOAT,
	STRING,
	FUNCTION,
};

typedef struct token
{
	int token;
	union {
		int val;
		float flt;
		char *str;
	};
} token;

typedef struct astnode
{
	int op;
	int value;
	struct astnode *left;
	struct astnode *right;
} astnode;

typedef struct symbol 
{
	int type;
	const char* k;
	union {
		long int integer;
		double decimal;
		char* string;
	} v;
} symbol;

#define TABLESIZE 1024
// global trash
FILE* infile;
int c, putbc = 0, line = 0, tbnxt = 0;
token T;
symbol table[TABLESIZE];
union {long int integer;double decimal; char* string;} symbol_v;

static void insertsymbol (int type, const char *key)
{
	if (tbnxt == 1024)
		fatal("Symbol table has exceded 1024 items");
	table[tbnxt].type = type;
	table[tbnxt].k = key;
	switch (type)
	{
		case INT:
			table[tbnxt].v.integer = symbol_v.integer;
			break;
		case FLOAT:
			table[tbnxt].v.decimal = symbol_v.decimal;
			break;
		case STRING:
			table[tbnxt].v.string = symbol_v.string;
			break;
	}
	++tbnxt;
}

static int findsymbol (const char* key)
{
	for (int i = 0; i < tbnxt; ++i)
	{
		if (!strcmp(table[i].k, key))
			return i;
	}
}

static void putback(int c_) { putbc = c_; }

static char* stringscan(void)
{
	int i = 0;
	char* s = (char*)malloc(256);
	if (s == NULL)
		fatal("failed malloc");
	s[i] = fgetc(infile);
	while (s[i] != '"')
		s[++i] = fgetc(infile);
	s[i] = 0;
	
	return s;
}

static void identscan(void)
{
	int i = 0; char* s = (char*)malloc(256);
	if (s == NULL)
		fatal("failed malloc");
	s[0] = c;
	while ((s[i] <= 'z' && s[i] >= 'a') || (s[i] <= 'Z' && s[i] >= 'A') || (s[i] <= '9' && s[i] >= '0') || s[i] == '_')
		s[++i] = fgetc(infile);
	// put back the invalid char and end the string
	putback(s[i]);
	s[i] = 0;

	// Set the token type
	T.token = T_IDNT;
	T.str = s;
	switch (s[0])
	{
		case 'f':{ // fn, float
			if(!strcmp("float", s))
				T.token = T_FLT;
			else if (!strcmp("fn", s))
				T.token = T_FN;

			break;}
		case 'i':{ // int
			if(!strcmp("int", s))
				T.token = T_INT;

			break;}
		case 's':{ // str
			if(!strcmp("str", s))
				T.token = T_STR;

			break;}
	}
}

// wtf lmao, i'll clean this later
static void numscan (void)
{
	long int val = 0, k, dodec = 0;
	double d, decc = 1;
	char* p;
	while ((p = strchr((char*)"0123456789.", c)) != NULL)
	{
		if (*p == '.')
		{
			if (dodec)
				fatal("you cant have two decimals!");
			dodec = 1;
			d = val;
			c = fgetc(infile);
			continue;
		}
		k = (*p) - '0';
		if (dodec){
			float t = 0.1;
			for (int i = 1; i < decc; ++i)
				t *= 0.1;
			d = d + t * (float)k;
			++decc;
		}
		else
			val = val * 10 + k;
		c = fgetc(infile);
	}
	putback(c);
	T.token = dodec ? T_FLTLIT : T_FLTLIT;
	dodec ? (T.flt = d) : (T.val = val);
}

static int whitespace (int c)
{
	if (c == ' ' || c == '\t')
		return 1;
	else if (c == '\n')
		++line;
	else
		return 0;
	return 1;
}

static int next (void)
{
	// get the current next character
	if (putbc) {
		c = putbc;
		putbc = 0;
	} else
		c = fgetc(infile);

	// check its not EOF
	if (c == EOF)
		return 0;

	// skip whitespace
	if (whitespace(c))
		return next();

	switch (c)
	{
		case '+':
			T.token = T_ADD;
			break;
		case '-':
			T.token = T_SUB;
			break;
		case '*':
			T.token = T_MUL;
			break;
		case '/':
			{
				c = fgetc(infile);
				if (c == '?')
				{
					while (c != '\n')
					{c = fgetc(infile); if (c == EOF) return 0;}
					++line;
					return next();
				}
				putback(c);
				T.token = T_DIV;
				break;
			}
		case '"':
			T.token = T_STRLIT;
			T.str = stringscan();
			break;
		case '(':
			T.token = T_PRO;
			break;
		case ')':
			T.token = T_PRC;
			break;
		case '{':
			T.token = T_BKO;
			break;
		case '}':
			T.token = T_BKO;
			break;
		case ':':
			T.token = T_ASN;
			break;
		case ';':
			T.token = T_END;
			break;
		default:
		{
			if (isdigit(c))
				numscan();
			else if (isalpha(c))
				identscan();
			else
				fatal("Unknown character %c at line %d\n", c, line);
			break;
		}
	}
	return 1;
}

static void match (int op)
{
	if (T.token == op)
		next();
	else
		fatal("Expected: %s", TOKEN_TYPE_DEBUG[op]);
}

astnode* mknode (int op, int value, astnode *left, astnode *right)
{
	astnode *n = (astnode*)malloc(sizeof(astnode));
	if(!n)
		fatal("Couldn't malloc node");

	n->op = op;
	n->value = value;
	n->left = left;
	n->right = right;
	return n;
}

// astnode* mkast (void)
// {
// }

int main (int argc, char *argv[])
{
	if (argc < 2)
		fatal("cannot compile without a input file\nUSAGE: %s <input>\n", argv[0]);
	infile = fopen(argv[1], "r");
	while(next())
	{
		if ((T.token >= T_ADD  && T.token <= T_FLT) || (T.token >= T_BKO  && T.token <= T_END))
			printf("token: \'%s\'\n", TOKEN_TYPE_DEBUG[T.token]);
		else if (T.token == T_INTLIT)
			printf("token: %s\t Value: %d\n", TOKEN_TYPE_DEBUG[T.token], T.val);
		else if (T.token == T_FLTLIT)
			printf("token: %s\t\t Value: %f\n", TOKEN_TYPE_DEBUG[T.token], T.flt);
		else if (T.token == T_IDNT)
			printf("token: %s\t Value: %s\n", TOKEN_TYPE_DEBUG[T.token], T.str);
		else if (T.token == T_STRLIT)
			printf("token: %s\t\t Value: \"%s\"\n", TOKEN_TYPE_DEBUG[T.token], T.str);
	}
	
	fclose(infile);
	exit(0);
}