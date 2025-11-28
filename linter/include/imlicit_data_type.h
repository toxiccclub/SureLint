#pragma once

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include <string>

using namespace SURELOG;

namespace Analyzer{

std::string findVarName(const FileContent* fC, NodeId dataDecl);

bool hasExplicitType(const FileContent* fC, NodeId dataDecl);

void checkImplicitDataTypeInDeclaration(const FileContent* fC);

}