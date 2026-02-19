#include "missing_for_loop_initialization.h"

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

static std::string findForLoopVariableName(const FileContent* fC,
                                           NodeId forNode) {
  if (!fC || !forNode) return "<unknown>";

  NodeId condition = InvalidNodeId;
  NodeId forStep = InvalidNodeId;

  for (NodeId tmp = fC->Sibling(forNode); tmp; tmp = fC->Sibling(tmp)) {
    VObjectType t = fC->Type(tmp);
    if (t == VObjectType::paFor_initialization) {
      return "<unknown>";
    }
    if (t == VObjectType::paExpression && !condition) {
      condition = tmp;
    } else if (t == VObjectType::paFor_step && !forStep) {
      forStep = tmp;
    }
  }

  if (condition) {
    std::string name = extractName(fC, condition, "");
    if (!name.empty()) return name;
  }
  if (forStep) {
    std::string name = extractName(fC, forStep, "");
    if (!name.empty()) return name;
  }

  return "<unknown>";
}

void checkMissingForLoopInitialization(const FileContent* fC,
                                       ErrorContainer* errors,
                                       SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto forNodes = fC->sl_collect_all(root, VObjectType::paFOR);

  for (NodeId forNode : forNodes) {
    if (!forNode) continue;

    bool hasInitialization = false;
    for (NodeId tmp = fC->Sibling(forNode); tmp; tmp = fC->Sibling(tmp)) {
      VObjectType t = fC->Type(tmp);
      if (t == VObjectType::paFor_initialization) {
        hasInitialization = true;
        break;
      }
    }

    if (hasInitialization) continue;

    std::string varName = findForLoopVariableName(fC, forNode);

    reportError(fC, forNode, varName,
                ErrorDefinition::LINT_MISSING_FOR_LOOP_INITIALIZATION, errors,
                symbols);
  }
}

}  // namespace Analyzer
