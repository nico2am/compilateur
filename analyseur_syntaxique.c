#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "symboles.h"
#include "analyseur_lexical.h"
#include "analyseur_syntaxique.h"
#include "util.h"
#include "suivants.h"
#include "premiers.h"
#include "syntabs.h"
#include "dico.h"
#include "generation_code.h"

int unite_courante;
extern int trace_synt;
extern int trace_asynt;
extern int trace_dico;
extern int trace_mips;
extern int nb_ligne;
extern char *filename;

#define balise_ouvrante() affiche_balise_ouvrante(__func__, trace_synt);
#define balise_fermante() affiche_balise_fermante(__func__, trace_synt);

void test_analyseur() {
    initialise_premiers();
    initialise_suivants();
 
    unite_courante = yylex();

    n_prog* p = programme();

    if (trace_asynt == 1)
        affiche_n_prog(p);

    if (trace_mips == 1 || trace_dico == 1)
        parcour_n_prog(p);
        
    if (unite_courante != FIN) {
        gen_erreur(__func__, NULL);
    }
}

void affiche_unite_courante() {
    char nom[100];
    char valeur[100];
    nom_token(unite_courante, nom, valeur);
    affiche_balise_ouvrante(nom, trace_synt);
    affiche_texte(valeur, trace_synt);
    affiche_balise_fermante(nom, trace_synt);
}

void gen_erreur(const char *fonction, char *message) {
    (void) message;
    char *filename_local = (strrchr(filename, '/') == NULL ? filename : strrchr(filename, '/') + 1);
    printf("%s: erreur de syntaxe\n"
            "\tligne: %d\n"
            "\t%s\n"
            "\t[fonction dans le compilateur: %s]\n", filename_local, nb_ligne, message, fonction);
    exit(1);
}

n_prog* programme() {
    balise_ouvrante();

    n_l_dec *variables = optDecVariables(NULL);
    n_l_dec *fonctions = listeDecFonctions();

    balise_fermante();

    return cree_n_prog(variables, fonctions);
}

n_l_dec* optDecVariables() {
    balise_ouvrante();
    n_l_dec *resultat = NULL;

    if (est_premier(unite_courante, _listeDecVariables_)) {
        resultat = listeDecVariables();
        if (unite_courante != POINT_VIRGULE)
            gen_erreur(__func__, "Point virgule manquant");
        affiche_unite_courante();
        unite_courante = yylex();
    } else if (est_suivant(unite_courante, _optDecVariables_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_l_dec* listeDecVariables() {
    balise_ouvrante();
    n_l_dec *resultat = NULL;

    if (est_premier(unite_courante, _declarationVariable_)) {
        n_dec *tete = declarationVariable();
        n_l_dec *queue = listeDecVariablesBis();
        resultat = cree_n_l_dec(tete, queue);
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_l_dec* listeDecVariablesBis() {
    balise_ouvrante();
    n_l_dec *resultat = NULL;

    if (unite_courante == VIRGULE) {
        affiche_unite_courante();
        unite_courante = yylex();
        n_dec *tete = declarationVariable();
        n_l_dec *queue = listeDecVariablesBis();
        resultat = cree_n_l_dec(tete, queue);
    } else if (est_suivant(unite_courante, _listeDecVariablesBis_)) {

    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_dec* declarationVariable() {
    balise_ouvrante();
    n_dec *resultat = NULL;

    int taille;
    char *nom = malloc(YYTEXT_MAX * sizeof *nom);

    if (unite_courante != ENTIER)
        gen_erreur(__func__, "Mot clé 'entier' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    if (unite_courante != ID_VAR)
        gen_erreur(__func__, "Variable manquante après déclaration du mot 'entier'");

    get_yytext(nom);

    affiche_unite_courante();
    unite_courante = yylex();

    taille = optTailleTableau();

    if (taille == -1) {
        resultat = cree_n_dec_var(nom);
    } else {
        resultat = cree_n_dec_tab(nom, taille);
    }

    balise_fermante();
    return resultat;
}

int optTailleTableau() {
    balise_ouvrante();
    int resultat = -1;
    char *taille_str = malloc(YYTEXT_MAX * sizeof *taille_str);

    if (unite_courante == CROCHET_OUVRANT) {
        affiche_unite_courante();
        unite_courante = yylex();
        if (unite_courante != NOMBRE)
            gen_erreur(__func__, "La taille d'un tableau doit être un nombre");

        get_yytext(taille_str);
        resultat = atoi(taille_str);

        affiche_unite_courante();
        unite_courante = yylex();

        if (unite_courante != CROCHET_FERMANT)
            gen_erreur(__func__, "crochet fermant manquant");

        affiche_unite_courante();
        unite_courante = yylex();
    } else if (est_suivant(unite_courante, _optTailleTableau_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_l_dec* listeDecFonctions() {
    balise_ouvrante();
    n_l_dec *resultat = NULL;

    if (est_premier(unite_courante, _declarationFonction_)) {
        n_dec *tete = declarationFonction();
        n_l_dec *queue = listeDecFonctions();
        resultat = cree_n_l_dec(tete, queue);
    } else if (est_suivant(unite_courante, _listeDecFonctions_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_dec* declarationFonction() {
    balise_ouvrante();
   
    char *nom = malloc(YYTEXT_MAX * sizeof *nom);

    if (unite_courante != ID_FCT)
        gen_erreur(__func__, "nom de function manquant");

    get_yytext(nom);

    affiche_unite_courante();
    unite_courante = yylex();

    n_l_dec *param = listeParam();
    n_l_dec *variables = optDecVariables();
    n_instr *corps = instructionBloc();

    balise_fermante();
    return cree_n_dec_fonc(nom, param, variables, corps);
}

n_l_dec* listeParam() {
    balise_ouvrante();
    n_l_dec *resulat = NULL;

    if (unite_courante != PARENTHESE_OUVRANTE)
        gen_erreur(__func__, "Parenthese ouvrante manquate");

    affiche_unite_courante();
    unite_courante = yylex();

    resulat = optListeDecVariables();

    if (unite_courante != PARENTHESE_FERMANTE)
        gen_erreur(__func__, "Parenthese fermante manquate");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return resulat;
}

n_l_dec* optListeDecVariables() {
    balise_ouvrante();
    n_l_dec *resulat = NULL;

    if (est_premier(unite_courante, _listeDecVariables_)) {
        resulat = listeDecVariables();
    } else if (est_suivant(unite_courante, _optListeDecVariables_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resulat;
}

n_instr* instruction() {
    balise_ouvrante();
    n_instr *resultat;

    if (est_premier(unite_courante, _instructionAffect_)) {
        resultat = instructionAffect();
    } else if (est_premier(unite_courante, _instructionBloc_)) {
        resultat = instructionBloc();
    } else if (est_premier(unite_courante, _instructionSi_)) {
        resultat = instructionSi();
    } else if (est_premier(unite_courante, _instructionTantque_)) {
        resultat = instructionTantque();
    } else if (est_premier(unite_courante, _instructionAppel_)) {
        resultat = instructionAppel();
    } else if (est_premier(unite_courante, _instructionRetour_)) {
        resultat = instructionRetour();
    } else if (est_premier(unite_courante, _instructionEcriture_)) {
        resultat = instructionEcriture();
    } else if (est_premier(unite_courante, _instructionVide_)) {
        resultat = instructionVide();
    } else {
        gen_erreur(__func__, "instruction manquante");
    }

    balise_fermante();
    return resultat;
}

n_instr* instructionAffect() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _var_))
        gen_erreur(__func__, "variable manquante");

    n_var *variable = var();

    if (unite_courante != EGAL)
        gen_erreur(__func__, "symbole egal manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *exp = expression();

    if (unite_courante != POINT_VIRGULE)
        gen_erreur(__func__, "point virgule manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_affect(variable, exp);
}

n_instr* instructionBloc() {
    balise_ouvrante();

    if (unite_courante != ACCOLADE_OUVRANTE)
        gen_erreur(__func__, "accolade ouvrante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    n_l_instr *liste = listeInstructions();

    if (unite_courante != ACCOLADE_FERMANTE)
        gen_erreur(__func__, "accolade fermante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_bloc(liste);
}

n_l_instr* listeInstructions() {
    balise_ouvrante();
    n_l_instr *resulat = NULL;

    if (est_premier(unite_courante, _instruction_)) {
        n_instr *tete = instruction();
        n_l_instr *queue = listeInstructions();
        resulat = cree_n_l_instr(tete, queue);
    } else if (est_suivant(unite_courante, _listeInstructions_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resulat;
}

n_instr* instructionSi() {
    balise_ouvrante();

    if (unite_courante != SI)
        gen_erreur(__func__, "mot clé 'si' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *test = expression();

    if (unite_courante != ALORS)
        gen_erreur(__func__, "mot clé 'alors' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    n_instr *alors = instructionBloc();
    n_instr *sinon = optSinon();

    balise_fermante();
    return cree_n_instr_si(test, alors, sinon);
}

n_instr* optSinon() {
    balise_ouvrante();
    n_instr *resultat = NULL;

    if (unite_courante == SINON) {
        affiche_unite_courante();
        unite_courante = yylex();
        resultat = instructionBloc();
    } else if (est_suivant(unite_courante, _optSinon_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_instr* instructionTantque() {
    balise_ouvrante();

    if (unite_courante != TANTQUE)
        gen_erreur(__func__, "mot clé 'tantque' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *test = expression();

    if (unite_courante != FAIRE)
        gen_erreur(__func__, "mot clé 'faire' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    n_instr *faire = instructionBloc();

    balise_fermante();
    return cree_n_instr_tantque(test, faire);
}

n_instr* instructionAppel() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _appelFct_))
        gen_erreur(__func__, "mot clé 'alors' manquant");

    n_appel *appel = appelFct();

    if (unite_courante != POINT_VIRGULE)
        gen_erreur(__func__, "point virgule manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_appel(appel);
}

n_instr* instructionRetour() {
    balise_ouvrante();

    if (unite_courante != RETOUR)
        gen_erreur(__func__, "mot clé 'retour' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *exp = expression();

    if (unite_courante != POINT_VIRGULE)
        gen_erreur(__func__, "point virgule manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_retour(exp);
}

n_instr* instructionEcriture() {
    balise_ouvrante();

    if (unite_courante != ECRIRE)
        gen_erreur(__func__, "mot clé 'ecrire' manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    if (unite_courante != PARENTHESE_OUVRANTE)
        gen_erreur(__func__, "parenthese ouvrante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *exp = expression();

    if (unite_courante != PARENTHESE_FERMANTE)
        gen_erreur(__func__, "parenthese fermante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    if (unite_courante != POINT_VIRGULE)
        gen_erreur(__func__, "point virgule manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_ecrire(exp);
}

n_instr* instructionVide() {
    balise_ouvrante();

    if (unite_courante != POINT_VIRGULE)
        gen_erreur(__func__, "point virgule manquant");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_instr_vide();
}

n_exp* expression() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _conjonction_))
        gen_erreur(__func__, "expression manquante");

    n_exp *op1 = conjonction();
    n_exp *op2 = expressionBis(op1);

    balise_fermante();
    return op2;
}

n_exp* expressionBis(n_exp *pere) {
    balise_ouvrante();
    n_exp *resultat = pere;

    n_exp *op1;
    n_exp *fils;

    if (unite_courante == OU) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = conjonction();
        fils = cree_n_exp_op(ou, pere, op1);
        resultat = expressionBis(fils);
    } else if (est_suivant(unite_courante, _expressionBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_exp* conjonction() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _negation_))
        gen_erreur(__func__, "conjonction manquante");

    n_exp *op1 = negation();
    n_exp *op2 = conjonctionBis(op1);

    balise_fermante();
    return op2;
}

n_exp* conjonctionBis(n_exp *pere) {
    balise_ouvrante();
    n_exp *resultat = pere;

    n_exp *op1;
    n_exp *fils;

    if (unite_courante == ET) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = negation();
        fils = cree_n_exp_op(ou, pere, op1);
        resultat = conjonctionBis(fils);
    } else if (est_suivant(unite_courante, _conjonctionBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_exp* negation() {
    balise_ouvrante();
    n_exp *resulat = NULL;

    if (unite_courante == NON) {
        affiche_unite_courante();
        unite_courante = yylex();
        n_exp *op1 = comparaison();
        resulat = cree_n_exp_op(non, op1, NULL);
    } else if (est_premier(unite_courante, _comparaison_)) {
        resulat = comparaison();
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resulat;
}

n_exp* comparaison() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _expArith_))
        gen_erreur(__func__, "comparaison manquante");

    n_exp *op1 = expArith();
    n_exp *op2 = comparaisonBis(op1);

    balise_fermante();
    return op2;
}

n_exp* comparaisonBis(n_exp *pere) {
    balise_ouvrante();
    n_exp *resultat = pere;

    n_exp *op1;
    n_exp *fils;

    if (unite_courante == EGAL) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = expArith();
        fils = cree_n_exp_op(egal, pere, op1);
        resultat = comparaisonBis(fils);
    } else if (unite_courante == INFERIEUR) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = expArith();
        fils = cree_n_exp_op(inf, pere, op1);
        resultat = comparaisonBis(fils);
    } else if (est_suivant(unite_courante, _comparaisonBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_exp* expArith() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _terme_))
        gen_erreur(__func__, "expression arithmétique manquante");

    n_exp *op1 = terme();
    n_exp *op2 = expArithBis(op1);

    balise_fermante();
    return op2;
}

n_exp* expArithBis(n_exp *pere) {
    balise_ouvrante();
    n_exp *resultat = pere;

    n_exp *op1;
    n_exp *fils;

    if (unite_courante == PLUS) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = terme();
        fils = cree_n_exp_op(plus, pere, op1);
        resultat = expArithBis(fils);
    } else if (unite_courante == MOINS) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = terme();
        fils = cree_n_exp_op(moins, pere, op1);
        resultat = expArithBis(fils);
    } else if (est_suivant(unite_courante, _expArithBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_exp* terme() {
    balise_ouvrante();

    if (!est_premier(unite_courante, _facteur_))
        gen_erreur(__func__, "terme manquant");

    n_exp *op1 = facteur();
    n_exp *op2 = termeBis(op1);

    balise_fermante();
    return op2;
}

n_exp* termeBis(n_exp *pere) {
    balise_ouvrante();
    n_exp *resultat = pere;

    n_exp *op1;
    n_exp *fils;

    if (unite_courante == FOIS) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = facteur();
        fils = cree_n_exp_op(fois, pere, op1);
        resultat = termeBis(fils);
    } else if (unite_courante == DIVISE) {
        affiche_unite_courante();
        unite_courante = yylex();
        op1 = facteur();
        fils = cree_n_exp_op(divise, pere, op1);
        resultat = termeBis(fils);
    } else if (est_suivant(unite_courante, _termeBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_exp* facteur() {
    balise_ouvrante();
    n_exp *resultat = NULL;

    if (unite_courante == PARENTHESE_OUVRANTE) {
        affiche_unite_courante();
        unite_courante = yylex();

        resultat = expression();

        if (unite_courante != PARENTHESE_FERMANTE)
            gen_erreur(__func__, "parenthese fermante manquante");

        affiche_unite_courante();
        unite_courante = yylex();
    } else if (unite_courante == NOMBRE) {
        char *entier_str = malloc(YYTEXT_MAX * sizeof *entier_str);
        get_yytext(entier_str);
        int entier = atoi(entier_str);

        affiche_unite_courante();
        unite_courante = yylex();

        resultat = cree_n_exp_entier(entier);
    } else if (est_premier(unite_courante, _appelFct_)) {
        n_appel *app = appelFct();
        resultat = cree_n_exp_appel(app);
    } else if (est_premier(unite_courante, _var_)) {
        n_var *variable = var();
        resultat = cree_n_exp_var(variable);
    } else if (unite_courante == LIRE) {
        affiche_unite_courante();
        unite_courante = yylex();

        if (unite_courante != PARENTHESE_OUVRANTE)
            gen_erreur(__func__, "parenthese ouvrante manquante");

        affiche_unite_courante();
        unite_courante = yylex();

        if (unite_courante != PARENTHESE_FERMANTE)
            gen_erreur(__func__, "parenthese fermante manquante");

        affiche_unite_courante();
        unite_courante = yylex();
        resultat = cree_n_exp_lire();
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_var* var() {
    balise_ouvrante();

    if (unite_courante != ID_VAR)
        gen_erreur(__func__, "variable manquante");

    char *nom = malloc(YYTEXT_MAX * sizeof *nom);
    get_yytext(nom);

    affiche_unite_courante();
    unite_courante = yylex();

    n_exp *indice = optIndice();

    balise_fermante();
    return indice == NULL ? cree_n_var_simple(nom) : cree_n_var_indicee(nom, indice);
}

n_exp* optIndice() {
    balise_ouvrante();
    n_exp *resultat = NULL;

    if (unite_courante == CROCHET_OUVRANT) {
        affiche_unite_courante();
        unite_courante = yylex();

        resultat = expression();

        if (unite_courante != CROCHET_FERMANT)
            gen_erreur(__func__, "crochet fermant manquant");

        affiche_unite_courante();
        unite_courante = yylex();
    } else if (est_suivant(unite_courante, _optIndice_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_appel* appelFct() {
    balise_ouvrante();

    char *fonction = malloc(YYTEXT_MAX * sizeof *fonction);
    
    if (unite_courante != ID_FCT)
        gen_erreur(__func__, "fonction manquante");

    get_yytext(fonction);

    affiche_unite_courante();
    unite_courante = yylex();

    if (unite_courante != PARENTHESE_OUVRANTE)
        gen_erreur(__func__, "parenthese ouvrante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    n_l_exp *exps = listeExpressions();

    if (unite_courante != PARENTHESE_FERMANTE)
        gen_erreur(__func__, "parenthese fermante manquante");

    affiche_unite_courante();
    unite_courante = yylex();

    balise_fermante();
    return cree_n_appel(fonction, exps);
}

n_l_exp* listeExpressions() {
    balise_ouvrante();
    n_l_exp *resultat = NULL;

    if (est_premier(unite_courante, _expression_)) {
        n_exp *tete = expression();
        n_l_exp *queue = listeExpressionsBis();
        resultat = cree_n_l_exp(tete, queue);
    } else if (est_suivant(unite_courante, _listeExpressions_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

n_l_exp* listeExpressionsBis() {
    balise_ouvrante();
    n_l_exp *resultat = NULL;

    if (unite_courante == VIRGULE) {
        affiche_unite_courante();
        unite_courante = yylex();
        n_exp *tete = expression();
        n_l_exp *queue = listeExpressionsBis();
        resultat = cree_n_l_exp(tete, queue);
    } else if (est_suivant(unite_courante, _listeExpressionsBis_)) {
        // ok
    } else {
        gen_erreur(__func__, NULL);
    }

    balise_fermante();
    return resultat;
}

