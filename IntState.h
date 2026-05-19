#ifndef INTSTATE_H
#define INTSTATE_H

#include "State.h"

class IntState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif