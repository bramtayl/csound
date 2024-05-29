#pragma once

#include <stdio.h>

extern FILE *yyin;
extern int yyline;

void yyerror (char *);
int yylex (void);
