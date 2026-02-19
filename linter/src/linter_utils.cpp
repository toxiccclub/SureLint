#include "linter_utils.h"

#include "Surelog/Common/FileSystem.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"

using namespace SURELOG;

namespace Analyzer {

uint32_t getColumnSafe(const FileContent* fC, NodeId node) {
  if (!fC || !node) return 0;
  try {
    return fC->Column(node);
  } catch (...) {
    return 0;
  }
}

Location getLocation(const FileContent* fC, NodeId node,
                     const std::string& symbolName, SymbolTable* symbols) {
  if (!fC || !node || !symbols) {
    PathId fileId;
    return Location(fileId, 0, 0, symbols->registerSymbol(symbolName));
  }

  PathId fileId = fC->getFileId(node);
  uint32_t line = fC->Line(node);
  uint32_t column = getColumnSafe(fC, node);
  SymbolId obj = symbols->registerSymbol(symbolName);

  return Location(fileId, line, column, obj);
}

void reportError(const FileContent* fC, NodeId node,
                 const std::string& symbolName,
                 ErrorDefinition::ErrorType errorType, ErrorContainer* errors,
                 SymbolTable* symbols) {
  if (!fC || !node || !errors || !symbols) return;

  Location loc = getLocation(fC, node, symbolName, symbols);
  Error err(errorType, loc);
  errors->addError(err, false);
}

std::string extractName(const FileContent* fC, NodeId node,
                        const std::string& defaultName) {
  if (!fC || !node) return defaultName;

  auto stringNodes = fC->sl_collect_all(node, VObjectType::slStringConst);
  for (NodeId nameNode : stringNodes) {
    if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
      return std::string(fC->SymName(nameNode));
    }
  }

  NodeId child = fC->Child(node);
  if (child && fC->Type(child) == VObjectType::slStringConst) {
    return std::string(fC->SymName(child));
  }

  return defaultName;
}

std::string findForLoopVariableName(const FileContent* fC, NodeId forNode) {
  if (!fC || !forNode) return "<unknown>";

  NodeId forInit = InvalidNodeId;
  NodeId condition = InvalidNodeId;
  NodeId forStep = InvalidNodeId;

  for (NodeId tmp = fC->Sibling(forNode); tmp; tmp = fC->Sibling(tmp)) {
    VObjectType t = fC->Type(tmp);
    if (t == VObjectType::paFor_initialization && !forInit) {
      forInit = tmp;
    } else if (t == VObjectType::paExpression && !condition) {
      condition = tmp;
    } else if (t == VObjectType::paFor_step && !forStep) {
      forStep = tmp;
    }
  }

  if (forInit) {
    std::string name = extractName(fC, forInit, "");
    if (!name.empty()) return name;
  }
  if (condition) {
    std::string name = extractName(fC, condition, "");
    if (!name.empty()) return name;
  }
  if (forStep) {
    std::string name = extractName(fC, forStep, "");
  }

  return "<unknown>";
}

std::string extractVariableName(const FileContent* fC, NodeId parentNode) {
  if (!fC || !parentNode) return "<unknown>";

  auto listNodes = fC->sl_collect_all(
      parentNode, VObjectType::paList_of_variable_decl_assignments);
  for (NodeId listNode : listNodes) {
    auto assignNodes =
        fC->sl_collect_all(listNode, VObjectType::paVariable_decl_assignment);
    for (NodeId assignNode : assignNodes) {
      NodeId nameNode = fC->Child(assignNode);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
    }
  }
  return "<unknown>";
}

std::string extractParameterName(const FileContent* fC, NodeId parentNode) {
  if (!fC || !parentNode) return "<unknown>";

  auto listNodes =
      fC->sl_collect_all(parentNode, VObjectType::paList_of_param_assignments);
  for (NodeId listNode : listNodes) {
    auto assignNodes =
        fC->sl_collect_all(listNode, VObjectType::paParam_assignment);
    for (NodeId assignNode : assignNodes) {
      NodeId nameNode = fC->Child(assignNode);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
    }
  }
  return "<unknown>";
}

std::string trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) return "";
  auto end = s.find_last_not_of(" \t\n\r");
  return s.substr(start, end - start + 1);
}

}  // namespace Analyzer
