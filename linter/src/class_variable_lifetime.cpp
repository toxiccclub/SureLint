#include <cstdint>
#include <iostream>
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

std::string findVariableName(const FileContent* fC, NodeId propId) {
  // Ищем все Variable_decl_assignment внутри List_of_variable_decl_assignments
  auto listNodes = fC->sl_collect_all(
      propId, VObjectType::paList_of_variable_decl_assignments);
  for (NodeId listNode : listNodes) {
    auto assignNodes =
        fC->sl_collect_all(listNode, VObjectType::paVariable_decl_assignment);
    for (NodeId assignNode : assignNodes) {
      // Находим StringConst внутри Variable_decl_assignment
      NodeId nameNode = fC->Child(assignNode);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
    }
  }
  return "<unknown>";
}

void checkClassVariableLifetime(const FileContent* fC, ErrorContainer* errors, SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  // Ищем все Class_declaration
  auto classNodes = fC->sl_collect_all(root, VObjectType::paClass_declaration);

  for (NodeId classId : classNodes) {
    // Ищем Class_property
    auto classProps =
        fC->sl_collect_all(classId, VObjectType::paClass_property);

    for (NodeId propId : classProps) {
      // Ищем Lifetime_Automatic
      auto autoNodes =
          fC->sl_collect_all(propId, VObjectType::paLifetime_Automatic);

      for (NodeId autoId : autoNodes) {
        std::string varName = findVariableName(fC, propId);

        // Получаем позицию в файле
        auto fileId = fC->getFileId(autoId);
        uint32_t line = fC->Line(autoId);
        uint32_t column = 0;
        
        try {
          column = fC->Column(autoId);
        } catch (...) {
          column = 0;
        }

        SymbolId obj = symbols->registerSymbol(varName);

        Location loc(fileId, line, column, obj);
        Error err(ErrorDefinition::LINT_CLASS_VARIABLE_LIFETIME, loc);

        // Добавляем ошибку; второй аргумент = showDuplicates
        errors->addError(err, false);
      }
    }
  }
}
}  // namespace Analyzer