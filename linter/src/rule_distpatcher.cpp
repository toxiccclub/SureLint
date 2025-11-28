#include <iostream>
#include "Surelog/Common/FileSystem.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "Surelog/linter/repet_in_sequence.h"
#include "Surelog/linter/prototype_return_data_type.h"
#include "Surelog/linter/parametr_dynamic_array.h"
#include "Surelog/Design/FileContent.h"

using namespace SURELOG;

void runAllRules(const FileContent* fC) {
    Analyzer::checkRepetitionInSequence(fC);
    Analyzer::checkPrototypeReturnDataType(fC);
    Analyzer::checkParameterDynamicArray(fC);
}


void runAllRulesOnDesign(Design* design) {
    if (!design) return;

    for (auto& it : design->getAllFileContents()) {
        const FileContent* fC = it.second;
        if (!fC) continue;

        runAllRules(fC);
    }
}