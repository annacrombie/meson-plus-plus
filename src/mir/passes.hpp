// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Dylan Baker

/**
 * Lowering passes for MIR
 */

#pragma once

#include "machines.hpp"
#include "mir.hpp"

namespace MIR::Passes {

/**
 * Prune dead condition branches
 *
 * Once we've been able to lower away conditions in the condition ndoes we want
 * to trim away dead branches and join the ir lists together so we end up with a
 * single flat list of Objects.
 */
bool branch_pruning(BasicBlock *);

/**
 * Join basic blocks together
 *
 * Specifically for use after branch_pruning, when we have two continguous
 * blocks with no condition to move between thme
 */
bool join_blocks(BasicBlock *);

/**
 * Lower away machine related information.
 *
 * This replaces function calls to `host_machine`, `build_machine`, and
 * `target_machine` methods with their values.
 */
bool machine_lower(BasicBlock *, const Meson::Machines::PerMachine<Meson::Machines::Info> &);

} // namespace MIR::Passes
