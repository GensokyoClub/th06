// Helpers to improve the diff quality of DIFFBUILDs.
//
// A DIFFBUILD is a build of the project made not to be run, but to be diffed
// against the original TH06 to find inaccuracies in our reimplementation. To
// make a DIFFBUILD, one must call the makefile with DIFFBUILD=1
//
// This helper provides two macros: DIFFABLE_EXTERN and DIFFABLE_STATIC. All
// static variables should be defined and declared through those macros. So for
// instance, instead of
//
// ```
// Stage g_Stage;
// ```
//
// One should write
//
// ```
// DIFFABLE_STATIC(Stage, g_Stage)
// ```
//
// The first argument is the type, while the second argument is the name of the
// static variable.
//
// When assigning to an array, the `DIFFABLE_STATIC_ARRAY_ASSIGN` macro should
// be used like this:
//
// ```
// DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 5, g_ArrayName) = { 0, 1, 2 };
// ```

#pragma once

#ifdef DIFFBUILD
#define DIFFABLE_EXTERN(type, name) extern "C" type name;
#define DIFFABLE_EXTERN_ARRAY(type, size, name) extern "C" type name[size];
#define DIFFABLE_STATIC(type, name) extern "C" type name;
#define DIFFABLE_STATIC_ARRAY(type, size, name) extern "C" type name[size];
// This macro is meant to be used like so:
// DIFFABLE_STATIC_ARRAY_ASSIGN(u32, g_ArrayName) = 12;
//
// In diffbuild, we want to discard the content of the array, so we generate a
// second, fake static, that we store in a template<> to make sure it doesn't
// get instanciated.
#define DIFFABLE_STATIC_ASSIGN(type, name)                                                                             \
    extern "C" type name;                                                                                              \
    template <> type DIFFBUILD_HIDE_NAME_##name
// This macro is meant to be used like so:
// DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 5, g_ArrayName) = { 0, 1, 2 };
//
// In diffbuild, we want to discard the content of the array, so we generate a
// second, fake static, that we store in a template<> to make sure it doesn't
// get instanciated.
#define DIFFABLE_STATIC_ARRAY_ASSIGN(type, size, name)                                                                 \
    extern "C" type name[size];                                                                                        \
    template <> type DIFFBUILD_HIDE_NAME_##name[size]
#else
#define DIFFABLE_EXTERN_ARRAY(type, size, name) extern "C" type name[size]
#define DIFFABLE_STATIC_ARRAY(type, size, name) type name[size]
#define DIFFABLE_STATIC_ASSIGN(type, name) type name
#define DIFFABLE_STATIC_ARRAY_ASSIGN(type, size, name) type name[size]
#endif

#if defined(BINARYMATCHBUILD) || defined(DIFFBUILD)
#define ZUN_ASSERT_SIZE(type, size) static_assert(sizeof(type) == size);
#else
#define ZUN_ASSERT_SIZE(type, size) static_assert(true);
#endif
