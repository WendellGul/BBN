//
// Created by Great on 2017/12/9.
//

#ifndef BBN_FULLOBBBN_H
#define BBN_FULLOBBBN_H

#include "BBN.h"

BBN fCancer();
BBN fCancerNoCPT();
void fLearn(string trainFile, string cptFile);
void fPredict(string testFile, string cptFile);
void fGenerateData(int k, string type);

#endif //BBN_FULLOBBBN_H
