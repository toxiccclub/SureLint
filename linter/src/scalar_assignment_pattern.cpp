#include "scalar_assignment_pattern.h"

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static bool is1BitScalarType(VObjectType type) {
  return type == VObjectType::paIntVec_TypeBit ||
         type == VObjectType::paIntVec_TypeLogic ||
         type == VObjectType::paIntVec_TypeReg;
}

static bool isScalarVariable(const FileContent* fC, NodeId root,
                             const std::string& varName) {
  if (varName.empty() || varName == "<unknown>") return false;

  auto varDecls = fC->sl_collect_all(root, VObjectType::paVariable_declaration);

  for (NodeId vd : varDecls) {
    if (!vd) continue;

    if (extractVariableName(fC, vd) != varName) continue;

    NodeId dataType = fC->Child(vd);
    if (!dataType) continue;

    NodeId typeKeyword = fC->Child(dataType);
    if (!typeKeyword) continue;

    if (!is1BitScalarType(fC->Type(typeKeyword))) continue;

    if (!fC->sl_collect_all(dataType, VObjectType::paPacked_dimension).empty())
      continue;

    auto vdas = fC->sl_collect_all(vd, VObjectType::paVariable_decl_assignment);
    bool hasUnpacked = false;
    for (NodeId vda : vdas) {
      if (!fC->sl_collect_all(vda, VObjectType::paUnpacked_dimension).empty()) {
        hasUnpacked = true;
        break;
      }
    }
    if (hasUnpacked) continue;

    return true;
  }

  return false;
}

void checkScalarAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto patterns = fC->sl_collect_all(root, VObjectType::paAssignment_pattern);

  for (NodeId pat : patterns) {
    if (!pat) continue;

    std::string varName = findLhsVariableName(fC, pat);

    if (isScalarVariable(fC, root, varName)) {
      reportError(fC, pat, varName,
                  ErrorDefinition::LINT_SCALAR_ASSIGNMENT_PATTERN, errors,
                  symbols);
    }
  }
}

}  // namespace Analyzer