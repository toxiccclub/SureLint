#pragma once

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

namespace Analyzer {

// Проверяет, что у всех операторов/генераторов `for` присутствует
// инициализация счётчика цикла.
void checkMissingForLoopInitialization(const FileContent* fC,
                                       ErrorContainer* errors,
                                       SymbolTable* symbols);

}  // namespace Analyzer
