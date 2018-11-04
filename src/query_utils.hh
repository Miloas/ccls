// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "query.hh"
#include "working_files.hh"

#include <optional>

namespace ccls {
Maybe<DeclRef> GetDefinitionSpell(DB *db, SymbolIdx sym);

// Get defining declaration (if exists) or an arbitrary declaration (otherwise)
// for each id.
std::vector<Use> GetFuncDeclarations(DB *, const std::vector<Usr> &);
std::vector<Use> GetTypeDeclarations(DB *, const std::vector<Usr> &);
std::vector<Use> GetVarDeclarations(DB *, const std::vector<Usr> &, unsigned);

// Get non-defining declarations.
std::vector<DeclRef> &GetNonDefDeclarations(DB *db, SymbolIdx sym);

std::vector<Use> GetUsesForAllBases(DB *db, QueryFunc &root);
std::vector<Use> GetUsesForAllDerived(DB *db, QueryFunc &root);
std::optional<lsRange> GetLsRange(WorkingFile *working_file,
                                  const Range &location);
DocumentUri GetLsDocumentUri(DB *db, int file_id, std::string *path);
DocumentUri GetLsDocumentUri(DB *db, int file_id);

std::optional<Location> GetLsLocation(DB *db, WorkingFiles *wfiles, Use use);
std::optional<Location> GetLsLocation(DB *db, WorkingFiles *wfiles,
                                        SymbolRef sym, int file_id);
std::vector<Location> GetLsLocations(DB *db, WorkingFiles *wfiles,
                                           const std::vector<Use> &uses);
// Returns a symbol. The symbol will *NOT* have a location assigned.
std::optional<SymbolInformation> GetSymbolInfo(DB *db, SymbolIdx sym,
                                                 bool detailed);

std::vector<SymbolRef> FindSymbolsAtLocation(WorkingFile *working_file,
                                             QueryFile *file,
                                             Position &ls_pos,
                                             bool smallest = false);

template <typename Fn> void WithEntity(DB *db, SymbolIdx sym, Fn &&fn) {
  switch (sym.kind) {
  case Kind::Invalid:
  case Kind::File:
    break;
  case Kind::Func:
    fn(db->GetFunc(sym));
    break;
  case Kind::Type:
    fn(db->GetType(sym));
    break;
  case Kind::Var:
    fn(db->GetVar(sym));
    break;
  }
}

template <typename Fn> void EachEntityDef(DB *db, SymbolIdx sym, Fn &&fn) {
  WithEntity(db, sym, [&](const auto &entity) {
    for (auto &def : entity.def)
      if (!fn(def))
        break;
  });
}

template <typename Fn>
void EachOccurrence(DB *db, SymbolIdx sym, bool include_decl, Fn &&fn) {
  WithEntity(db, sym, [&](const auto &entity) {
    for (Use use : entity.uses)
      fn(use);
    if (include_decl) {
      for (auto &def : entity.def)
        if (def.spell)
          fn(*def.spell);
      for (Use use : entity.declarations)
        fn(use);
    }
  });
}

SymbolKind GetSymbolKind(DB *db, SymbolIdx sym);

template <typename Fn>
void EachDefinedFunc(DB *db, const std::vector<Usr> &usrs, Fn &&fn) {
  for (Usr usr : usrs) {
    auto &obj = db->Func(usr);
    if (!obj.def.empty())
      fn(obj);
  }
}
} // namespace ccls
