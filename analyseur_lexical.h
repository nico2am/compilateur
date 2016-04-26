#ifndef __LIRE_UNITE__
#define __LIRE_UNITE__

#include "stdio.h"

#define YYTEXT_MAX 100
#define is_num(c)(('0' <= (c)) && ((c) <= '9'))
#define is_maj(c)(('A' <= (c)) && ((c) <= 'Z'))
#define is_min(c)(('a' <= (c)) && ((c) <= 'z'))
#define is_alpha(c)(is_maj(c) || is_min(c) || (c) == '_' || (c) == '$')
#define is_alphanum(c)(is_num((c)) || is_alpha((c)))

int yylex(void);
void nom_token( int token, char *nom, char *valeur );
void test_yylex_internal( FILE *yyin );

void get_yytext(char *valeur);

#endif
