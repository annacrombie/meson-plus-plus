// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

#include "exceptions.hpp"
#include "passes.hpp"
#include "private.hpp"

namespace MIR::Passes {

namespace {

using namespace Meson::Machines;

std::optional<Machine> machine_map(const std::string & func_name) {
    if (func_name == "build_machine") {
        return Meson::Machines::Machine::BUILD;
    } else if (func_name == "host_machine") {
        return Meson::Machines::Machine::HOST;
    } else if (func_name == "target_machine") {
        return Meson::Machines::Machine::TARGET;
    } else {
        return std::nullopt;
    }
}

MIR::Object lower_function(const std::string & holder, const std::string & name,
                           const Info & info) {
    if (name == "cpu_family") {
        // TODO: it's probably going to be useful to have a helper for this...
        return MIR::Object{std::make_unique<MIR::String>(info.cpu_family)};
    } else if (name == "cpu") {
        // TODO: it's probably going to be useful to have a helper for this...
        return MIR::Object{std::make_unique<MIR::String>(info.cpu)};
    } else if (name == "system") {
        // TODO: it's probably going to be useful to have a helper for this...
        return MIR::Object{std::make_unique<MIR::String>(info.system())};
    } else if (name == "endian") {
        return MIR::Object{std::make_unique<MIR::String>(
            info.endian == Meson::Machines::Endian::LITTLE ? "little" : "big")};
    } else {
        throw Util::Exceptions::MesonException{holder + " has no method " + name};
    }
}

using MachineInfo = Meson::Machines::PerMachine<Meson::Machines::Info>;

std::optional<Object> lower_functions(const MachineInfo & machines, const Object & obj) {
    if (std::holds_alternative<std::unique_ptr<MIR::FunctionCall>>(obj)) {
        const auto & f = std::get<std::unique_ptr<MIR::FunctionCall>>(obj);
        const auto & holder = f->holder.value_or("");

        auto maybe_m = machine_map(holder);
        if (maybe_m.has_value()) {
            const auto & info = machines.get(maybe_m.value());

            return lower_function(holder, f->name, info);
        }
    }
    return std::nullopt;
}

} // namespace

bool machine_lower(BasicBlock * block, const MachineInfo & machines) {
    bool progress =
        instruction_walker(block, [&](const Object & o) { return lower_functions(machines, o); });

    // Check if we have a condition, and try to lower that as well.
    // XXX: need a test for this
    if (block->condition.has_value()) {
        auto & con = block->condition.value();
        if (std::holds_alternative<std::unique_ptr<MIR::FunctionCall>>(con.condition)) {
            const auto & f = std::get<std::unique_ptr<MIR::FunctionCall>>(con.condition);
            const auto & holder = f->holder.value_or("");

            auto maybe_m = machine_map(holder);
            if (maybe_m.has_value()) {
                const auto & info = machines.get(maybe_m.value());
                MIR::Object new_value = lower_function(holder, f->name, info);
                con.condition = std::move(new_value);
                progress = true;
            }
        }
    }

    return progress;
};

} // namespace MIR::Passes
