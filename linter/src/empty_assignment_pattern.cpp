#include "empty_assignment_pattern.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static std::string findLhsVariableName(const FileContent* fC,
                                       NodeId patternNode) {
  if (!fC || !patternNode) return "<unknown>";

  NodeId current = patternNode;
  while (current) {
    VObjectType type = fC->Type(current);
    if (type == VObjectType::paOperator_assignment ||
        type == VObjectType::paBlocking_assignment ||
        type == VObjectType::paNonblocking_assignment) {
      for (NodeId child = fC->Child(current); child;
           child = fC->Sibling(child)) {
        if (fC->Type(child) == VObjectType::paVariable_lvalue) {
          return extractName(fC, child);
        }
      }
      break;
    }
    current = fC->Parent(current);
  }

  return "<unknown>";
}

void checkEmptyAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                                 SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto patterns = fC->sl_collect_all(root, VObjectType::paAssignment_pattern);

  for (NodeId pat : patterns) {
    if (!pat) continue;

    if (fC->Child(pat)) continue;

    std::string varName = findLhsVariableName(fC, pat);

    reportError(fC, pat, varName,
                ErrorDefinition::LINT_EMPTY_ASSIGNMENT_PATTERN, errors,
                symbols);
  }
}

}  // namespace Analyzer
