#pragma once

#ifdef SB_LIBRARY_EXPORT
/**
 * This macro is used to export a class or function from the engine
 */
#	define SNOWBITE_API __declspec(dllexport)
/**
 * This macro is used to export a template class from the engine
 */
#	define SB_TEMPLATE_EXPORT
#else
/**
 * This macro is used to export a class or function from the engine
 */
#	define SNOWBITE_API __declspec(dllimport)
/**
 * This macro is used to export a template class from the engine
 */
#	define SB_TEMPLATE_EXPORT extern
#endif

// ReSharper disable CppClangTidyBugproneMacroParentheses
#define SB_DISABLE_COPY(Class) \
	Class(const Class&) = delete; \
	Class& operator=(const Class&) = delete;

#define SB_DISABLE_MOVE(Class) \
	Class(Class&&) = delete; \
	Class& operator=(Class&&) = delete;

#define SB_DISABLE_COPY_AND_MOVE(Class) \
	SB_DISABLE_COPY(Class) \
	SB_DISABLE_MOVE(Class)

#define SB_ENABLE_COPY(Class) \
	Class(const Class&) = default; \
	Class& operator=(const Class&) = default;

#define SB_ENABLE_MOVE(Class) \
	Class(Class&&) = default; \
	Class& operator=(Class&&) = default;

#define SB_ENABLE_COPY_AND_MOVE(Class) \
	SB_ENABLE_COPY(Class) \
	SB_ENABLE_MOVE(Class)
// ReSharper enable CppClangTidyBugproneMacroParentheses

/**
 * This macro is used to safely delete a pointer and set it to nullptr
 */
#define SB_SAFE_DELETE(Pointer) \
	if (Pointer) \
	{ \
		delete Pointer; \
		Pointer = nullptr; \
	}

/**
 * This macro is used to safely delete an array and set it to nullptr
 */
#define SB_SAFE_DELETE_ARRAY(Pointer) \
	if (Pointer) \
	{ \
		delete[] Pointer; \
		Pointer = nullptr; \
	}

/**
 * This macro is used to safely reset a shared_ptr or unique_ptr and set it to nullptr
 */
#define SB_SAFE_RESET(Pointer) \
	if (Pointer) \
	{ \
		Pointer.reset(); \
		Pointer = nullptr; \
	}

/**
 * This macro is used to export a standard library container with a specific class
 */
#define SB_EXPORT_STL_CONTAINER(Container, ClassName) \
	class ClassName; \
	SB_TEMPLATE_EXPORT template class SNOWBITE_API Container<ClassName>;

/**
 * This macro is used to make a version number. It uses bit shifting to store the version number in a single uint32_t
 */
#define SB_MAKE_VERSION(major, minor, patch) ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))
