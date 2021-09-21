// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

#include <cstdint>
#include <unordered_map>

#include "passes.hpp"
#include "private.hpp"

namespace MIR::Passes {

namespace {

/// Simple class that counts upward
class Counter {
  public:
    Counter() : counter{0} {};

    /// Return the value, then increment
    uint64_t next() { return counter++; }

    /// Return the current value unchanged
    uint64_t get() const { return counter; }

  private:
    uint64_t counter;
};

using Counters = std::unordered_map<std::string, Counter>;

bool number(Object & obj, Counters & cts) {
    // TODO: mutative assignment?
    MIR::Variable * var = std::visit([](auto & o) { return &o->var; }, obj);
    if (!var) {
        return false;
    }

    if (cts.find(var->name) == cts.end()) {
        cts[var->name] = Counter{};
    }
    var->version = cts[var->name].next();

    return true;
}

} // namespace

bool value_numbering(BasicBlock * block, const State::Persistant & pstate) {
    Counters counters{};

    bool progress =
        instruction_walker(block, {
                                      [&](Object & obj) { return number(obj, counters); },
                                  });

    return progress;
}

} // namespace MIR::Passes
