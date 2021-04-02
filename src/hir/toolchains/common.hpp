// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Intel Corporation

/* Common helpers for toolchains
 */

#pragma once

#include <string>

namespace HIR::Toolchain {

/**
 * If the tool (compiler, linker, archiver) support response files, and which
 * dialect they use.
 */
enum class RSPFileSupport {
    NONE,
    MSVC,
    GCC,
};

/**
 * The toolchain language
 */
enum class Language {
    CPP,
};

Language from_string(const std::string &);

} // namespace HIR::Toolchain
