#include "hierarchical_interface_identifier.h"

#include <cstdint>
#include <string>
#include <vector>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

// получить все StringConst внутри заданного узла
static std::vector<NodeId> collectStringConsts(const FileContent* fC,
                                               NodeId node) {
  std::vector<NodeId> out;
  auto stringNodes = fC->sl_collect_all(node, VObjectType::slStringConst);
  for (NodeId s : stringNodes) out.push_back(s);
  return out;
}

// получить «объединенное» имя из списка StringConst (например, «top.id2.id3»)
static std::string joinNames(const FileContent* fC,
                             const std::vector<NodeId>& parts) {
  if (parts.empty()) return "<unknown>";
  std::string res;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i) res += ".";
    res += std::string(fC->SymName(parts[i]));
  }
  return res;
}

void checkHierarchicalInterfaceIdentifier(const FileContent* fC,
                                          ErrorContainer* errors,
                                          SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  // Ищем interface_identifier
  auto iidNodes = fC->sl_collect_all(root, VObjectType::paInterface_identifier);

  for (NodeId iid : iidNodes) {
    auto parts = collectStringConsts(fC, iid);

    // Если частей > 1 — это иерархическое имя → нарушение
    if (parts.size() > 1) {
      std::string fullName = joinNames(fC, parts);
      reportError(fC, iid, fullName,
                  ErrorDefinition::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER,
                  errors, symbols);
    }
  }
}
}  // namespace Analyzer