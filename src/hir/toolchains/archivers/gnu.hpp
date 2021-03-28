// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Intel Corporation

#pragma once

#include "toolchains/archiver.hpp"

namespace HIR::Toolchain::Archiver {

/**
 * The GNU ar archiver.
 */
class Gnu : public Archiver {
  public:
    Gnu(const std::vector<std::string> & c) : Archiver{c} {};
    ~Gnu(){};

    RSPFileSupport rsp_support() const override final;
};

} // namespace HIR::Toolchain::Archiver
