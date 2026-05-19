#ifndef OPERATORSTATE2_H
#define OPERATORSTATE2_H

#include "State.h"

class OperatorState2 : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif