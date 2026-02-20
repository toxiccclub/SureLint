#include "foreach_loop_condition.h"

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static int countForeachDimensionGroups(const FileContent* fC,
                                       NodeId foreachKeyword) {
  NodeId arrayIdNode = InvalidNodeId;
  for (NodeId sib = fC->Sibling(foreachKeyword); sib; sib = fC->Sibling(sib)) {
    if (fC->Type(sib) == VObjectType::paPs_or_hierarchical_array_identifier) {
      arrayIdNode = sib;
      break;
    }
  }

  if (!arrayIdNode) return 0;

  int groups = 0;
  for (NodeId sib = fC->Sibling(arrayIdNode); sib; sib = fC->Sibling(sib)) {
    VObjectType t = fC->Type(sib);
    if (t == VObjectType::paLoop_variables || t == VObjectType::slStringConst) {
      ++groups;
    }
  }

  return groups;
}

static std::string getForeachArrayName(const FileContent* fC,
                                       NodeId foreachKeyword) {
  for (NodeId sib = fC->Sibling(foreachKeyword); sib; sib = fC->Sibling(sib)) {
    if (fC->Type(sib) == VObjectType::paPs_or_hierarchical_array_identifier) {
      return extractName(fC, sib, "<unknown>");
    }
  }
  return "<unknown>";
}

void checkForeachLoopCondition(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  // Собираем все FOREACH-ноды в файле
  auto foreachNodes = fC->sl_collect_all(root, VObjectType::paFOREACH);

  for (NodeId foreachNode : foreachNodes) {
    if (!foreachNode) continue;

    // Многомерный выбор = больше одной группы скобок
    if (countForeachDimensionGroups(fC, foreachNode) <= 1) continue;

    // Имя массива для сообщения об ошибке
    std::string arrayName = getForeachArrayName(fC, foreachNode);

    // reportError() из linter_utils
    reportError(fC, foreachNode, arrayName,
                ErrorDefinition::LINT_FOREACH_LOOP_CONDITION, errors, symbols);
  }
}

}  // namespace Analyzer