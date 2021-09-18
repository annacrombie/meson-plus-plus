// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Intel Corporation

/**
 * Main ninja backend entry point.
 */

#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <variant>
#include <vector>

#include "entry.hpp"
#include "exceptions.hpp"
#include "toolchains/compiler.hpp"

namespace fs = std::filesystem;

namespace Backends::Ninja {

namespace {

void write_compiler_rule(const std::string & lang,
                         const std::unique_ptr<MIR::Toolchain::Compiler::Compiler> & c,
                         std::ofstream & out) {

    // TODO: build or host correctly
    out << "rule " << lang << "_compiler_for_"
        << "build" << std::endl;

    // Write the command
    // TODO: write the depfile stuff
    out << "  command =";
    for (const auto & c : c->command) {
        out << " " << c;
    }
    out << " ${ARGS}";
    for (const auto & c : c->output_command("${out}")) {
        out << " " << c;
    }
    for (const auto & c : c->compile_only_command()) {
        out << " " << c;
    }
    out << " ${in}" << std::endl;

    // Write the description
    out << "  description = Compiling " << c->language() << " object ${out}" << std::endl
        << std::endl;
}

void write_linker_rule(const std::string & lang,
                       const std::unique_ptr<MIR::Toolchain::Linker::Linker> & c,
                       std::ofstream & out) {

    // TODO: build or host correctly
    out << "rule " << lang << "_linker_for_"
        << "build" << std::endl;

    // Write the command
    // TODO: write the depfile stuff
    out << "  command =";
    for (const auto & c : c->command()) {
        out << " " << c;
    }
    out << " ${ARGS}";
    for (const auto & c : c->output_command("${out}")) {
        out << " " << c;
    }
    out << " ${in} ${LINK_ARGS}" << std::endl;

    // Write the description
    out << "  description = Linking target ${out}" << std::endl << std::endl;
}

std::string escape(const std::string & str) {
    std::string new_s{};
    for (const auto & s : str) {
        if (s == ' ') {
            new_s.push_back('$');
            new_s.push_back(s);
        } else {
            new_s.push_back(s);
        }
    }
    return new_s;
}

} // namespace

void generate(const MIR::BasicBlock * const block, const MIR::State::Persistant & pstate) {
    if (!fs::exists(pstate.build_root)) {
        int ret = mkdir(pstate.build_root.c_str(), 0777);
        if (ret != 0) {
            int err = errno;
            if (err != EEXIST) {
                throw Util::Exceptions::MesonException{"Could not create build directory"};
            }
        }
    }

    std::ofstream out{};
    out.open(pstate.build_root / "build.ninja", std::ios::out | std::ios::trunc);
    out << "# This is a build file for the project \"" << pstate.name << "\"." << std::endl
        << "# It is autogenerated by the Meson++ build system." << std::endl
        << "# Do not edit by hand." << std::endl
        << std::endl
        << "ninja_required_version = 1.8.2" << std::endl
        << std::endl;

    out << "# Compilation rules" << std::endl << std::endl;

    for (const auto & [l, tc] : pstate.toolchains) {
        const auto & lstr = MIR::Toolchain::to_string(l);
        // TODO: should also have a _for_host
        write_compiler_rule(lstr, tc.build()->compiler, out);
    }

    out << "# Static Linking rules" << std::endl << std::endl;
    // TODO:

    out << "# Dynamic Linking rules" << std::endl << std::endl;

    for (const auto & [l, tc] : pstate.toolchains) {
        const auto & lstr = MIR::Toolchain::to_string(l);
        // TODO: should also have a _for_host
        write_linker_rule(lstr, tc.build()->linker, out);
    }

    out << "# Phony build target, always out of date\n\n"
        << "build PHONY: phony\n\n";
    out << "# Build rules for targets\n\n";

    // This is a completely bullshit approach, as it breaks as soon as you have
    // a relationship between any targets
    for (const auto & i : block->instructions) {
        if (std::holds_alternative<std::unique_ptr<MIR::Executable>>(i)) {
            // TODO: handle the correct machine
            const auto & e = std::get<std::unique_ptr<MIR::Executable>>(i)->value;
            std::vector<std::string> srcs{};
            for (const auto & f : e.sources) {
                // TODO: obj files are a per compiler thing, I think
                // TODO: get the proper language
                // TODO: actually set args to something
                auto built = escape(f.get_name()) + ".o";
                srcs.emplace_back(built);
                out << "build " << built << ": cpp_compiler_for_build "
                    << escape(f.relative_to_build_dir()) << std::endl;
                out << "  ARGS = " << std::endl << std::endl;
            }

            // TODO: detect the actual linker to use
            out << "build " << e.name << ": cpp_linker_for_build";
            for (const auto & s : srcs) {
                out << " " << s;
            }
            out << "\n";
            out << "  LINK_ARGS = " << std::endl;
        }
    }

    out.flush();
    out.close();
}

} // namespace Backends::Ninja
