#pragma once

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

namespace Analyzer {

// Основная функция проверки всех множественных конкатенаций {N{expr}}
void checkConcatenationMultiplier(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols);

}  // namespace Analyzer