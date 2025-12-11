#pragma once
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

bool hasMultipleDotStarConnections(const FileContent* fC, NodeId instNode);

void reportMultipleDotStarError(const FileContent* fC, NodeId badNode,
                                ErrorContainer* errors, SymbolTable* symbols);

void checkMultipleDotStarConnections(const FileContent* fC,
                                     ErrorContainer* errors,
                                     SymbolTable* symbols);

}  // namespace Analyzer
