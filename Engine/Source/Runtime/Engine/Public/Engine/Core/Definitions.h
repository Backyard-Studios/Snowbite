#pragma once

#define SB_UNIQUE_NAME_CONCAT_INNER(a, b) a ## b
#define SB_UNIQUE_NAME_CONCAT(a, b) SB_UNIQUE_NAME_CONCAT_INNER(a, b)

/**
 * Generates a unique name for a variable to be used in a macro.
 * @param prefix The prefix to use for the variable name.
 */
#define SB_UNIQUE_NAME(prefix) SB_UNIQUE_NAME_CONCAT(prefix, __LINE__)

#define SB_SAFE_RELEASE(Pointer) if (Pointer) { (Pointer)->Release(); (Pointer) = nullptr; }
#define SB_SAFE_DELETE(Pointer) if (Pointer) { delete (Pointer); (Pointer) = nullptr; }
#define SB_SAFE_DELETE_ARRAY(Pointer) if (Pointer) { delete[] (Pointer); (Pointer) = nullptr; }
#define SB_SAFE_RESET(Pointer) if (Pointer) { (Pointer).reset(); (Pointer) = nullptr; }

#define SB_CHECK_RESULT(Result, Message) if (FAILED(Result)) { FPlatform::Fatal(Message, Result); }

#define SB_RETURN_IF_FAILED(Result) if (FAILED(Result)) { return Result; }
