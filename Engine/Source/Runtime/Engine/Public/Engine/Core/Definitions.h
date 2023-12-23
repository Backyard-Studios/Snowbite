#pragma once

#define SB_UNIQUE_NAME_CONCAT_INNER(a, b) a ## b
#define SB_UNIQUE_NAME_CONCAT(a, b) SB_UNIQUE_NAME_CONCAT_INNER(a, b)

/**
 * Generates a unique name for a variable to be used in a macro.
 * @param prefix The prefix to use for the variable name.
 */
#define SB_UNIQUE_NAME(prefix) SB_UNIQUE_NAME_CONCAT(prefix, __LINE__)
