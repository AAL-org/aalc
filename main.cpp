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
*   .------------..:oshddy+-----------------:shdds+/...------------.   
*   .---------..`      ./hdh+-------------:yddo-       `..---------.   
*  `---------.`    `.`    :hdy-----------/dds.   `--.    `.---------`  
*  `--------.`   -ydddy-   .hdh---------:ddo    +ddddy.   `---------`  
*  .--------.    ydddddy    -+/----------//.   .dddddd+   `.--------.  
*  `--------.`   -hdddh:   `.--------------.    /hdddo`   .---------`  
*  `--------:/`    .-.    `.----------------.`    ``     -/:--------`  
*   .-----:////:.      `-:::----------------:::-`     `-/////------.   
*   .----/////////::::ydddddddddhhhhhhhhdddddddddy:://////////:----.   
*    ..-////////////////+oosyyyhhhhhhhhhhyyysso+////////////////-..    
*    `:///////////////:                          ////////////////:`    
*    ./////////////////hs/.                  ./sy/////////////////.    
*    ://///////////////hddddyso//:::::://osyddddh/////////////////:    
*    -/////////////////dddddddddddddddddddddddddh/////////////////-    
*    `:///////////////oddddddddddddddddddddddddddo///////////////:`    
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
#include <vector>

void diemofo (const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);
	exit(1);
}



enum TOKEN_TYPE
{
	T_ADD, // +
	T_SUB, // -
	T_MUL, // *
	T_DIV, // /
	T_INTLIT, //* 123
	T_STRLIT, //* "gamer"
	T_FN, // fn
	T_IDNT, //* myint
	T_BRKO, // {
	T_BRKC, // }
	T_PARO, // (
	T_PARC, // )
	T_INT, // int
	T_STR, // str
	T_ASIGN, // :
	T_END, // ;
	T_FLOAT, // float
	T_FLTLIT, // 123.553
	T_SEP, // ,
};

static const char *TOKEN_TYPE_DEBUG[] = {
	"+",
	"-",
	"*",
	"/",
	"INT LIT",
	"STR LIT",
	"FN",
	"IDENTIFIER",
	"{",
	"}",
	"(",
	")",
	"INT",
	"STR",
	"ASIGN",
	"END LINE",
	"FLOAT",
	"FLOAT LIT",
	"SEP",
};


struct token
{
	int type;
	union {
		int val;
		float flt;
		char *str;
	};
};
std::vector<token> tokens;
FILE* infile;
int c, b, line;
token t;

void putback(int c_)
{
	b = c_;
}

char* stringscan(void)
{
	int i = 0;
	char* s = (char*)malloc(256);
	s[i] = fgetc(infile);
	while (s[i] != '"')
		s[++i] = fgetc(infile);
	s[i] = 0;
	return s;
}

void identscan(void)
{
	int i = 0;
	char* s = (char*)malloc(256);
	s[0] = c;
	while ((s[i] <= 'z' && s[i] >= 'a') || (s[i] <= 'Z' && s[i] >= 'A') || (s[i] <= '9' && s[i] >= '0') || s[i] == '_')
		s[++i] = fgetc(infile);
	putback(s[i]);
	s[i] = 0;

	// see if its a keyword
	if (!strcmp("fn", s))
		t.type = T_FN;
	else if (!strcmp("int", s))
		t.type = T_INT;
	else if (!strcmp("str", s))
		t.type = T_STR;
	else
		t.type = T_IDNT;
	t.str = s;
}

// wtf lmao
void numscan (void)
{
	int val = 0, k, dodec = 0;
	float d, decc = 1;
	char* p;
	while ((p = strchr((char*)"0123456789.", c)) != NULL)
	{
		if (*p == '.')
		{
			if (dodec)
				diemofo("you cant have two decimals!");
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
	t.type = dodec ? T_FLTLIT : T_INTLIT;
	dodec ? (t.flt = d) : (t.val = val);
}

int whitespace (int c)
{
	if (c == ' ' || c == '\t')
		return 1;
	else if (c == '\n')
		++line;
	else
		return 0;
	return 1;
}

int next (void)
{
	if (b) {
		c = b;
		b = 0;
	} else
		c = fgetc(infile);

	if (c == EOF)
		return 0;
	if (whitespace(c))
		return next();
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
				t.type = T_DIV;
				break;
			}
		case '"':
			t.type = T_STRLIT;
			t.str = stringscan();
			break;
		case '(':
			t.type = T_PARO;
			break;
		case ')':
			t.type = T_PARC;
			break;
		case '{':
			t.type = T_BRKO;
			break;
		case '}':
			t.type = T_BRKC;
			break;
		case ':':
			t.type = T_ASIGN;
			break;
		case ';':
			t.type = T_END;
			break;
		default:
		{
			if (isalpha(c))
			{
				identscan();
				break;
			}
			if (isdigit(c))
			{
				numscan();
				break;
			}
			diemofo("Unknown character %c at line %d\n", c, line);
		}
	}
	return 1;
}

struct astnode
{
	int op;
	union {
		int val;
		float flt;
		char *str;
	};
	astnode *left;
	astnode *right;

	static astnode makeast(std::vector<token> *tokens)
	{
		
	}
};

int main (int argc, char *argv[])
{
	infile = fopen(argv[1], "r");
	while(next())
	{
		tokens.push_back(t);
		#ifdef debug
		if (t.type == T_INTLIT)
			printf("Token: %s Value: %d\n", TOKEN_TYPE_DEBUG[t.type], t.val);
		else if (t.type == T_FLTLIT)
			printf("Token: %s Value: %f\n", TOKEN_TYPE_DEBUG[t.type], t.flt);
		else if (t.type == T_STRLIT || t.type == T_FN || t.type == T_INT || t.type == T_STR || t.type == T_IDNT)
			printf("Token: %s Value: %s\n", TOKEN_TYPE_DEBUG[t.type], t.str);
		else
			printf("Token: %s\n", TOKEN_TYPE_DEBUG[t.type]);
		#endif
	}

	
	fclose(infile);
	return 0;
}