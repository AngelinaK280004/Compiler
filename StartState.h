#ifndef STARTSTATE_H
#define STARTSTATE_H

#include "State.h"

class StartState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif