#include <map>
#include <set>
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

// Собираем все константные параметры (parameter, localparam)
std::set<std::string> collectConstantParameters(const FileContent* fC) {
  std::set<std::string> constants;

  // 1. Собираем parameters
  auto paramDecls = fC->sl_collect_all(fC->getRootNode(),
                                       VObjectType::paParameter_declaration);
  for (NodeId paramDeclId : paramDecls) {
    auto paramAssigns =
        fC->sl_collect_all(paramDeclId, VObjectType::paParam_assignment, false);
    for (NodeId assignId : paramAssigns) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        std::string name = std::string(fC->SymName(nameNode));
        constants.insert(name);
      }
    }
  }

  // 2. Собираем localparams
  auto localParamDecls = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paLocal_parameter_declaration);
  for (NodeId localParamId : localParamDecls) {
    auto paramAssigns = fC->sl_collect_all(
        localParamId, VObjectType::paParam_assignment, false);
    for (NodeId assignId : paramAssigns) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        std::string name = std::string(fC->SymName(nameNode));
        constants.insert(name);
      }
    }
  }

  return constants;
}

// Собираем все переменные
std::set<std::string> collectVariables(const FileContent* fC) {
  std::set<std::string> variables;

  // 1. Собираем переменные через Variable_declaration
  auto varDecls = fC->sl_collect_all(fC->getRootNode(),
                                     VObjectType::paVariable_declaration);
  for (NodeId varDeclId : varDecls) {
    auto varAssigns = fC->sl_collect_all(
        varDeclId, VObjectType::paVariable_decl_assignment, false);
    for (NodeId assignId : varAssigns) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        std::string name = std::string(fC->SymName(nameNode));
        variables.insert(name);
      }
    }
  }

  // 2. Собираем Data_declaration
  auto dataDecls =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paData_declaration);
  for (NodeId dataDeclId : dataDecls) {
    NodeId parent = fC->Parent(dataDeclId);
    if (parent &&
        (fC->Type(parent) == VObjectType::paParameter_declaration ||
         fC->Type(parent) == VObjectType::paLocal_parameter_declaration)) {
      continue;
    }

    auto varAssigns = fC->sl_collect_all(
        dataDeclId, VObjectType::paVariable_decl_assignment, false);
    for (NodeId assignId : varAssigns) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        std::string name = std::string(fC->SymName(nameNode));
        variables.insert(name);
      }
    }
  }

  return variables;
}

// Рекурсивная проверка, является ли выражение константным
bool isConstantExpression(const FileContent* fC, NodeId node,
                          const std::set<std::string>& constantParams,
                          const std::set<std::string>& variables,
                          std::string* nonConstantVar = nullptr) {
  if (!node) return true;

  VObjectType type = fC->Type(node);

  // 1. Узлы, явно помеченные как константные
  if (type == VObjectType::paConstant_expression ||
      type == VObjectType::paConstant_primary ||
      type == VObjectType::paConstant_mintypmax_expression ||
      type == VObjectType::paConstant_param_expression) {
    return true;
  }

  // 2. Числовые литералы всегда константы
  if (type == VObjectType::slIntConst || type == VObjectType::slRealConst ||
      type == VObjectType::paNumber_TickB0) {
    return true;
  }

  // Дополнительные числовые типы
  if (type >= VObjectType::paNumber_1Tickb0 &&
      type <= VObjectType::paNumber_1TickB1) {
    return true;
  }

  // 3. Идентификаторы
  if (type == VObjectType::slStringConst) {
    std::string name = std::string(fC->SymName(node));

    if (variables.count(name) > 0) {
      if (nonConstantVar) {
        *nonConstantVar = name;
      }
      return false;
    }

    if (constantParams.count(name) > 0) {
      return true;
    }

    return true;
  }

  // 4. Primary_literal
  if (type == VObjectType::paPrimary_literal) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  // 5. Primary
  if (type == VObjectType::paPrimary) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  // 6. Hierarchical_identifier или Ps_or_hierarchical_identifier
  if (type == VObjectType::paHierarchical_identifier ||
      type == VObjectType::paPs_or_hierarchical_identifier) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  // 7. Expression
  if (type == VObjectType::paExpression) {
    for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
      VObjectType childType = fC->Type(child);

      if (childType >= VObjectType::paBinOp_Plus &&
          childType <= VObjectType::paEdge_descriptor) {
        continue;
      }

      if (childType >= VObjectType::paUnary_Minus &&
          childType <= VObjectType::paUnary_Tilda) {
        continue;
      }

      if (!isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar)) {
        return false;
      }
    }
    return true;
  }

  // 8. Constant_range и другие специальные конструкции
  if (type == VObjectType::paConstant_range) {
    return true;
  }

  // 9. Mintypmax_expression
  if (type == VObjectType::paMintypmax_expression) {
    for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
      if (!isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar)) {
        return false;
      }
    }
    return true;
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (!isConstantExpression(fC, child, constantParams, variables,
                              nonConstantVar)) {
      return false;
    }
  }

  return true;
}

// Проверка одной конструкции {N{expr}}
void checkSingleMultipleConcatenation(
    const FileContent* fC, NodeId multiConcatNode,
    const std::set<std::string>& constantParams,
    const std::set<std::string>& variables, ErrorContainer* errors,
    SymbolTable* symbols) {
  if (!multiConcatNode) return;

  NodeId multiplierExpr = fC->Child(multiConcatNode);
  if (!multiplierExpr) return;

  std::string nonConstantVar;
  if (!isConstantExpression(fC, multiplierExpr, constantParams, variables,
                            &nonConstantVar)) {
    auto fileId = fC->getFileId(multiplierExpr);
    uint32_t line = fC->Line(multiplierExpr);
    uint32_t column = 0;

    try {
      column = fC->Column(multiplierExpr);
    } catch (...) {
    }

    SymbolId obj = symbols->registerSymbol(nonConstantVar);
    Location loc(fileId, line, column, obj);
    Error err(ErrorDefinition::LINT_CONCATENATION_MULTIPLIER, loc);
    errors->addError(err, false);
  }
}

// Основная функция проверки всех {N{expr}} в файле
void checkConcatenationMultiplier(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  std::set<std::string> constantParams = collectConstantParameters(fC);
  std::set<std::string> variables = collectVariables(fC);

  auto multiConcatNodes =
      fC->sl_collect_all(root, VObjectType::paMultiple_concatenation);

  for (NodeId node : multiConcatNodes) {
    checkSingleMultipleConcatenation(fC, node, constantParams, variables,
                                     errors, symbols);
  }
}

}  // namespace Analyzer