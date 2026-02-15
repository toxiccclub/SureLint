#include "select_in_event_control.h"

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

void checkSelectInEventControl(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto eventControls = fC->sl_collect_all(root, VObjectType::paEvent_control);

  for (NodeId eventControlId : eventControls) {
    auto selects = fC->sl_collect_all(eventControlId, VObjectType::paSelect);
    auto constantSelects =
        fC->sl_collect_all(eventControlId, VObjectType::paConstant_select);

    if (!selects.empty() || !constantSelects.empty()) {
      std::string eventName = extractName(fC, eventControlId);
      reportError(fC, eventControlId, eventName,
                  ErrorDefinition::LINT_SELECT_IN_EVENT_CONTROL, errors,
                  symbols);
    }
  }
}

}  // namespace Analyzer
