#ifndef IDSTATE_H
#define IDSTATE_H

#include "State.h"

class IdState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif