#include "assignment_pattern.h"

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static std::string findLhsVariableName(const FileContent* fC,
                                       NodeId concatNode) {
  if (!fC || !concatNode) return "<unknown>";

  NodeId current = concatNode;
  while (current) {
    VObjectType type = fC->Type(current);

    if (type == VObjectType::paOperator_assignment ||
        type == VObjectType::paBlocking_assignment ||
        type == VObjectType::paNonblocking_assignment ||
        type == VObjectType::paNet_assignment) {
      for (NodeId child = fC->Child(current); child;
           child = fC->Sibling(child)) {
        VObjectType ct = fC->Type(child);
        if (ct == VObjectType::paVariable_lvalue ||
            ct == VObjectType::paNet_lvalue) {
          return extractName(fC, child);
        }
      }
      break;
    }

    if (type == VObjectType::paVariable_decl_assignment) {
      NodeId nameNode = fC->Child(current);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
      break;
    }

    current = fC->Parent(current);
  }

  return "<unknown>";
}

static bool isStructVariable(const FileContent* fC, NodeId root,
                             const std::string& varName) {
  if (varName.empty() || varName == "<unknown>") return false;

  std::set<std::string> structTypeNames;
  auto typeDecls = fC->sl_collect_all(root, VObjectType::paType_declaration);
  for (NodeId td : typeDecls) {
    if (!td) continue;
    if (fC->sl_collect_all(td, VObjectType::paStruct_union).empty()) continue;
    for (NodeId ch = fC->Child(td); ch; ch = fC->Sibling(ch)) {
      if (fC->Type(ch) == VObjectType::slStringConst)
        structTypeNames.insert(std::string(fC->SymName(ch)));
    }
  }

  auto varDecls = fC->sl_collect_all(root, VObjectType::paVariable_declaration);
  for (NodeId vd : varDecls) {
    if (!vd) continue;
    if (extractVariableName(fC, vd) != varName) continue;

    NodeId dataType = fC->Child(vd);
    if (!dataType) continue;

    if (!fC->sl_collect_all(dataType, VObjectType::paStruct_union).empty())
      return true;

    NodeId dtChild = fC->Child(dataType);
    if (dtChild && fC->Type(dtChild) == VObjectType::slStringConst &&
        structTypeNames.count(std::string(fC->SymName(dtChild))))
      return true;
  }

  auto netDecls = fC->sl_collect_all(root, VObjectType::paNet_declaration);
  for (NodeId nd : netDecls) {
    if (!nd) continue;

    auto assignNodes =
        fC->sl_collect_all(nd, VObjectType::paNet_decl_assignment);
    bool nameMatch = false;
    for (NodeId an : assignNodes) {
      NodeId nameNode = fC->Child(an);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst &&
          std::string(fC->SymName(nameNode)) == varName) {
        nameMatch = true;
        break;
      }
    }
    if (!nameMatch) continue;

    if (!fC->sl_collect_all(nd, VObjectType::paStruct_union).empty())
      return true;

    NodeId firstCh = fC->Child(nd);
    if (firstCh && fC->Type(firstCh) == VObjectType::slStringConst &&
        structTypeNames.count(std::string(fC->SymName(firstCh))))
      return true;
  }

  return false;
}

static bool isArrayVariable(const FileContent* fC, NodeId root,
                            const std::string& varName) {
  if (varName.empty() || varName == "<unknown>") return false;

  auto vdas = fC->sl_collect_all(root, VObjectType::paVariable_decl_assignment);
  for (NodeId vda : vdas) {
    if (!vda) continue;
    NodeId nameNode = fC->Child(vda);
    if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst) continue;
    if (std::string(fC->SymName(nameNode)) != varName) continue;
    if (!fC->sl_collect_all(vda, VObjectType::paUnpacked_dimension).empty())
      return true;
  }

  auto netDecls = fC->sl_collect_all(root, VObjectType::paNet_declaration);
  for (NodeId nd : netDecls) {
    if (!nd) continue;
    if (fC->sl_collect_all(nd, VObjectType::paUnpacked_dimension).empty())
      continue;
    auto assignNodes =
        fC->sl_collect_all(nd, VObjectType::paNet_decl_assignment);
    for (NodeId an : assignNodes) {
      NodeId nameNode = fC->Child(an);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst &&
          std::string(fC->SymName(nameNode)) == varName)
        return true;
    }
  }

  return false;
}

void checkAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto concats = fC->sl_collect_all(root, VObjectType::paConcatenation);

  for (NodeId concat : concats) {
    if (!concat) continue;

    bool hasLabel = false;
    for (NodeId child = fC->Child(concat); child; child = fC->Sibling(child)) {
      if (fC->Type(child) == VObjectType::paArray_member_label) {
        hasLabel = true;
        break;
      }
    }

    if (hasLabel) {
      std::string varName = findLhsVariableName(fC, concat);
      reportError(fC, concat, varName, ErrorDefinition::LINT_ASSIGNMENT_PATTERN,
                  errors, symbols);
      continue;
    }

    std::string varName = findLhsVariableName(fC, concat);
    if (isStructVariable(fC, root, varName) ||
        isArrayVariable(fC, root, varName)) {
      reportError(fC, concat, varName, ErrorDefinition::LINT_ASSIGNMENT_PATTERN,
                  errors, symbols);
    }
  }
}

}  // namespace Analyzer