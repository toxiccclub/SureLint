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

void checkClassVariableLifetime(const FileContent* fC) {
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

        auto fileId = fC->getFileId(autoId);
        std::string fileName =
            std::string(FileSystem::getInstance()->toPath(fileId));
        uint32_t line = fC->Line(autoId);

        std::cerr << "Error CLASS_VARIABLE_LIFETIME: variable '" << varName
                  << "' uses automatic lifetime at " << fileName << ":" << line
                  << std::endl;
      }
    }
  }
}
}  // namespace Analyzer