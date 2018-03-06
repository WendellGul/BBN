//
// Created by Great on 2017/12/9.
//

#ifndef BBN_PARTOBBBN_H
#define BBN_PARTOBBBN_H

#include "BBN.h"

BBN cancer();
BBN cancerNoCPT();
void learn(string trainFile, string cptFile);
void predict(string testFile, string cptFile);
void generateData(int k, string type);

#endif //BBN_PARTOBBBN_H
