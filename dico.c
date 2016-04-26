#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dico.h"

dico_ dico;
int contexte;
int adresseLocaleCourante;
int adresseArgumentCourant;

/*-------------------------------------------------------------------------*/
void contexte_globale() {
    contexte = C_VARIABLE_GLOBALE;
}

void contexte_locale() {
    contexte = C_VARIABLE_LOCALE;
}

void contexte_argument() {
    contexte = C_ARGUMENT;
}

void entree_fonction() {
    dico.base = dico.sommet;
    contexte = C_VARIABLE_LOCALE;
    adresseLocaleCourante = 0;
    adresseArgumentCourant = 0;
}

void sortie_fonction() {
    dico.sommet = dico.base;
    dico.base = 0;
    contexte = C_VARIABLE_GLOBALE;
}

/*-------------------------------------------------------------------------*/

int ajoute_identificateur(char *identif, int classe, int type, int adresse, int complement) {
    dico.tab[dico.sommet].identif = strdup(identif);
    dico.tab[dico.sommet].classe = classe;
    dico.tab[dico.sommet].type = type;
    dico.tab[dico.sommet].adresse = adresse;
    dico.tab[dico.sommet].complement = complement;
    dico.sommet++;

    if (dico.sommet == maxDico) {
        fprintf(stderr, "attention, plus de place dans le dictionnaire des \
                     identificateurs, augmenter maxDico\n");
        exit(1);
    }
    /* affiche_dico(); */
    return dico.sommet - 1;
}

/*-------------------------------------------------------------------------*/

int recherche_executable(char *identif) {
    int i;
    for (i = dico.sommet - 1; i >= 0; i--) {
        if (!strcmp(identif, dico.tab[i].identif))
            return i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

int recherche_declarative(char *identif) {
    int i;
    for (i = dico.base; i < dico.sommet; i++) {
        if (!strcmp(identif, dico.tab[i].identif))
            return i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

void affiche_dico(void) {
    int i;
    printf("------------------------------------------\n");
    printf("base = %d\n", dico.base);
    printf("sommet = %d\n", dico.sommet);
    for (i = 0; i < dico.sommet; i++) {
        printf("%d ", i);
        printf("%s ", dico.tab[i].identif);
        if (dico.tab[i].classe == C_VARIABLE_GLOBALE)
            printf("GLOBALE ");
        else
            if (dico.tab[i].classe == C_VARIABLE_LOCALE)
            printf("LOCALE ");
        else
            if (dico.tab[i].classe == C_ARGUMENT)
            printf("ARGUMENT ");

        if (dico.tab[i].type == T_ENTIER)
            printf("ENTIER ");
        else if (dico.tab[i].type == T_TABLEAU_ENTIER)
            printf("TABLEAU ");
            /*     else if(dico.tab[i].type == _ARGUMENT) */
            /*       printf("ARGUMENT "); */
        else if (dico.tab[i].type == T_FONCTION)
            printf("FONCTION ");

        printf("%d ", dico.tab[i].adresse);
        printf("%d\n", dico.tab[i].complement);
    }
    printf("------------------------------------------\n");
}
