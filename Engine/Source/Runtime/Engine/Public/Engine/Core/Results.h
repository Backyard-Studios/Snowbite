#pragma once

#include <winerror.h>

#define E_TOO_MUCH_REFERENCES MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0001)

const char* GetResultDescription(HRESULT Result);
