/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "SingleImpl.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "Debug.h"
#include "DexLoader.h"
#include "DexOutput.h"
#include "DexUtil.h"
#include "SingleImplDefs.h"
#include "SingleImplUtil.h"
#include "Trace.h"
#include "walkers.h"

size_t SingleImplPass::s_invoke_intf_count = 0;

namespace {

/**
 * Build a map from interface to the type implementing that
 * interface. We also walk up the interface chain and for every interface
 * in scope (defined in the DEXes) we add an entry as well. So
 * interface B {}
 * interface A extends B {}
 * class C implements A {}
 * generates 2 entries in the map (assuming A, B and C are in the DEXes)
 * { A => C, B => C }
 * whereas if B was outside the DEXes (i.e. java or android interface)
 * we will only have one entry { A => C }
 * keep that in mind when using this map
 */
void map_interfaces(const std::list<DexType*>& intf_list,
                    DexClass* cls,
                    TypeToTypes& intfs_to_classes) {
  for (auto& intf : intf_list) {
    const auto intf_cls = type_class(intf);
    if (intf_cls == nullptr || intf_cls->is_external()) continue;
    if (std::find(intfs_to_classes[intf].begin(),
                  intfs_to_classes[intf].end(), cls->get_type()) ==
        intfs_to_classes[intf].end()) {
      intfs_to_classes[intf].push_back(cls->get_type());
      auto intfs = intf_cls->get_interfaces();
      map_interfaces(intfs->get_type_list(), cls, intfs_to_classes);
    }
  }
};

/**
 * Collect all interfaces.
 */
void build_type_maps(const Scope& scope,
                     TypeToTypes& intfs_to_classes,
                     TypeSet& interfs) {
  for (const auto& cls : scope) {
    if (is_interface(cls)) {
      interfs.insert(cls->get_type());
      continue;
    }
    auto intfs = cls->get_interfaces();
    map_interfaces(intfs->get_type_list(), cls, intfs_to_classes);
  }
}

void collect_single_impl(const TypeToTypes& intfs_to_classes,
                         TypeMap& single_impl) {
  for (const auto intf_it : intfs_to_classes) {
    if (intf_it.second.size() != 1) continue;
    auto intf = intf_it.first;
    auto intf_cls = type_class(intf);
    always_assert(intf_cls && !intf_cls->is_external());
    if (intf_cls->get_access() & DexAccessFlags::ACC_ANNOTATION) continue;
    auto impl = intf_it.second[0];
    auto impl_cls = type_class(impl);
    always_assert(impl_cls && !impl_cls->is_external());
    // I don't know if it's possible but it's cheap enough to check
    if (impl_cls->get_access() & DexAccessFlags::ACC_ANNOTATION) continue;
    single_impl[intf] = impl;
  }
}
}

const int MAX_PASSES = 8;

void SingleImplPass::run_pass(DexClassesVector& dexen, PgoFiles& pgo) {
  auto scope = build_class_scope(dexen);
  int max_steps = 0;
  while (true) {
    DEBUG_ONLY size_t scope_size = scope.size();
    TypeToTypes intfs_to_classes;
    TypeSet intfs;
    build_type_maps(scope, intfs_to_classes, intfs);
    TypeMap single_impl;
    collect_single_impl(intfs_to_classes, single_impl);

    std::unique_ptr<SingleImplAnalysis> single_impls =
        SingleImplAnalysis::analyze(scope, single_impl, intfs, m_config);
    auto optimized = optimize(std::move(single_impls), scope);
    if (optimized == 0 || ++max_steps >= MAX_PASSES) break;
    removed_count += optimized;
    assert(scope_size > scope.size());
  }

  TRACE(INTF, 1, "Removed interfaces %ld\n", removed_count);
  TRACE(INTF, 1,
          "Updated invoke-interface to invoke-virtual %ld\n",
          s_invoke_intf_count);
  post_dexen_changes(scope, dexen);
}
