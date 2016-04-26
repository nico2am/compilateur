#ifndef __PARCOUR_ARBRE_ABSTRAIT__
#define __PARCOUR_ARBRE_ABSTRAIT__

#include "syntabs.h"

void parcour_n_prog(n_prog *n);
void parcour_l_instr(n_l_instr *n);
void parcour_instr(n_instr *n);
void parcour_instr_si(n_instr *n);
void parcour_instr_tantque(n_instr *n);
void parcour_instr_faire(n_instr *n); /* MODIFIE POUR EVAL */
void parcour_instr_pour(n_instr *n); /* MODIFIE POUR EVAL */
void parcour_instr_affect(n_instr *n);
void parcour_instr_appel(n_instr *n);
void parcour_instr_retour(n_instr *n);
void parcour_instr_ecrire(n_instr *n);
void parcour_l_exp(n_l_exp *n);
void parcour_exp(n_exp *n);
void parcour_varExp(n_exp *n);
void parcour_opExp(n_exp *n);
void parcour_intExp(n_exp *n);
void parcour_lireExp(n_exp *n);
void parcour_appelExp(n_exp *n);
void parcour_l_dec(n_l_dec *n);
void parcour_dec(n_dec *n);
void parcour_foncDec(n_dec *n);
void parcour_varDec(n_dec *n);
void parcour_tabDec(n_dec *n);
void parcour_var(n_var *n, int active);
void parcour_var_simple(n_var *n, int active);
void parcour_var_indicee(n_var *n, int active);
void parcour_appel(n_appel *n);

#endif

