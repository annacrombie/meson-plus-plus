// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

#include "passes.hpp"
#include "private.hpp"

namespace MIR::Passes {

namespace {}

bool constant_propogation(BasicBlock * block, State::Persistant & pstate) {
    // TODO: this will have some concern about premature optimization across
    // blocks. We want to eliminate the dead variables at some point, but we
    // can't remove them entirely until we're sure there are no more uses.
    // That might mean just looking forward across all blocks to find additional
    // uses of the constant

    return false;
}

} // namespace MIR::Passes
