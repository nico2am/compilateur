#ifndef ANALYSEUR_SYNTAXIQUE_H
#define	ANALYSEUR_SYNTAXIQUE_H

#include "affiche_arbre_abstrait.h"
#include "syntabs.h"

void test_analyseur();
void affiche_unite_courante();
void gen_erreur(const char *fonction, char *message);

n_prog* programme();
n_l_dec* optDecVariables();
n_l_dec* listeDecVariables();
n_l_dec* listeDecVariablesBis();
n_dec* declarationVariable();
int optTailleTableau();
n_l_dec* listeDecFonctions();
n_dec* declarationFonction();
n_l_dec* listeParam();
n_l_dec* optListeDecVariables();
n_instr* instruction();
n_instr* instructionAffect();
n_instr* instructionBloc();
n_l_instr* listeInstructions();
n_instr* instructionSi();
n_instr* optSinon();
n_instr* instructionTantque();
n_instr* instructionAppel();
n_instr* instructionRetour();
n_instr* instructionEcriture();
n_instr* instructionVide();
n_exp* expression();
n_exp* expressionBis();
n_exp* conjonction();
n_exp* conjonctionBis();
n_exp* negation();
n_exp* comparaison();
n_exp* comparaisonBis();
n_exp* expArith();
n_exp* expArithBis();
n_exp* terme();
n_exp* termeBis();
n_exp* facteur();
n_var* var();
n_exp* optIndice();
n_appel* appelFct();
n_l_exp* listeExpressions();
n_l_exp* listeExpressionsBis();

#endif	/* ANALYSEUR_SYNTAXIQUE_H */

