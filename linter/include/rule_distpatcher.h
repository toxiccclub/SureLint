#pragma once

#include <string>
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/Design/Design.h"

void runAllRules(const SURELOG::FileContent* fC);

void runAllRulesOnDesign(SURELOG::Design* design);