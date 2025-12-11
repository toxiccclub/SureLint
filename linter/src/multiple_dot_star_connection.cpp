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

// Рекурсивно проверяем, содержит ли узел DOTSTAR
bool hasDotStarChild(const FileContent* fC, NodeId node) {
  if (!fC || !node) return false;

  if (fC->Type(node) == VObjectType::paDOTSTAR) {
    return true;
  }

  NodeId child = fC->Child(node);
  while (child) {
    if (hasDotStarChild(fC, child)) {
      return true;
    }
    child = fC->Sibling(child);
  }

  return false;
}

bool hasMultipleDotStarConnections(const FileContent* fC, NodeId instNode,
                                   NodeId& secondDotStarNode,
                                   NodeId& instanceNameNode) {
  if (!fC || !instNode) return false;

  NodeId hierInst =
      fC->sl_collect(instNode, VObjectType::paHierarchical_instance);
  if (!hierInst) return false;

  NodeId nameOfInst = fC->sl_collect(hierInst, VObjectType::paName_of_instance);
  if (nameOfInst) {
    instanceNameNode = fC->Child(nameOfInst);
  }

  NodeId portList =
      fC->sl_collect(hierInst, VObjectType::paList_of_port_connections);
  if (!portList) return false;

  int dotStarCount = 0;
  NodeId child = fC->Child(portList);

  while (child) {
    VObjectType type = fC->Type(child);

    // Проверяем Named_port_connection
    if (type == VObjectType::paNamed_port_connection) {
      // Проверяем, содержит ли этот узел DOTSTAR
      if (hasDotStarChild(fC, child)) {
        dotStarCount++;
        if (dotStarCount == 2) {
          // Находим сам узел DOTSTAR для точной локации ошибки
          NodeId dotStarNode = fC->sl_collect(child, VObjectType::paDOTSTAR);
          secondDotStarNode = dotStarNode ? dotStarNode : child;
          return true;
        }
      }
    }

    child = fC->Sibling(child);
  }

  return false;
}

void reportMultipleDotStarError(const FileContent* fC, NodeId badNode,
                                NodeId instanceNameNode, ErrorContainer* errors,
                                SymbolTable* symbols) {
  if (!fC || !badNode || !errors || !symbols) return;

  // Получаем имя инстанса
  std::string instanceName = "unknown";
  if (instanceNameNode) {
    try {
      instanceName = std::string(fC->SymName(instanceNameNode));
    } catch (...) {
      instanceName = "unknown";
    }
  }

  auto fileId = fC->getFileId(badNode);
  uint32_t line = fC->Line(badNode);
  uint32_t column = 0;

  try {
    column = fC->Column(badNode);
  } catch (...) {
    column = 0;
  }

  // Используем имя инстанса для сообщения об ошибке
  SymbolId obj = symbols->registerSymbol(instanceName);
  Location loc(fileId, line, column, obj);

  Error err(ErrorDefinition::LINT_MULTIPLE_DOT_STAR_CONNECTIONS, loc);
  errors->addError(err, false);
}

void checkMultipleDotStarConnections(const FileContent* fC,
                                     ErrorContainer* errors,
                                     SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  // Собираем все Module_instantiation
  auto instantiations =
      fC->sl_collect_all(root, VObjectType::paModule_instantiation);

  for (NodeId inst : instantiations) {
    if (!inst) continue;

    NodeId secondDotStarNode;
    NodeId instanceNameNode;
    if (hasMultipleDotStarConnections(fC, inst, secondDotStarNode,
                                      instanceNameNode)) {
      reportMultipleDotStarError(fC, secondDotStarNode, instanceNameNode,
                                 errors, symbols);
    }
  }
}

}  // namespace Analyzer