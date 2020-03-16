#include "aalc.h"

#define expect(t) {if (t != c) {\
	fprintf(stderr, "Expected a %c on line %d but got a %c\n", t, line, c); \
	exit(1);}}

#define unexpect(t) {\
	fprintf(stderr, "Unexpected %c on line %d\n", t, line); \
	exit(1);}

static vec_Token *tokens;
static Token t;
static char c, *s;
static int i = -1, line = 1;

// Set c to the next character in the buffer
static inline char ntc(void) { c = s[++i]; return c; }
// Set c the previous character in the buffer
static inline char bkc(void) { c = s[--i]; return c; }

// Scans an identifier or keyword
static inline void identscan(void)
{
	// Read into a string until we reach a invalid identifier character
	int in = 0; char* str = (char*)malloc(MAX_IDENT_LENGTH);
	if (str == NULL)
		fatal("Failed malloc\n");
	str[0] = c;
	while ((str[in] <= 'z' && str[in] >= 'a') || (str[in] <= 'Z' && str[in] >= 'A') || (str[in] <= '9' && str[in] >= '0') || str[in] == '_')
		str[++in] = ntc();
	bkc();
	str[in] = 0;
	
	// Do keyword testing
	t.type = T_IDNT;
	switch (str[0])
	{
		case 'c':{ // char
					if (!strcmp (str, "char"))
						t.type = T_CHR;
					break;}
		case 'e':{ // else
					if (!strcmp (str, "else"))
						t.type = T_ELS;
					break;}
		case 'f':{ // fn float
					if (!strcmp (str, "float"))
						t.type = T_FLT;
					else if (!strcmp (str, "fn"))
						t.type = T_FN;
					break;}
		case 'i': {// if int
					if (!strcmp (str, "if"))
						t.type = T_IF;
					else if (!strcmp (str, "int"))
						t.type = T_INT;
					break;}
		case 's':{ // str
					if (!strcmp (str, "str"))
						t.type = T_STR;
					break;}
		case 'w':{ // while
					if (!strcmp (str, "while"))
						t.type = T_WHL;
					break;}
	}
	if (t.type == T_IDNT)
		t.string = str;
	else
		free(str);
}

// Checks if the character is esacped
static inline int esacped(void)
{
	// this was not esacped that means end of string
	if (ntc() == '"')
		return 0;

	// see if we need to esacpe something
	if (c == '\\')
	{
		ntc();
		switch (c)
		{
			case 't':
				c = '\t';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 'f':
				c = '\f';
				break;
			case '\\':
				c = '\\';
				break;
			case '"':
				c = '"';
				break;
		}
	}
	return 1;
}

// Scans a string with proper esacpe sequences
static inline void stringscan(void)
{
	int in = 0; char* str = (char*)malloc(MAX_IDENT_LENGTH);
	if (str == NULL)
		fatal("Failed malloc\n");
	str[0] = ntc();
	while (esacped())
		str[++in] = c;
	str[in] = 0;
	
	t.type = T_STRL;
	t.string = str;
}

// Scans a integer or decimal
static inline void numberscan(void)
{
	long int integer = 0;
	int digit, isdecimal = 0;
	double decimal, f = 1.0;
	char* p;
	while ((p = strchr((char*)"0123456789.", c)) != NULL)
	{
		// Check to see if we are entering the decimal place
		if (*p == '.')
		{
			// Check that we dont already have a decimal marker
			if (isdecimal++)
				unexpect(*p);
			// Convert what we have of the whole number
			decimal = integer;
			// Continue on
			ntc(); continue;
		}

		// Convert our character to a number
		digit = *p - '0';

		// If we are decimal to decimal math
		if (isdecimal) {
			f *= 0.1; // Shift our place over one
			decimal = decimal + f * digit;}
		else // Otherwise do normal integer math
			integer = integer * 10 + digit;

		ntc();
	}
	// Put back the invalid character
	bkc();

	// Set the type and the value
	t.type = isdecimal ? T_FLTL : T_INTL;
	isdecimal ? 
		(t.decimal = decimal) :
		(t.integer = integer);
}

// Skips through a comment
static inline void comment(void)
{
	while (ntc() != '\n')
		{}
	++line;
}

// Checks if we are whitespace
static inline int whitespace(void)
{
	if (c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\v')
		return 1;
	else if (c == '\n')
		{++line; return 1;}
	return 0;
}

// Scans the next token
static int scan(void)
{
	ntc();
	// if we are at the end of the string stop
	if (c == 0)
		return 0;
	// if we are whitespace return our next self
	if (whitespace())
		return scan();

	switch (c)
	{
		case '+':
			t.type = T_ADD;
			break;
		case '-':
			t.type = T_SUB;
			break;
		case '*':
			t.type = T_MUL;
			break;
		case '/':
			ntc(); // Check for comment
			if (c == '/') // We have a comment
				comment();
			else{ // if not put that back
				bkc();
				t.type = T_DIV;}
			break;
		case ':':
			ntc(); // Check if the next thing is an equals
			if (c == ':')
				t.type = T_EQL;
			else{ // if not put that back
				bkc();
				t.type = T_ASN;}
			break;
		case ';':
			t.type = T_END;
			break;
		case '"':
			stringscan();
			break;
		case '(':
			t.type = T_PRO;
			break;
		case ')':
			t.type = T_PRC;
			break;
		case '{':
			t.type = T_BRO; // whats up bro?!
			break;
		case '}':
			t.type = T_BRC;
			break;
		default:
		{
			if (isalpha(c))
				identscan();
			else if (isdigit(c))
				numberscan();
			else
				fatal("Unknown character %c on line %d", c, line);
			break;
		}
	}
	return 1;
}

vec_Token* lex (char* source)
{
	s = source;

	// Scan the tokens and push them onto the vector
	tokens = vec_Token_new();
	while(scan())
		vec_Token_push(tokens, t);

	/*while(scan()){
		if (t.type <= T_STR)
			printf("token: %s\n", TOKEN_TYPE_DEBUG[t.type]);
		else if (t.type == T_IDNT || t.type == T_STRL)
			printf("token: %s value: %s\n", TOKEN_TYPE_DEBUG[t.type], t.string);
		else if (t.type == T_INTL)
			printf("token: %s value: %ld\n", TOKEN_TYPE_DEBUG[t.type], t.integer);
		else if (t.type == T_FLTL)
			printf("token: %s value: %lf\n", TOKEN_TYPE_DEBUG[t.type], t.decimal);}*/

	return tokens;
}