#include "dpll.h"
#include "formula.h"

#include <stdio.h>
#include <stdlib.h>

/*
Passos para implementar o VSIDS:
1 - Fazer uma heap de máximo que guarda as maiores pontuações de cada variável - FEITO
2 - Iniciar a fila de prioridades com todas as pontuações zeradas - FEITO

Atualizar pontuação:
3 - Durante análise de conflito (resolveConflict): Quando um conflito ocorre, identifique
    todas as variáveis envolvidas no conflito.
4 - Aumente a pontuação de cada variável envolvida no conflito (normalmente +1 para
    cada ocorrência)
5 - Atualize a fila de prioridade após as mudanças nas pontuações

Decaimento:
6 - Implemente um contador de conflitos para acionar o decaimento periódico
7 - A cada N conflitos (por exemplo, a cada 255 conflitos):
        Multiplique todas as pontuações por um fator de decaimento (exemplo: 0.95)
        Alternativamente, divida todas as pontuações por um valor constante
        Atualize a fila de prioridade após o decaimento

Decisão:
8 - Quando precisar escolher uma nova variável:
        Selecione a variável não atribuída com maior pontuação da fila de prioridade
        Escolha a polaridade (verdadeiro/falso) baseada em heurística auxiliar ou histórico

Possíveis melhorias:
9 - Implemente decaimento "preguiçoso" (lazy decay) para evitar atualizar todas as variáveis
10 - Use uma heap binária ou estrutura similar para a fila de prioridade
11 - Mantenha um contador separado de "bumps" (incrementos) para cada variável
*/

typedef struct VarScore {
    double score;
    int var;
} VarScore;

typedef struct VarScoreHeap {
    VarScore **arr;
    int16_t size;
    int16_t *varHeapIdx; // Vetor que mapeia: (id da variável-1) -> index na heap. 
} VarScoreHeap;

// Heap de pontuação máxima das variáveis.
VarScoreHeap *varScoreHeap;

VarScoreHeap* createHeap(int16_t size) {
    VarScoreHeap *h = (VarScoreHeap *)malloc(sizeof(VarScoreHeap));
    h->arr = (VarScore **)malloc(size * sizeof(VarScore *));
    h->size = size;
    h->varHeapIdx = (int *)malloc(size * sizeof(int));

    for(int i=0; i<=size; i++) {
        h->arr[i] = (VarScore *)malloc(sizeof(VarScore));
        h->arr[i]->var = i+1;
        h->arr[i]->score = 0.0;
        h->varHeapIdx[i] = i;
    }

    return h;
}

void heapify(VarScoreHeap *h, int *hIdx, int index) {
    if (index <= 0) return;
    int parent = (index - 2) / 2;

    if (h->arr[index]->score > h->arr[parent]->score) {
        // atualizar posições no varHeapIdx
        hIdx[h->arr[index]->var-1] = parent;
        hIdx[h->arr[parent]->var-1] = index;

        // trocar posições na heap
        VarScore *temp = h->arr[parent];
        h->arr[parent] = h->arr[index];
        h->arr[index] = temp;
    }
}

void printHeap(VarScoreHeap *h) {
    printf("HEAP:\n");
    for(int i=0; i<h->size; i++) {
        printf("%d -> [lit: %d | score: %f]\n", i, h->arr[i]->var, h->arr[i]->score);
    }
    printf("heap size: %d\n", h->size);
}

void PreProcessing(Form* form){
    varScoreHeap = createHeap(form->numVars);
    printHeap(varScoreHeap);
}

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

