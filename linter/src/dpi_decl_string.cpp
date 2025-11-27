#include <cstdint>
#include <iostream>
#include <string>
#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

int main(int argc, const char** argv) {
    uint32_t code = 0;

    auto* symbolTable = new SymbolTable();
    auto* errors = new ErrorContainer(symbolTable);
    auto* clp = new CommandLineParser(errors, symbolTable, false, false);

    clp->noPython();
    clp->setParse(true);
    clp->setCompile(true);
    clp->setElaborate(true);
    clp->setwritePpOutput(true);
    clp->setCacheAllowed(false);

    bool success = clp->parseCommandLine(argc, argv);
    Design* the_design = nullptr;
    scompiler* compiler = nullptr;

    if (success && !clp->help()) {
        compiler = start_compiler(clp);
        the_design = get_design(compiler);
    }

    if (!the_design) {
        std::cerr << "No design created" << std::endl;
        return 1;
    }

    for (auto& it : the_design->getAllFileContents()) {
        const FileContent* fC = it.second;
        if (!fC) continue;

        NodeId root = fC->getRootNode();

        // Ищем все DPI-import/export
        auto dpiNodes = fC->sl_collect_all(root, VObjectType::paDpi_import_export);
        for (NodeId dpiId : dpiNodes) {
            // 1. Ищем узел IMPORT
            NodeId importNode = fC->Child(dpiId);
            if (!importNode || fC->Type(importNode) != VObjectType::paIMPORT) continue;

            // 2. Ищем StringLiteral внутри IMPORT
            NodeId stringNode = fC->Sibling(importNode);
            if (stringNode && fC->Type(stringNode) == VObjectType::slStringLiteral) {
                std::string dpiStr = std::string(fC->SymName(stringNode));
                if (!dpiStr.empty() && dpiStr.front() == '"' && dpiStr.back() == '"') {
                    dpiStr = dpiStr.substr(1, dpiStr.size() - 2);
                }

                // Убираем пробелы в начале и конце
                dpiStr.erase(0, dpiStr.find_first_not_of(" \t\n\r"));
                dpiStr.erase(dpiStr.find_last_not_of(" \t\n\r") + 1);

                if (dpiStr != "DPI-C" || dpiStr != "DPI") {
                    auto fileId = fC->getFileId(stringNode);
                    std::string fileName = std::string(FileSystem::getInstance()->toPath(fileId));
                    uint32_t line = fC->Line(stringNode);
                    std::cerr << "[SNT:DPI_DECLARATION_STRING] " << fileName
                              << ":" << line
                              << " - expecting \"DPI-C\" instead of \"" << dpiStr << "\""
                              << std::endl;
                }
            }

        }
    }

    if (success && !clp->help()) {
        shutdown_compiler(compiler);
    }

    delete clp;
    delete symbolTable;

    return code;
}
