#ifndef __DICO__
#define __DICO__

#include "syntabs.h"

#define maxDico 100

#define C_VARIABLE_GLOBALE 1
#define C_VARIABLE_LOCALE 2
#define C_ARGUMENT 3

#define T_ENTIER 1
#define T_TABLEAU_ENTIER 2
#define T_FONCTION 3

typedef struct {
    char *identif;
    int classe;
    int type;
    int adresse;
    int complement;
} desc_identif;

typedef struct {
    desc_identif tab[maxDico];
    int base;
    int sommet;
} dico_;

void entree_fonction();
void sortie_fonction();

void contexte_globale();
void contexte_locale();
void contexte_argument();

int ajoute_identificateur(char *identif, int classe, int type, int adresse, int complement);
int recherche_executable(char *identif);
int recherche_declarative(char *identif);
void affiche_dico(void);

extern dico_ dico;
extern int contexte;
extern int adresseLocaleCourante;
extern int adresseArgumentCourant;
#endif
