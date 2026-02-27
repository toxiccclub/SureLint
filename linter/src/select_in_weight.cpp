#include "select_in_weight.h"

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

void checkSelectInWeight(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;
  auto rsRules = fC->sl_collect_all(root, VObjectType::paRs_rule);

  for (NodeId rsRuleId : rsRules) {
    auto selects = fC->sl_collect_all(rsRuleId, VObjectType::paSelect);
    auto constantSelects =
        fC->sl_collect_all(rsRuleId, VObjectType::paConstant_select);

    if (!selects.empty() || !constantSelects.empty()) {
      std::string name = extractName(fC, rsRuleId);
      reportError(fC, rsRuleId, name, ErrorDefinition::LINT_SELECT_IN_WEIGHT,
                  errors, symbols);
    }
  }
}

}  // namespace Analyzer