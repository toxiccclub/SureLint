#pragma once

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

namespace Analyzer {

void checkParameterDynamicArray(const FileContent* fC);

std::string findParamName(const FileContent* fC, NodeId paramDeclId);

}  // namespace Analyzer
