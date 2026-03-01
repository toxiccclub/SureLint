#pragma once

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

namespace Analyzer {

// Основная функция проверки использования конкатенации unpacked-массивов
// как целевого выражения (lvalue) в двух случаях:
//   1. {a, b} = value;              — прямое присваивание
//   2. .port_name({ a, b })         — подключение к порту модуля
void checkTargetUnpackedArrayConcatenation(const FileContent* fC,
                                           ErrorContainer* errors,
                                           SymbolTable* symbols);

}  // namespace Analyzer