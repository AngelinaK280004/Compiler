#ifndef OPERATORSTATE_H
#define OPERATORSTATE_H

#include "State.h"

class OperatorState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif