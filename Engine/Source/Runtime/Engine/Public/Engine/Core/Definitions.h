#pragma once

#ifdef SB_LIBRARY_EXPORT
/*
 * This macro is used to export a class or function from the engine
 */
#	define SNOWBITE_API __declspec(dllexport)
/*
 * This macro is used to export a template class from the engine
 */
#	define SB_TEMPLATE_EXPORT
#else
/*
 * This macro is used to export a class or function from the engine
 */
#	define SNOWBITE_API __declspec(dllimport)
/*
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

/*
 * This macro is used to safely delete a pointer and set it to nullptr
 */
#define SB_SAFE_DELETE(ptr) \
	if (ptr) \
	{ \
		delete ptr; \
		ptr = nullptr; \
	}

/*
 * This macro is used to safely delete an array and set it to nullptr
 */
#define SB_SAFE_DELETE_ARRAY(ptr) \
	if (ptr) \
	{ \
		delete[] ptr; \
		ptr = nullptr; \
	}

/*
 * This macro is used to safely reset a shared_ptr or unique_ptr and set it to nullptr
 */
#define SB_SAFE_RESET(ptr) \
	if (ptr) \
	{ \
		ptr.reset(); \
		ptr = nullptr; \
	}

/*
 * This macro is used to export a standard library container with a specific class
 */
#define SB_EXPORT_STL_CONTAINER(container, className) \
	class className; \
	SB_TEMPLATE_EXPORT template class SNOWBITE_API container<className>;
