#ifndef STATE_H
#define STATE_H

#include "Tokenizer.h"
using namespace std;

class State {
public:
    virtual ~State() {}
    virtual Token process(Tokenizer& tokenizer) = 0;
};

#endif