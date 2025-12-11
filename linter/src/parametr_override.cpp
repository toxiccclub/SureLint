#include <cstdint>
#include <string>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

namespace Analyzer {

bool hasInvalidParameterOverride(const FileContent* fC, NodeId instNode) {
  if (!fC || !instNode) return false;

  NodeId child = fC->Child(instNode);
  if (!child) return false;

  NodeId secondChild = fC->Sibling(child);
  if (!secondChild) return false;

  VObjectType secondType = fC->Type(secondChild);

  if (secondType == VObjectType::slIntConst ||
      secondType == VObjectType::slRealConst ||
      secondType == VObjectType::slStringConst ||
      secondType == VObjectType::ppNumber) {
    return true;
  }

  if (secondType == VObjectType::paConstant_expression ||
      secondType == VObjectType::paPrimary_literal ||
      secondType == VObjectType::paConstant_primary) {
    NodeId thirdChild = fC->Sibling(secondChild);
    if (thirdChild) {
      VObjectType thirdType = fC->Type(thirdChild);
      if (thirdType == VObjectType::paHierarchical_instance ||
          thirdType == VObjectType::paName_of_instance) {
        return true;
      }
    }
  }

  return false;
}

void reportError(const FileContent* fC, NodeId badNode, ErrorContainer* errors,
                 SymbolTable* symbols) {
  if (!fC || !badNode || !errors || !symbols) return;

  std::string tokenName = "<unknown>";
  try {
    tokenName = std::string(fC->SymName(badNode));
  } catch (...) {
  }

  auto fileId = fC->getFileId(badNode);
  uint32_t line = fC->Line(badNode);
  uint32_t column = 0;
  try {
    column = fC->Column(badNode);
  } catch (...) {
    column = 0;
  }

  SymbolId obj = symbols->registerSymbol(tokenName);
  Location loc(fileId, line, column, obj);
  Error err(ErrorDefinition::LINT_PARAMETR_OVERRIDE, loc);

  errors->addError(err, false);
}

void checkParameterOverride(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  // Собираем все Module_instantiation
  auto instantiations =
      fC->sl_collect_all(root, VObjectType::paModule_instantiation);

  for (NodeId inst : instantiations) {
    if (!inst) continue;

    if (hasInvalidParameterOverride(fC, inst)) {
      // Находим узел с некорректным значением
      NodeId moduleName = fC->Child(inst);
      NodeId badNode = fC->Sibling(moduleName);

      if (!badNode) badNode = inst;

      reportError(fC, badNode, errors, symbols);
    }
  }
}

}  // namespace Analyzer