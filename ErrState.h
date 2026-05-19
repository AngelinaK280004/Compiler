#ifndef ERRSTATE_H
#define ERRSTATE_H

#include "State.h"

class ErrState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif