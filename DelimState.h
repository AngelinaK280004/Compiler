#ifndef DELIMSTATE_H
#define DELIMSTATE_H

#include "State.h"

class DelimState : public State {
public:
    Token process(Tokenizer& tokenizer) override;
};

#endif