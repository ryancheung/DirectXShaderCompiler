//===- WinIncludes.h --------------------------------------------*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// WinIncludes.h                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _MSC_VER

// winapifamily.h must be included before using WINAPI_FAMILY_PARTITION.
#include <winapifamily.h>

// mingw-w64 tends to define it as 0x0502 in its headers.
#undef _WIN32_WINNT
#undef _WIN32_IE

// UWP (Windows Store) apps run on Windows 10+; desktop requires at least Win7.
#if defined(WINAPI_FAMILY) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define _WIN32_WINNT 0x0A00
#else
#define _WIN32_WINNT 0x0601
#endif
#define _WIN32_IE 0x0800 // MinGW at it again.

#define NOATOM 1
#define NOGDICAPMASKS 1
#define NOMETAFILE 1
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#define NOOPENFILE 1
#define NORASTEROPS 1
#define NOSCROLL 1
#define NOSOUND 1
#define NOSYSMETRICS 1
#define NOWH 1
#define NOCOMM 1
#define NOKANJI 1
#define NOCRYPT 1
#define NOMCX 1
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NONAMELESSSTRUCT 1

#include <ObjIdl.h>
// On UWP builds with older Windows SDKs, IMalloc may be guarded behind
// WINAPI_PARTITION_DESKTOP and thus absent from objidlbase.h.  Provide a
// definition here so that FileIOHelper.h and ATL headers can use it.
// __IMalloc_INTERFACE_DEFINED__ is set by the SDK when it defines IMalloc,
// so this block is a no-op on newer SDKs or desktop builds.
#ifndef __IMalloc_INTERFACE_DEFINED__
#define __IMalloc_INTERFACE_DEFINED__
MIDL_INTERFACE("00000002-0000-0000-C000-000000000046")
IMalloc : public IUnknown {
public:
  virtual void *STDMETHODCALLTYPE Alloc(SIZE_T cb) = 0;
  virtual void *STDMETHODCALLTYPE Realloc(void *pv, SIZE_T cb) = 0;
  virtual void STDMETHODCALLTYPE Free(void *pv) = 0;
  virtual SIZE_T STDMETHODCALLTYPE GetSize(void *pv) = 0;
  virtual int STDMETHODCALLTYPE DidAlloc(void *pv) = 0;
  virtual void STDMETHODCALLTYPE HeapMinimize(void) = 0;
};
#endif // __IMalloc_INTERFACE_DEFINED__
// In UWP/WinRT builds, prevent ATL from auto-linking desktop-only ATL libraries.
#if defined(WINAPI_FAMILY) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#ifndef _ATL_NO_DEFAULT_LIBS
#define _ATL_NO_DEFAULT_LIBS
#endif
#endif
#include <atlbase.h> // atlbase.h needs to come before strsafe.h
#include <intsafe.h>
#include <strsafe.h>
#include <unknwn.h>
#include <windows.h>

#include "dxc/config.h"

// Support older atlbase.h if needed
#ifndef _ATL_DECLSPEC_ALLOCATOR
#define _ATL_DECLSPEC_ALLOCATOR
#endif

/// Swap two ComPtr classes.
template <class T> void swap(CComHeapPtr<T> &a, CComHeapPtr<T> &b) {
  T *c(a.m_pData);
  a.m_pData = b.m_pData;
  b.m_pData = c;
}

/// DxcCreateFileW
/// CreateFile2 is available in all WINAPI partitions (including UWP/App).
/// CreateFileW was re-added to WINAPI_PARTITION_APP in SDK 14393, but using
/// CreateFile2 is safer and unambiguous across all supported SDK versions.
#if defined(WINAPI_FAMILY) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
inline HANDLE DxcCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                             DWORD dwShareMode, DWORD dwCreationDisposition,
                             DWORD dwFlagsAndAttributes) {
  CREATEFILE2_EXTENDED_PARAMETERS ex = {};
  ex.dwSize = sizeof(ex);
  ex.dwFileAttributes = dwFlagsAndAttributes & 0x0000FFFFu;
  ex.dwFileFlags      = dwFlagsAndAttributes & 0xFFF00000u;
  return ::CreateFile2(lpFileName, dwDesiredAccess, dwShareMode,
                       dwCreationDisposition, &ex);
}
#else
inline HANDLE DxcCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                             DWORD dwShareMode, DWORD dwCreationDisposition,
                             DWORD dwFlagsAndAttributes) {
  return ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, nullptr,
                       dwCreationDisposition, dwFlagsAndAttributes, nullptr);
}
#endif

#else // _MSC_VER

#include "dxc/WinAdapter.h"

#ifdef __cplusplus
#if !defined(DEFINE_ENUM_FLAG_OPERATORS)
// Define operator overloads to enable bit operations on enum values that are
// used to define flags. Use DEFINE_ENUM_FLAG_OPERATORS(YOUR_TYPE) to enable
// these operators on YOUR_TYPE.
extern "C++" {
template <size_t S> struct _ENUM_FLAG_INTEGER_FOR_SIZE;

template <> struct _ENUM_FLAG_INTEGER_FOR_SIZE<1> { typedef int8_t type; };

template <> struct _ENUM_FLAG_INTEGER_FOR_SIZE<2> { typedef int16_t type; };

template <> struct _ENUM_FLAG_INTEGER_FOR_SIZE<4> { typedef int32_t type; };

// used as an approximation of std::underlying_type<T>
template <class T> struct _ENUM_FLAG_SIZED_INTEGER {
  typedef typename _ENUM_FLAG_INTEGER_FOR_SIZE<sizeof(T)>::type type;
};
}
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)                                   \
  extern "C++" {                                                               \
  inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b) {                          \
    return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) |            \
                    ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));            \
  }                                                                            \
  inline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE b) {                       \
    return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |=     \
                        ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));        \
  }                                                                            \
  inline ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b) {                          \
    return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) &            \
                    ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));            \
  }                                                                            \
  inline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE b) {                       \
    return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &=     \
                        ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));        \
  }                                                                            \
  inline ENUMTYPE operator~(ENUMTYPE a) {                                      \
    return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a));           \
  }                                                                            \
  inline ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b) {                          \
    return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^            \
                    ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));            \
  }                                                                            \
  inline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE b) {                       \
    return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^=     \
                        ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b));        \
  }                                                                            \
  }
#endif // !defined(DEFINE_ENUM_FLAG_OPERATORS)
#else
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif

#endif // _MSC_VER

/// DxcCoGetMalloc
/// CoGetMalloc is only available under WINAPI_PARTITION_DESKTOP.
/// For UWP builds (and non-Windows), use the custom DxcCoMalloc implementation.
#if defined(_WIN32) && !defined(DXC_DISABLE_ALLOCATOR_OVERRIDES) &&           \
    (!defined(WINAPI_FAMILY) ||                                                \
     WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))

#define DxcCoGetMalloc CoGetMalloc

#else // desktop Windows with overrides

#ifndef _WIN32
struct IMalloc;
#endif

HRESULT DxcCoGetMalloc(DWORD dwMemContext, IMalloc **ppMalloc);

#endif // DxcCoGetMalloc
