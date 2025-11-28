#pragma once

#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include <string>

using namespace SURELOG;

namespace Analyzer{
void checkPrototypeReturnDataType(const FileContent* fC);

std::string getFunctionName(const FileContent* fC, NodeId typeNode);
bool hasReturnType(const FileContent* fC, NodeId typeNode);
void checkFunctionPrototype(const FileContent* fC, NodeId protoId);
}


