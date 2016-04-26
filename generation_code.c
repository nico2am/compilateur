#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include "syntabs.h"
#include "generation_code.h"
#include "util.h"
#include "dico.h"
#include "analyseur_syntaxique.h"

#define TAILLE 4

#define TRUE 1
#define FALSE 0

extern char *filename;

extern int trace_dico;
extern int trace_mips;

int nb_globale = 0;
int nb_locale = 0;
int nb_argument = 0;

int etiquette = 0;

int retour = FALSE;

int nb_mips_args = 0;

void gen_erreur_parcour(const char *format, ...) {
    char *filename_local = (strrchr(filename, '/') == NULL ? filename : strrchr(filename, '/') + 1);
    va_list ap;

    printf("%s: erreur: ", filename_local);

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    printf("\n");

    exit(1); /* ARRET DU PARCOUR */
}

void empile(char *reg) {
    printf("\tsubi\t$sp, $sp, %d\t\t # empile registre\n", TAILLE);
    printf("\tsw\t%s, 0($sp)\n", reg);
}

void depile(char *reg) {
    printf("\tlw\t%s, 0($sp)\t\t # depile vers registre\n", reg);
    printf("\taddi\t$sp, $sp, %d\n", TAILLE);
}

int calcul_nb_n_l_exp(n_l_exp* var) {
    int taille = 0;
    n_l_exp* tmp = var;

    while (tmp != NULL) {
        tmp = tmp->queue;
        taille++;
    }

    return taille;
}

int calcul_nb_n_l_dec(n_l_dec* var) {
    int taille = 0;
    n_l_dec* tmp = var;

    while (tmp != NULL) {
        tmp = tmp->queue;
        taille++;
    }

    return taille;
}

void parcour_n_prog(n_prog *n) {
    contexte_globale();

    if (trace_mips) {
        printf("\t.data\n");
    }

    parcour_l_dec(n->variables);

    if (trace_mips) {
        printf("\n\t.text\n"
                "__start:\n"
                "\tjal\tmain\n"
                "\tli\t$v0, 10\n"
                "\tsyscall\n");
    }

    parcour_l_dec(n->fonctions);

    int index = recherche_executable("main");

    if (index == -1) {
        gen_erreur_parcour("la fonction main n'est pas déclaréé.");
    }

    if (dico.tab[index].complement != 0) {
        gen_erreur_parcour("la fonction main ne doit pas avoir d'arguments.");
    }

}

void parcour_l_instr(n_l_instr *n) {
    if (n) {
        parcour_instr(n->tete);
        parcour_l_instr(n->queue);

    }
}

void parcour_instr(n_instr *n) {
    if (n) {
        if (n->type == blocInst) parcour_l_instr(n->u.liste);
        else if (n->type == affecteInst) parcour_instr_affect(n);
        else if (n->type == siInst) parcour_instr_si(n);
        else if (n->type == tantqueInst) parcour_instr_tantque(n);
        else if (n->type == faireInst) parcour_instr_faire(n);
        else if (n->type == pourInst) parcour_instr_pour(n);
        else if (n->type == appelInst) parcour_instr_appel(n);
        else if (n->type == retourInst) parcour_instr_retour(n);
        else if (n->type == ecrireInst) parcour_instr_ecrire(n);
    }
}

void parcour_instr_si(n_instr *n) {
    int e_suite_si = etiquette++;
    int e_sinon = etiquette++;

    parcour_exp(n->u.si_.test);

    if (trace_mips) {
        depile("$t0");
        if (n->u.si_.sinon) {
            printf("\tbeq\t$t0, $zero, e%d\n", e_sinon);
        } else {
            printf("\tbeq\t$t0, $zero, e%d\n", e_suite_si);
        }
    }

    parcour_instr(n->u.si_.alors);

    if (n->u.si_.sinon) {
        if (trace_mips) {
            printf("\tj\te%d\n"
                    "e%d:\n", e_suite_si, e_sinon);
        }
        parcour_instr(n->u.si_.sinon);
    }

    if (trace_mips) {
        printf("e%d:\n", e_suite_si);
    }
}

void parcour_instr_tantque(n_instr *n) {
    int e_test = etiquette++;
    int e_suite_tantque = etiquette++;

    if (trace_mips) {
        printf("e%d:\n", e_test);
    }
    parcour_exp(n->u.tantque_.test);

    if (trace_mips) {
        depile("$t0");
        printf("\tbeq\t$t0, $zero, e%d\n", e_suite_tantque);
    }
    parcour_instr(n->u.tantque_.faire);

    if (trace_mips) {
        printf("\tj\te%d\n"
                "e%d:\n", e_test, e_suite_tantque);
    }
}

void parcour_instr_faire(n_instr *n) /* MODIFIE POUR EVAL */ { /* MODIFIE POUR EVAL */
    /* MODIFIE POUR EVAL */
    parcour_instr(n->u.faire_.faire); /* MODIFIE POUR EVAL */
    parcour_exp(n->u.faire_.test); /* MODIFIE POUR EVAL */
    /* MODIFIE POUR EVAL */
} /* MODIFIE POUR EVAL */

void parcour_instr_pour(n_instr *n) /* MODIFIE POUR EVAL */ { /* MODIFIE POUR EVAL */
    /* MODIFIE POUR EVAL */
    parcour_instr(n->u.pour_.init); /* MODIFIE POUR EVAL */
    parcour_exp(n->u.pour_.test); /* MODIFIE POUR EVAL */
    parcour_instr(n->u.pour_.faire); /* MODIFIE POUR EVAL */
    parcour_instr(n->u.pour_.incr); /* MODIFIE POUR EVAL */
    /* MODIFIE POUR EVAL */
} /* MODIFIE POUR EVAL */

void parcour_instr_affect(n_instr *n) {
    parcour_exp(n->u.affecte_.exp);
    parcour_var(n->u.affecte_.var, FALSE);

    int index = recherche_executable(n->u.affecte_.var->nom);

    if (index == -1)
        return;

    if (trace_mips) {
        if (n->u.affecte_.var->type == simple) {
            depile("$t1");
            switch (dico.tab[index].classe) {
                case C_VARIABLE_GLOBALE:
                    printf("\tsw\t$t1, %s\t\t # stocke variable (globale)\n", n->u.affecte_.var->nom);
                    break;
                case C_VARIABLE_LOCALE:
                    printf("\tsw\t$t1, %d($fp)\t\t # stocke variable (locale)\n", -8 - dico.tab[index].adresse);
                    break;
                case C_ARGUMENT:
                    printf("\tsw\t$t1, %d($fp)\t\t # stocke variable (argument)\n", 4 * nb_argument - dico.tab[index].adresse);
                    break;
            }
        } else {
            depile("$t0");
            printf("\tadd\t$t0, $t0, $t0\n"
                    "\tadd\t$t0, $t0, $t0\n");
            depile("$t1");

            printf("\tsw\t$t1, %s($t0)\t\t # stocke variable (tableau)\n", n->u.affecte_.var->nom);
        }
    }
}

void parcour_instr_appel(n_instr *n) {
    parcour_appel(n->u.appel);

    if (trace_mips) {
        printf("\taddi\t$sp, $sp, %d\t\t # valeur de retour ignoree\n", TAILLE);
    }
}

void parcour_appel(n_appel *n) {
    int index = recherche_executable(n->fonction);

    if (index < 0) {
        gen_erreur_parcour("la fonction %s n'est pas déclarée", n->fonction);
    }

    if (trace_mips) {
        printf("\tsubi\t$sp, $sp, %d\t\t # allocation valeur de retour\n", TAILLE);
    }

    int nb_args = calcul_nb_n_l_exp(n->args);

    if (nb_args != dico.tab[index].complement) {
        gen_erreur_parcour("le nombre d'arguments de la function %s est invalides", n->fonction);
    }

    parcour_l_exp(n->args);

    if (trace_mips) {
        printf("\tjal\t%s\n", n->fonction);

        if (nb_args > 0) {
            printf("\taddi\t$sp, $sp, %d\t\t # desallocation parametres\n", nb_args * TAILLE);
        }
    }

}

void parcour_instr_retour(n_instr *n) {
    parcour_exp(n->u.retour_.expression);

    if (trace_mips) {
        depile("$t0");
        printf("\tsw\t$t0, %d($fp)\t\t # ecriture de la valeur de retour\n", 4 * (nb_argument + 1));
        depile("$ra");
        depile("$fp");
        printf("\tjr\t$ra\n");
    }

    retour = TRUE;
}

void parcour_instr_ecrire(n_instr *n) {
    parcour_exp(n->u.ecrire_.expression);

    if (trace_mips) {
        depile("$a0");
        printf("\tli\t$v0, 1\n"
                "\tsyscall\t\t # ecriture\n"
                "\tli\t$a0, '\\n'\n"
                "\tli\t$v0, 11\n"
                "\tsyscall\t\t # ecrire char\n");
    }
}

void parcour_l_exp(n_l_exp *n) {
    if (n) {
        if (trace_mips) {
            printf("\t\t\t\t # empile arg %d\n", nb_mips_args++);
        }

        parcour_exp(n->tete);
        parcour_l_exp(n->queue);
        nb_mips_args = 0;
    }
}

void parcour_exp(n_exp *n) {
    if (n->type == varExp) parcour_varExp(n);
    else if (n->type == opExp) parcour_opExp(n);
    else if (n->type == intExp) parcour_intExp(n);
    else if (n->type == appelExp) parcour_appelExp(n);
    else if (n->type == lireExp) parcour_lireExp(n);
}

void parcour_varExp(n_exp *n) {
    parcour_var(n->u.var, TRUE);
}

void parcour_opExp(n_exp *n) {
    if (n->u.opExp_.op1 != NULL) {
        parcour_exp(n->u.opExp_.op1);
    }

    if (n->u.opExp_.op2 != NULL) {
        parcour_exp(n->u.opExp_.op2);
    }

    if (trace_mips) {
        if (n->u.opExp_.op2 != NULL) {
            depile("$t1");
        }

        if (n->u.opExp_.op1 != NULL) {
            depile("$t0");
        }

        switch (n->u.opExp_.op) {
            case plus:
                printf("\tadd\t$t2, $t0, $t1\n");
                empile("$t2");
                break;
            case moins:
                printf("\tsub\t$t2, $t0, $t1\n");
                empile("$t2");
                break;
            case fois:
                printf("\tmult\t$t0, $t1\n"
                        "\tmflo\t$t2\n");
                empile("$t2");
                break;
            case divise:
                printf("\tdiv\t$t0, $t1\n"
                        "\tmflo\t$t2\n");
                empile("$t2");
                break;
            case egal:
                printf("\tli\t$t2, -1\t\t # egal\n"
                        "\tbeq\t$t0, $t1, e%d\n"
                        "\tli\t$t2, 0\n"
                        "e%d:\n", etiquette, etiquette);
                etiquette++;
                empile("$t2");
                break;
            case diff:
                printf("\tli\t$t2, -1\t\t # diff\n"
                        "\tbne\t$t0, $t1, e%d\n"
                        "\tli\t$t2, 0\n"
                        "e%d:\n", etiquette, etiquette);
                etiquette++;
                empile("$t2");
                break;
            case inf:
                printf("\tli\t$t2, -1\t\t # inf\n"
                        "\tblt\t$t0, $t1, e%d\n"
                        "\tli\t$t2, 0\n"
                        "e%d:\n", etiquette, etiquette);
                etiquette++;
                empile("$t2");
                break;
            case ou:
                printf("\tor\t$t2, $t0, $t1\n");
                empile("$t2");
                break;
            case et:
                printf("\tand\t$t2, $t0, $t1\n");
                empile("$t2");
                break;
            case non:
                printf("\tnot\t$t2, $t0\n");
                empile("$t2");
                break;
            default:
                break;
        }
    }
}

void parcour_intExp(n_exp * n) {
    if (trace_mips) {
        printf("\tli\t$t0, %d\n", n->u.entier);
        empile("$t0");
    }
}

void parcour_lireExp(n_exp * n) {
    if (trace_mips) {
        printf("\tli\t$v0, 5\n");
        printf("\tsyscall\t\t # lecture\n");
        empile("$v0");
    }
}

void parcour_appelExp(n_exp * n) {
    parcour_appel(n->u.appel);
}

void parcour_l_dec(n_l_dec * n) {
    if (n) {
        parcour_dec(n->tete);
        parcour_l_dec(n->queue);
    }
}

void parcour_dec(n_dec * n) {
    if (n) {
        if (n->type == foncDec) {
            parcour_foncDec(n);
        } else if (n->type == varDec) {
            parcour_varDec(n);
        } else if (n->type == tabDec) {
            parcour_tabDec(n);
        }
    }
}

void parcour_foncDec(n_dec * n) {
    int index = recherche_declarative(n->nom);

    if (index >= 0) {
        gen_erreur_parcour("la function %s est déjà déclarée\n", n->nom);
    }

    int nb_params = calcul_nb_n_l_dec(n->u.foncDec_.param);
    int nb_variables = calcul_nb_n_l_dec(n->u.foncDec_.variables);

    ajoute_identificateur(n->nom, C_VARIABLE_GLOBALE, T_FONCTION, 0, nb_params);

    if (trace_mips) {
        printf("%s:\n", n->nom);
        empile("$fp");
        printf("\tmove\t$fp, $sp\t\t # nouvelle valeur de $fp\n");

        empile("$ra");
    }

    entree_fonction();
    nb_argument = 0;
    nb_locale = 0;
    retour = FALSE;

    contexte_argument();
    parcour_l_dec(n->u.foncDec_.param);
    contexte_locale();

    parcour_l_dec(n->u.foncDec_.variables);

    if (trace_mips && nb_variables > 0) {
        printf("\tsubi\t$sp, $sp, %d\t # allocation variables locales\n", nb_variables * TAILLE);
    }

    parcour_instr(n->u.foncDec_.corps);

    if (trace_mips && nb_variables > 0) {
        printf("\taddi\t$sp, $sp, %d\t # allocation variables locales\n", nb_variables * TAILLE);
    }

    if (trace_dico) {
        affiche_dico();
    }

    if (trace_mips && retour == FALSE) {
        depile("$ra");
        depile("$fp");
        printf("\tjr\t$ra\n");
    }

    retour = FALSE;

    sortie_fonction();
}

void parcour_varDec(n_dec * n) {
    int index = recherche_declarative(n->nom);

    if (index != -1) {
        gen_erreur_parcour("la variable %s est déjà déclarée", n->nom);
    }

    switch (contexte) {
        case C_VARIABLE_GLOBALE:
            ajoute_identificateur(n->nom, contexte, T_ENTIER, nb_globale * TAILLE, -1);
            nb_globale++;
            if (trace_mips) {
                printf("%s:\t.space\t%d\n", n->nom, TAILLE);
            }
            break;
        case C_VARIABLE_LOCALE:
            ajoute_identificateur(n->nom, contexte, T_ENTIER, nb_locale * TAILLE, -1);
            nb_locale++;
            break;
        case C_ARGUMENT:
            ajoute_identificateur(n->nom, contexte, T_ENTIER, nb_argument * TAILLE, -1);
            nb_argument++;
            break;
    }
}

void parcour_tabDec(n_dec * n) {
    int index = recherche_declarative(n->nom);

    if (index != -1) {
        gen_erreur_parcour("le tableau %s est déjà déclarée", n->nom);
    }

    ajoute_identificateur(n->nom, C_VARIABLE_GLOBALE, T_TABLEAU_ENTIER, nb_globale, n->u.tabDec_.taille);

    if (trace_mips) {
        printf("%s:\t.space\t%d\n", n->nom, n->u.tabDec_.taille * TAILLE);
    }
}

void parcour_var(n_var * n, int active) {
    if (n->type == simple) {
        parcour_var_simple(n, active);
    } else if (n->type == indicee) {
        parcour_var_indicee(n, active);
    }
}

void parcour_var_simple(n_var *n, int active) {
    int index = recherche_executable(n->nom);

    if (index < 0) {
        gen_erreur_parcour("la variable %s n'est pas déclarée.\n", n->nom);
    }

    if (dico.tab[index].type != T_ENTIER) {
        gen_erreur_parcour("le type de la variable %s est incorrect.\n", n->nom);
    }

    if (trace_mips && active) {
        switch (dico.tab[index].classe) {
            case C_VARIABLE_GLOBALE:
                printf("\tlw\t$t1, %s\t\t # lit variable dans $t1 (globale)\n", n->nom);
                break;
            case C_VARIABLE_LOCALE:
                printf("\tlw\t$t1, %d($fp)\t\t # lit variable dans $t1 (locale)\n", -8 - dico.tab[index].adresse);
                break;
            case C_ARGUMENT:
                printf("\tlw\t$t1, %d($fp)\t\t # lit variable dans $t1 (argument)\n", 4 * nb_argument - dico.tab[index].adresse);
                break;
        }

        empile("$t1");
    }
}

void parcour_var_indicee(n_var *n, int active) {
    int index = recherche_executable(n->nom);

    if (index < 0) {
        gen_erreur_parcour("le tableau %s n'est pas déclaré.\n", n->nom);
    }

    if (dico.tab[index].type != T_TABLEAU_ENTIER) {
        gen_erreur_parcour("le type de la variable %s est incorrect.\n", n->nom);
    }

    parcour_exp(n->u.indicee_.indice);

    if(trace_mips && active) {
        depile("$t0");
        printf("\tadd\t$t0, $t0, $t0\n"
                "\tadd\t$t0, $t0, $t0\n"
                "\tlw\t$t1, %s($t0)\t\t # lit variable dans $t1 (tableau)\n", n->nom);
        empile("$t1");
    }
}

