#include "implicit_data_type.h"

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

// Извлечь имя переменной из Variable_decl_assignment
std::string findVarName(const FileContent* fC, NodeId dataDecl) {
  return extractVariableName(fC, dataDecl);
}

// Проверка наличия явного типа
bool hasExplicitType(const FileContent* fC, NodeId dataDecl) {
  static const VObjectType typeNodes[] = {
      VObjectType::paNet_type,          VObjectType::paData_type,
      VObjectType::paInteger_atom_type, VObjectType::paInteger_vector_type,
      VObjectType::paNon_integer_type,  VObjectType::paString_type,
      VObjectType::paClass_type,        VObjectType::paIntVec_TypeBit};

  for (auto t : typeNodes) {
    if (!fC->sl_collect_all(dataDecl, t).empty()) return true;
  }
  return false;
}

void checkImplicitDataTypeInDeclaration(const FileContent* fC,
                                        ErrorContainer* errors,
                                        SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  // Ищем Data_declaration
  auto dataDecls = fC->sl_collect_all(root, VObjectType::paData_declaration);

  for (NodeId dataDecl : dataDecls) {
    auto packedDims =
        fC->sl_collect_all(dataDecl, VObjectType::paPacked_dimension);
    if (packedDims.empty()) continue;

    if (hasExplicitType(fC, dataDecl)) continue;

    // Error: IMPLICIT_DATA_TYPE_IN_DECLARATION
    std::string varName = findVarName(fC, dataDecl);
    NodeId where = packedDims.front();

    reportError(fC, where, varName, ErrorDefinition::LINT_IMPLICIT_DATA_TYPE,
                errors, symbols);
  }
}
}  // namespace Analyzer
