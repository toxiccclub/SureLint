#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

// UHDM
#include <uhdm/VpiListener.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

class FatalListener : public UHDM::VpiListener {
    // Чтобы избежать двойных выводов
    std::set<const UHDM::sys_func_call*> seen_;

public:
    void enterSys_func_call(const UHDM::sys_func_call* object, vpiHandle handle) override {
        if (!object) return;

        // Не печатать повторно
        if (seen_.count(object)) return;
        seen_.insert(object);

        // Проверяем имя
        if (object->VpiName() != "$fatal") return;

        // --- Локация ---
        const char* file = nullptr;
        int line = 0;
        if (handle) {
            file = vpi_get_str(vpiFile, handle);
            line = vpi_get(vpiLineNo, handle);
        }
        const UHDM::VectorOfany* args = object->Tf_call_args();
        /*std::cout << "[FatalListener] Found $fatal";
        if (file) std::cout << " at " << file << ":" << line;
        std::cout << "\n";

        // --- Аргументы ---
        const UHDM::VectorOfany* args = object->Tf_call_args();
        if (!args || args->empty()) {
            std::cout << "  Error: $fatal has no arguments\n";
            return;
        }*/

        UHDM::any* firstArg = (*args)[0];
        if (!firstArg) return;

        int val = 0;
        bool isInteger = false;

        // -----------------------
        // CASE 1: CONSTANT
        // -----------------------
        if (auto c = dynamic_cast<UHDM::constant*>(firstArg)) {
            int ctype = c->VpiConstType();
            isInteger = (ctype == vpiIntConst || ctype == vpiDecConst ||
                         ctype == vpiHexConst || ctype == vpiOctConst ||
                         ctype == vpiBinaryConst || ctype == vpiUIntConst);

            if (!isInteger) {
                std::cout << "  Error: First argument is not an integer constant\n";
                return;
            }

            std::string raw = std::string(c->VpiValue());
            size_t pos = raw.find(':');
            if (pos != std::string::npos) raw = raw.substr(pos + 1);

            try { val = std::stoi(raw); }
            catch (...) {
                std::cout << "  Error: Cannot parse integer: '" << raw << "'\n";
                return;
            }
        }

        // -----------------------
        // CASE 2: UNARY +/- OPERATION
        // -----------------------
        else if (auto op = dynamic_cast<UHDM::operation*>(firstArg)) {
            int opType = op->VpiOpType();

            if ((opType == vpiPlusOp || opType == vpiMinusOp) &&
                op->Operands() && !op->Operands()->empty()) {

                if (auto c = dynamic_cast<UHDM::constant*>((*op->Operands())[0])) {
                    std::string raw = std::string(c->VpiValue());
                    size_t pos = raw.find(':');
                    if (pos != std::string::npos) raw = raw.substr(pos + 1);

                    try { val = std::stoi(raw); }
                    catch (...) {
                        std::cout << "  Error: Cannot parse integer: '" << raw << "'\n";
                        return;
                    }

                    if (opType == vpiMinusOp) val = -val;
                    isInteger = true;
                }
            }
        }

        // -----------------------
        // CASE 3: ref_obj → не константа
        // -----------------------
        else if (dynamic_cast<UHDM::ref_obj*>(firstArg)) {
            std::cout << "  Error: First argument is not constant (ref_obj)\n";
            return;
        }

        // -----------------------
        // Final check
        // -----------------------
        if (isInteger) {
            if (!(val == 0 || val == 1 || val == 2)) {
                std::cout << "  Error: First argument must be 0, 1, or 2. Got: " << val << "\n";
            }
        } else {
            std::cout << "  Error: First argument is not constant; cannot verify\n";
        }

        // -----------------------
        // SECOND ARG (MESSAGE)
        // -----------------------
        if (args->size() > 1) {
            auto secondArg = (*args)[1];
            if (auto str = dynamic_cast<UHDM::constant*>(secondArg)) {
                
            } else {
                std::cout << "  Warning: message is not a string constant\n";
            }
        } else {
            std::cout << "  Warning: Missing message\n";
        }
    }
};


int main(int argc, const char** argv) {
    uint32_t code = 0;

    auto* symbolTable = new SURELOG::SymbolTable();
    auto* errors = new SURELOG::ErrorContainer(symbolTable);
    auto* clp = new SURELOG::CommandLineParser(errors, symbolTable, false, false);

    clp->noPython();
    clp->setCacheAllowed(false);
    clp->setParse(true);
    clp->setwritePpOutput(true);
    clp->setCompile(true);
    clp->setElaborate(true);

    bool success = clp->parseCommandLine(argc, argv);

    vpiHandle the_design = nullptr;
    SURELOG::scompiler* compiler = nullptr;

    if (success && !clp->help()) {
        compiler = SURELOG::start_compiler(clp);
        the_design = SURELOG::get_uhdm_design(compiler);
    }

    if (the_design) {
        std::cout << "UHDM walk...\n";

        FatalListener listener;
        listener.listenDesigns({the_design}); 
    }

    SURELOG::shutdown_compiler(compiler);
    delete clp;
    delete symbolTable;
    delete errors;
    return code;
}