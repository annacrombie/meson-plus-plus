// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

#include <stdexcept>

#include "passes.hpp"
#include "private.hpp"

namespace MIR::Passes {

namespace {

using ToolchainMap =
    std::unordered_map<Meson::Toolchain::Language,
                       Meson::Machines::PerMachine<std::shared_ptr<Meson::Toolchain::Toolchain>>>;

std::optional<Object> replace_compiler(const Object & obj, const ToolchainMap & tc) {
    if (!std::holds_alternative<std::unique_ptr<FunctionCall>>(obj)) {
        return std::nullopt;
    }
    const auto & f = std::get<std::unique_ptr<FunctionCall>>(obj);

    if (f->holder.value_or("") != "meson" && f->name != "get_compiler") {
        return std::nullopt;
    }

    // XXX: if there is no argument here this is going to blow up spectacularly
    const auto & l = f->pos_args[0];
    // If we haven't reduced this to a string then we need to wait and try again later
    if (!std::holds_alternative<std::unique_ptr<String>>(l)) {
        return std::nullopt;
    }

    const auto lang = Meson::Toolchain::from_string(std::get<std::unique_ptr<String>>(l)->value);

    Meson::Machines::Machine m;
    try {
        const auto & n = f->kw_args.at("native");
        // If we haven't lowered this away yet, then we can't reduce this.
        if (!std::holds_alternative<std::unique_ptr<Boolean>>(n)) {
            return std::nullopt;
        }
        const auto & native = std::get<std::unique_ptr<Boolean>>(n)->value;

        m = native ? Meson::Machines::Machine::BUILD : Meson::Machines::Machine::HOST;
    } catch (std::out_of_range &) {
        m = Meson::Machines::Machine::HOST;
    }

    return std::make_unique<Compiler>(tc.at(lang).get(m));
}

} // namespace

bool insert_compilers(BasicBlock * block, const ToolchainMap & toolchains) {
    bool progress = instruction_walker(
        block, {[&](const Object & obj) { return replace_compiler(obj, toolchains); }});

    return progress;
};

} // namespace MIR::Passes
