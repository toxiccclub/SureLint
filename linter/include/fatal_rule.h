#pragma once

#include <uhdm/uhdm.h>
#include <uhdm/VpiListener.h>
#include <vector>

class FatalListener : public UHDM::VpiListener {
public:

    FatalListener() = default;

    void listen(const vpiHandle& design);

    void enterSys_func_call(const UHDM::sys_func_call* object, vpiHandle handle) override;

private:
    std::set<const UHDM::sys_func_call*> seen_;
};
