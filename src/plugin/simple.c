#include "dpll.h"
#include "formula.h"

#include <stdio.h>
#include <stdlib.h>

void PreProcessing(Form* form){
}

// decide padrão
// enum DecideState Decide(const Form* form)
// {

//     ClauseNode* list = form->clauses;
//     while(list != NULL)
//     {

//         Clause *clause = list->clause;

//         for(int i = 0; i<clause->size; ++i)
//         {
//             LiteralId lit = clause->literals[i];
//             lit = ((lit > 0 )? lit : -lit);
//             if(getVarState(lit-1) == UNK)
//             {
//                 insertDecisionLevel(lit-1, FALSE);

//                 return FOUND_VAR;
//             }
//         }

//         list = list->next;
//     }

//     return ALL_ASSIGNED;
// }

// jeroslaw-wang
enum DecideState Decide(const Form* form) {
    int bestVar = -1;
    double bestScore = -1.0;
    double bestVarNegScore = -1.0;
    double bestVarPosScore = -1.0;

    for (int v=0; v < form->numClauses; v++) {
        if (getVarState(v) != UNK) continue;
        double score = 0.0;
        double posScore = 0.0;
        double negScore = 0.0;

        ClauseNode* node = form->literals[2*v];
        while (node) {
            posScore += 1.0 / (1 << node->clause->size);
            node = node->next;
        }

        node = form->literals[2*v+1];
        while (node) {
            negScore += 1.0 / (1 << node->clause->size);
            node = node->next;
        }

        score = posScore + negScore;

        if (score > bestScore) {
            bestScore = score;
            bestVar = v;
            bestVarNegScore = negScore;
            bestVarPosScore = posScore;
        }
    }

    if (bestVar != -1) {
        if (bestVarNegScore > bestVarPosScore) {
            insertDecisionLevel(bestVar, FALSE);
        } else {
            insertDecisionLevel(bestVar, TRUE);
        }
        return FOUND_VAR;
    }

    return ALL_ASSIGNED;
}

bool BCP(Form *formula, const Decision decision)
{
    bool flag;
    ClauseNode *head;
    Clause *clause;

    LiteralId falseValuedLiteral = ((decision.value == FALSE) ? decision.id+1 : -decision.id -1);

    head = formula->literals[getPos(falseValuedLiteral)];



    // now if some clause have only negative
    // values than this is a conflict
    while(head!=NULL)
    {
        flag = false;
        clause = head->clause;

        for(int i = 0; i<clause->size; ++i)
        {
            LiteralId lit = clause->literals[i];

            if(getLitState(lit) != FALSE)
                flag=true;
        }

        if(!flag){
            return false;
        }

        head = head->next;
   }

   return true;
}

int resolveConflict()
{
    Decision* decisions =  getDecisions();

    int i = getLevel()-1;

    for(; i>=0; --i)
        if(decisions[i].flipped == false)
            break;

    return i+1;
}

