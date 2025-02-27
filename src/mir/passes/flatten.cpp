
// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

#include "passes.hpp"
#include "private.hpp"

namespace MIR::Passes {

namespace {

/// Recursively call this to flatten nested arrays to function calls
void do_flatten(const std::unique_ptr<Array> & arr, std::vector<Object> & newarr) {
    for (auto & e : arr->value) {
        if (std::holds_alternative<std::unique_ptr<Array>>(e)) {
            do_flatten(std::get<std::unique_ptr<Array>>(e), newarr);
        } else {
            newarr.emplace_back(std::move(e));
        }
    }
}

/**
 * Flatten arrays when passed as arguments to functions.
 */
std::optional<Object> flatten_cb(const Object & obj) {
    if (!std::holds_alternative<std::unique_ptr<Array>>(obj)) {
        return std::nullopt;
    }

    const auto & arr = std::get<std::unique_ptr<Array>>(obj);
    std::vector<Object> newarr{};
    do_flatten(arr, newarr);

    return std::make_unique<Array>(std::move(newarr));
}

} // namespace

bool flatten(BasicBlock * block, const State::Persistant & pstate) {
    // TODO: we need to skip this for message, error, and warning
    bool progress = instruction_walker(
        block, {[&](Object & obj) { return function_argument_walker(obj, flatten_cb); }});

    return progress;
}

} // namespace MIR::Passes
