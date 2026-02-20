#pragma once

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

namespace Analyzer {

void checkForeachLoopCondition(const SURELOG::FileContent* fC,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);

}