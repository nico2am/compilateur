#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyseur_lexical.h"
#include "analyseur_syntaxique.h"
#include "symboles.h"

FILE *yyin;
int trace_synt = 0;
int trace_asynt = 0;
int trace_dico = 0;
int trace_mips = 0;
char *filename;

void afficher_usage(char *name) {
    printf("usage: %s [-l|-s|-a|-t] nom-fichier\n", name);
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 2) {
        afficher_usage(argv[0]);
        return 1;
    }

    if (argc == 2) {
        filename = argv[1];
    } else {
        filename = argv[2];
    }

    yyin = fopen(filename, "r");
    if (yyin == NULL) {
        printf("impossible d'ouvrir le fichier %s\n", filename);
        exit(1);
    }

    if (argc == 2) {
        trace_synt = 1;
        test_analyseur();
    } else {
        if (strcmp(argv[1], "-l") == 0) {
            test_yylex_internal(yyin);
        } else if (strcmp(argv[1], "-s") == 0) {
            trace_synt = 1;
            test_analyseur();
        } else if (strcmp(argv[1], "-a") == 0) {
            trace_asynt = 1;
            test_analyseur();
        } else if (strcmp(argv[1], "-t") == 0) {
            trace_dico = 1;
            test_analyseur();
        } else if (strcmp(argv[1], "-m") == 0) {
            trace_mips = 1;
            test_analyseur();
        }
    }

    return 0;
}

