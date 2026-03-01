#pragma once

#include <cstdint>
#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

namespace Analyzer {

//Безопасно извлекает номер столбца из узла, возвращая 0, если он недоступен.
uint32_t getColumnSafe(const FileContent* fC, NodeId node);

//Создать объект Location из узла.
Location getLocation(const FileContent* fC, NodeId node,
                     const std::string& symbolName, SymbolTable* symbols);

//Сообщить об ошибке линтинга в определенном месте узла.
//Это основная функция отчетности об ошибках, используемая всеми правилами.
void reportError(const FileContent* fC, NodeId node,
                 const std::string& symbolName,
                 ErrorDefinition::ErrorType errorType, ErrorContainer* errors,
                 SymbolTable* symbols);

//Извлечь первое имя StringConst из поддерева узла.
//Возвращает defaultName, если имя не найдено.
std::string extractName(const FileContent* fC, NodeId node,
                        const std::string& defaultName = "<unknown>");

//Извлечь параметр цикла
//Используется для определения порпуска параметра
std::string findForLoopVariableName(const FileContent* fC, NodeId forNode);

//Извлечь имя переменной из шаблона Variable_decl_assignment.
//Используется для поиска имен переменных в объявлениях.
std::string extractVariableName(const FileContent* fC, NodeId parentNode);

//Извлечение имени параметра из шаблона Param_assignment.
//Используется для поиска имен параметров в декларациях.
std::string extractParameterName(const FileContent* fC, NodeId parentNode);

std::string trim(const std::string& s);

std::string findLhsVariableName(const FileContent* fC, NodeId startNode);

NodeId findEnclosingModule(const FileContent* fC, NodeId node);

}  // namespace Analyzer
