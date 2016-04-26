#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "symboles.h"
#include "analyseur_lexical.h"
#include "affiche_arbre_abstrait.h"
#include "syntabs.h"
#include "util.h"

extern FILE *yyin;

char *table_mots_clefs[] = {
    "si", "alors", "retour", "sinon", "entier", "lire", "ecrire", "tantque", "faire"
};

int code_mot_clefs[] = {
    SI, ALORS, RETOUR, SINON, ENTIER, LIRE, ECRIRE, TANTQUE, FAIRE
};

char table_symboles[] = {
    '=', ';', '(', ')', ',', '-', '+', '!', '&', '|', '*', '/', '[', ']', '{', '}', '<'
};

int code_symboles[] = {
    EGAL, POINT_VIRGULE, PARENTHESE_OUVRANTE, PARENTHESE_FERMANTE, VIRGULE, MOINS, PLUS, NON, ET, OU, FOIS, DIVISE, CROCHET_OUVRANT, CROCHET_FERMANT, ACCOLADE_OUVRANTE, ACCOLADE_FERMANTE, INFERIEUR
};

char yytext[YYTEXT_MAX];
int yyleng;
int nb_mots_clefs = sizeof (code_mot_clefs) / sizeof (int);
int nb_symboles = sizeof (code_symboles) / sizeof (int);
/* Compter les lignes pour afficher les messages d'erreur avec numero ligne */
int nb_ligne = 1;


/*******************************************************************************
 * Fonction qui ignore les espaces et commentaires. 
 * Renvoie -1 si arrivé à la fin du fichier, 0 si tout va bien 
 ******************************************************************************/
int mange_espaces() {
    char c = fgetc(yyin);
    int comment = 0;
    while (comment || (c == ' ') || (c == '\n') || (c == '\t') || (c == '#')) {
        if (c == '#') {
            comment = 1;
        }
        if (c == '\n') {
            nb_ligne++;
            comment = 0;
        }
        c = fgetc(yyin);
    }
    if (feof(yyin)) {
        return -1;
    }
    ungetc(c, yyin);
    return 0;
}

/*******************************************************************************
 * Lit un caractère et le stocke dans le buffer yytext 
 ******************************************************************************/
char lire_car(void) {
    yytext[yyleng++] = fgetc(yyin);
    yytext[yyleng] = '\0';
    return yytext[yyleng - 1];
}

/*******************************************************************************
 * Remet le dernier caractère lu au buffer clavier et enlève du buffer yytext 
 ******************************************************************************/
void delire_car() {
    char c;
    c = yytext[yyleng - 1];
    yytext[--yyleng] = '\0';
    ungetc(c, yyin);
}

/*******************************************************************************
 * Fonction principale de l'analyseur lexical, lit les caractères de yyin et
 * renvoie les tokens sous forme d'entier. Le code de chaque unité est défini 
 * dans symboles.h sinon (mot clé, idententifiant, etc.). Pour les tokens de 
 * type ID_FCT, ID_VAR et NOMBRE la valeur du token est dans yytext, visible 
 * dans l'analyseur syntaxique.
 ******************************************************************************/
int yylex(void) {
    char c;
    int i;
    yytext[yyleng = 0] = '\0';

    if (mange_espaces() == -1)
        return FIN;

    c = lire_car();

    for (i = 0; i < nb_symboles; i++)
        if (table_symboles[i] == c)
            return code_symboles[i];

    if (is_num(c)) {
        do {
            c = lire_car();
        } while (is_num(c));
        delire_car();

        return NOMBRE;
    }

    if (is_maj(c) || is_min(c)) {
        do {
            if (yyleng >= YYTEXT_MAX - 1) {
                printf("Erreur ligne %d: un nom de fonction fait plus de 99 caractères\n", nb_ligne);
                exit(1);
            }

            c = lire_car();
        } while (is_maj(c) || is_min(c) || (c) == '_');
        delire_car();

        for (i = 0; i < nb_mots_clefs; i++)
            if (strcasecmp(table_mots_clefs[i], yytext) == 0)
                return code_mot_clefs[i];

        return ID_FCT;
    }

    if ((c) == '$') {
        do {
            if (yyleng >= YYTEXT_MAX - 1) {
                printf("Erreur ligne %d: un nom de variable fait plus de 99 caractères\n", nb_ligne);
                exit(1);
            }

            c = lire_car();
        } while (is_maj(c) || is_min(c) || is_num(c) || (c) == '_');
        delire_car();

        return ID_VAR;
    }

    printf("Erreur inconnue ligne %d.\n", nb_ligne);
    exit(1);
}

/*******************************************************************************
 * Fonction auxiliaire appelée par l'analyseur syntaxique tout simplement pour 
 * afficher des messages d'erreur et l'arbre XML 
 ******************************************************************************/
void nom_token(int token, char *nom, char *valeur) {
    int i;

    strcpy(nom, "symbole");
    if (token == POINT_VIRGULE) strcpy(valeur, "POINT_VIRGULE");
    else if (token == PLUS) strcpy(valeur, "PLUS");
    else if (token == MOINS) strcpy(valeur, "MOINS");
    else if (token == FOIS) strcpy(valeur, "FOIS");
    else if (token == DIVISE) strcpy(valeur, "DIVISE");
    else if (token == PARENTHESE_OUVRANTE) strcpy(valeur, "PARENTHESE_OUVRANTE");
    else if (token == PARENTHESE_FERMANTE) strcpy(valeur, "PARENTHESE_FERMANTE");
    else if (token == CROCHET_OUVRANT) strcpy(valeur, "CROCHET_OUVRANT");
    else if (token == CROCHET_FERMANT) strcpy(valeur, "CROCHET_FERMANT");
    else if (token == ACCOLADE_OUVRANTE) strcpy(valeur, "ACCOLADE_OUVRANTE");
    else if (token == ACCOLADE_FERMANTE) strcpy(valeur, "ACCOLADE_FERMANTE");
    else if (token == EGAL) strcpy(valeur, "EGAL");
    else if (token == INFERIEUR) strcpy(valeur, "INFERIEUR");
    else if (token == ET) strcpy(valeur, "ET");
    else if (token == OU) strcpy(valeur, "OU");
    else if (token == NON) strcpy(valeur, "NON");
    else if (token == FIN) strcpy(valeur, "FIN");
    else if (token == VIRGULE) strcpy(valeur, "VIRGULE");

    else if (token == ID_VAR) {
        strcpy(nom, "id_variable");
        strcpy(valeur, yytext);
    } else if (token == ID_FCT) {
        strcpy(nom, "id_fonction");
        strcpy(valeur, yytext);
    } else if (token == NOMBRE) {
        strcpy(nom, "nombre");
        strcpy(valeur, yytext);
    } else {
        strcpy(nom, "mot_clef");
        for (i = 0; i < nb_mots_clefs; i++) {
            if (token == code_mot_clefs[i]) {
                strcpy(valeur, table_mots_clefs[i]);
                break;
            }
        }
    }
}

void get_yytext(char *valeur) {
    strcpy(valeur, yytext);
}

/*******************************************************************************
 * Fonction auxiliaire appelée par le compilo en mode -l, pour tester l'analyseur
 * lexical et, étant donné un programme en entrée, afficher la liste des tokens.
 ******************************************************************************/

void test_yylex_internal(FILE *yyin) {
    int uniteCourante;
    char nom[100];
    char valeur[100];
    do {
        uniteCourante = yylex();
        nom_token(uniteCourante, nom, valeur);
        printf("%s\t%s\t%s\n", yytext, nom, valeur);
    } while (uniteCourante != FIN);
}
