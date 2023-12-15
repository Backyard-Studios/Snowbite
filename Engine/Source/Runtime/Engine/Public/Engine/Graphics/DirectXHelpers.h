#pragma once

#include <Engine/Core/Assert.h>

#define SB_D3D_ASSERT(Result, Message) SB_ASSERT_CRITICAL(SUCCEEDED(Result), Result, Message);
