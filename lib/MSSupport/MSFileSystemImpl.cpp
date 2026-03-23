//===- llvm/Support/Windows/MSFileSystemImpl.cpp DXComplier Impl *- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MSFileSystemImpl.cpp                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file implements the DXCompiler specific implementation of the Path   //
//   API.                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <new>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dxc/Support/WinIncludes.h"
#ifdef _WIN32
#include <fileapi.h>
#endif
#include "dxc/WinAdapter.h"
#include "llvm/Support/MSFileSystem.h"

////////////////////////////////////////////////////////////////////////////////
// Externally visible functions.

HRESULT
CreateMSFileSystemForDisk(::llvm::sys::fs::MSFileSystem **pResult) throw();

////////////////////////////////////////////////////////////////////////////////
// Win32-and-CRT-based MSFileSystem implementation with direct filesystem
// access.

namespace llvm {
namespace sys {
namespace fs {

class MSFileSystemForDisk : public MSFileSystem {
public:
  unsigned _defaultAttributes;
  MSFileSystemForDisk();

  virtual BOOL
  FindNextFileW(HANDLE hFindFile,
                LPWIN32_FIND_DATAW lpFindFileData) throw() override;
  virtual HANDLE
  FindFirstFileW(LPCWSTR lpFileName,
                 LPWIN32_FIND_DATAW lpFindFileData) throw() override;
  virtual void FindClose(HANDLE findHandle) throw() override;
  virtual HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                             DWORD dwShareMode, DWORD dwCreationDisposition,
                             DWORD dwFlagsAndAttributes) throw() override;
  virtual BOOL SetFileTime(HANDLE hFile, const FILETIME *lpCreationTime,
                           const FILETIME *lpLastAccessTime,
                           const FILETIME *lpLastWriteTime) throw() override;
  virtual BOOL GetFileInformationByHandle(
      HANDLE hFile,
      LPBY_HANDLE_FILE_INFORMATION lpFileInformation) throw() override;
  virtual DWORD GetFileType(HANDLE hFile) throw() override;
  virtual BOOL CreateHardLinkW(LPCWSTR lpFileName,
                               LPCWSTR lpExistingFileName) throw() override;
  virtual BOOL MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,
                           DWORD dwFlags) throw() override;
  virtual DWORD GetFileAttributesW(LPCWSTR lpFileName) throw() override;
  virtual BOOL CloseHandle(HANDLE hObject) throw() override;
  virtual BOOL DeleteFileW(LPCWSTR lpFileName) throw() override;
  virtual BOOL RemoveDirectoryW(LPCWSTR lpFileName) throw() override;
  virtual BOOL CreateDirectoryW(LPCWSTR lpPathName) throw() override;
  virtual DWORD GetCurrentDirectoryW(DWORD nBufferLength,

                                     LPWSTR lpBuffer) throw() override;
  virtual DWORD GetMainModuleFileNameW(LPWSTR lpFilename,
                                       DWORD nSize) throw() override;
  virtual DWORD GetTempPathW(DWORD nBufferLength,

                             LPWSTR lpBuffer) throw() override;
  virtual BOOLEAN CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName,
                                      LPCWSTR lpTargetFileName,
                                      DWORD dwFlags) throw() override;
  virtual bool SupportsCreateSymbolicLink() throw() override;
  virtual BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer,
                        DWORD nNumberOfBytesToRead,
                        LPDWORD lpNumberOfBytesRead) throw() override;
  virtual HANDLE CreateFileMappingW(HANDLE hFile, DWORD flProtect,
                                    DWORD dwMaximumSizeHigh,
                                    DWORD dwMaximumSizeLow) throw() override;
  virtual LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
                               DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                               SIZE_T dwNumberOfBytesToMap) throw() override;
  virtual BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) throw() override;

  // Console APIs.
  virtual bool FileDescriptorIsDisplayed(int fd) throw() override;
  virtual unsigned GetColumnCount(DWORD nStdHandle) throw() override;
  virtual unsigned GetConsoleOutputTextAttributes() throw() override;
  virtual void
  SetConsoleOutputTextAttributes(unsigned attributes) throw() override;
  virtual void ResetConsoleOutputTextAttributes() throw() override;

  // CRT APIs.
  virtual int open_osfhandle(intptr_t osfhandle, int flags) throw() override;
  virtual intptr_t get_osfhandle(int fd) throw() override;
  virtual int close(int fd) throw() override;
  virtual long lseek(int fd, long offset, int origin) throw() override;
  virtual int setmode(int fd, int mode) throw() override;
  virtual errno_t resize_file(LPCWSTR path, uint64_t size) throw() override;
  virtual int Read(int fd, void *buffer, unsigned int count) throw() override;
  virtual int Write(int fd, const void *buffer,
                    unsigned int count) throw() override;
#ifndef _WIN32
  virtual int Open(const char *lpFileName, int flags,
                   mode_t mode) throw() override;
  virtual int Stat(const char *lpFileName,
                   struct stat *Status) throw() override;
  virtual int Fstat(int FD, struct stat *Status) throw() override;
#endif
};

MSFileSystemForDisk::MSFileSystemForDisk() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  _defaultAttributes = GetConsoleOutputTextAttributes();
#else
  _defaultAttributes = 0;
#endif
#endif
}

BOOL MSFileSystemForDisk::FindNextFileW(
    HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) throw() {
#ifdef _WIN32
  return ::FindNextFileW(hFindFile, lpFindFileData);
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

HANDLE
MSFileSystemForDisk::FindFirstFileW(LPCWSTR lpFileName,
                                    LPWIN32_FIND_DATAW lpFindFileData) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::FindFirstFileW(lpFileName, lpFindFileData);
#else
  return ::FindFirstFileExW(lpFileName, FindExInfoBasic, lpFindFileData,
                            FindExSearchNameMatch, nullptr, 0);
#endif
#else
  assert(false && "Not implemented for Unix");
  return nullptr;
#endif
}

void MSFileSystemForDisk::FindClose(HANDLE findHandle) throw() {
#ifdef _WIN32
  ::FindClose(findHandle);
#else
  assert(false && "Not implemented for Unix");
#endif
}

HANDLE MSFileSystemForDisk::CreateFileW(LPCWSTR lpFileName,
                                        DWORD dwDesiredAccess,
                                        DWORD dwShareMode,
                                        DWORD dwCreationDisposition,
                                        DWORD dwFlagsAndAttributes) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, nullptr,
                       dwCreationDisposition, dwFlagsAndAttributes, nullptr);
#else
  CREATEFILE2_EXTENDED_PARAMETERS params = {};
  params.dwSize = sizeof(params);
  params.dwFileAttributes = dwFlagsAndAttributes & 0xFFFFu;
  params.dwFileFlags = dwFlagsAndAttributes & 0xFFF00000u;
  return ::CreateFile2(lpFileName, dwDesiredAccess, dwShareMode,
                       dwCreationDisposition, &params);
#endif
#else
  assert(false && "Not implemented for Unix");
  return nullptr;
#endif
}

BOOL MSFileSystemForDisk::SetFileTime(HANDLE hFile,
                                      const FILETIME *lpCreationTime,
                                      const FILETIME *lpLastAccessTime,
                                      const FILETIME *lpLastWriteTime) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::SetFileTime(hFile, lpCreationTime, lpLastAccessTime,
                       lpLastWriteTime);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::GetFileInformationByHandle(
    HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::GetFileInformationByHandle(hFile, lpFileInformation);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

DWORD
MSFileSystemForDisk::GetFileType(HANDLE hFile) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::GetFileType(hFile);
#else
  return FILE_TYPE_UNKNOWN;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

BOOL MSFileSystemForDisk::CreateHardLinkW(LPCWSTR lpFileName,
                                          LPCWSTR lpExistingFileName) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::CreateHardLinkW(lpFileName, lpExistingFileName, nullptr);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::MoveFileExW(LPCWSTR lpExistingFileName,
                                      LPCWSTR lpNewFileName,
                                      DWORD dwFlags) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

DWORD
MSFileSystemForDisk::GetFileAttributesW(LPCWSTR lpFileName) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::GetFileAttributesW(lpFileName);
#else
  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!::GetFileAttributesExW(lpFileName, GetFileExInfoStandard, &data))
    return INVALID_FILE_ATTRIBUTES;
  return data.dwFileAttributes;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

BOOL MSFileSystemForDisk::CloseHandle(HANDLE hObject) throw() {
#ifdef _WIN32
  return ::CloseHandle(hObject);
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::DeleteFileW(LPCWSTR lpFileName) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::DeleteFileW(lpFileName);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::RemoveDirectoryW(LPCWSTR lpFileName) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::RemoveDirectoryW(lpFileName);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::CreateDirectoryW(LPCWSTR lpPathName) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::CreateDirectoryW(lpPathName, nullptr);
#else
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

DWORD MSFileSystemForDisk::GetCurrentDirectoryW(DWORD nBufferLength,
                                                LPWSTR lpBuffer) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::GetCurrentDirectoryW(nBufferLength, lpBuffer);
#else
  return 0;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

DWORD MSFileSystemForDisk::GetMainModuleFileNameW(LPWSTR lpFilename,
                                                  DWORD nSize) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  // Add some code to ensure that the result is null terminated.
  if (nSize <= 1) {
    ::SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return 0;
  }

  DWORD result = ::GetModuleFileNameW(nullptr, lpFilename, nSize - 1);
  if (result == 0)
    return result;
  lpFilename[result] = L'\0';
  return result;
#else
  // GetModuleFileNameW is not available in UWP.
  ::SetLastError(ERROR_NOT_SUPPORTED);
  return 0;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

DWORD MSFileSystemForDisk::GetTempPathW(DWORD nBufferLength,
                                        LPWSTR lpBuffer) throw() {
#ifdef _WIN32
  // GetTempPathW is available in both WINAPI_PARTITION_DESKTOP and
  // WINAPI_PARTITION_APP (UWP). GetTempPath2W is a newer desktop-only API
  // not reliably present in all SDK import libraries for UWP targets.
  return ::GetTempPathW(nBufferLength, lpBuffer);
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
namespace {
typedef BOOLEAN(WINAPI *PtrCreateSymbolicLinkW)(
    /*__in*/ LPCWSTR lpSymlinkFileName,
    /*__in*/ LPCWSTR lpTargetFileName,
    /*__in*/ DWORD dwFlags);

PtrCreateSymbolicLinkW create_symbolic_link_api =
    PtrCreateSymbolicLinkW(::GetProcAddress(::GetModuleHandleW(L"Kernel32.dll"),
                                            "CreateSymbolicLinkW"));
} // namespace
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#endif

BOOLEAN MSFileSystemForDisk::CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName,
                                                 LPCWSTR lpTargetFileName,
                                                 DWORD dwFlags) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return create_symbolic_link_api(lpSymlinkFileName, lpTargetFileName, dwFlags);
#else
  // CreateSymbolicLinkW is not available in UWP.
  return FALSE;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

bool MSFileSystemForDisk::SupportsCreateSymbolicLink() throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return create_symbolic_link_api != nullptr;
#else
  return false;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

BOOL MSFileSystemForDisk::ReadFile(HANDLE hFile, LPVOID lpBuffer,
                                   DWORD nNumberOfBytesToRead,
                                   LPDWORD lpNumberOfBytesRead) throw() {
#ifdef _WIN32
  return ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                    nullptr);
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

HANDLE MSFileSystemForDisk::CreateFileMappingW(HANDLE hFile, DWORD flProtect,
                                               DWORD dwMaximumSizeHigh,
                                               DWORD dwMaximumSizeLow) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::CreateFileMappingW(hFile, nullptr, flProtect, dwMaximumSizeHigh,
                              dwMaximumSizeLow, nullptr);
#else
  ULONG64 maxSize = ((ULONG64)dwMaximumSizeHigh << 32) | dwMaximumSizeLow;
  return ::CreateFileMappingFromApp(hFile, nullptr, flProtect, maxSize, nullptr);
#endif
#else
  assert(false && "Not implemented for Unix");
  return nullptr;
#endif
}

LPVOID MSFileSystemForDisk::MapViewOfFile(HANDLE hFileMappingObject,
                                          DWORD dwDesiredAccess,
                                          DWORD dwFileOffsetHigh,
                                          DWORD dwFileOffsetLow,
                                          SIZE_T dwNumberOfBytesToMap) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  return ::MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh,
                         dwFileOffsetLow, dwNumberOfBytesToMap);
#else
  ULONG64 offset = ((ULONG64)dwFileOffsetHigh << 32) | dwFileOffsetLow;
  return ::MapViewOfFileFromApp(hFileMappingObject, dwDesiredAccess, offset,
                                dwNumberOfBytesToMap);
#endif
#else
  assert(false && "Not implemented for Unix");
  return nullptr;
#endif
}

BOOL MSFileSystemForDisk::UnmapViewOfFile(LPCVOID lpBaseAddress) throw() {
#ifdef _WIN32
  return ::UnmapViewOfFile(lpBaseAddress);
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

bool MSFileSystemForDisk::FileDescriptorIsDisplayed(int fd) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  DWORD Mode; // Unused
  return (GetConsoleMode((HANDLE)_get_osfhandle(fd), &Mode) != 0);
#else
  return false;
#endif
#else
  assert(false && "Not implemented for Unix");
  return false;
#endif
}

unsigned MSFileSystemForDisk::GetColumnCount(DWORD nStdHandle) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  unsigned Columns = 0;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (::GetConsoleScreenBufferInfo(GetStdHandle(nStdHandle), &csbi))
    Columns = csbi.dwSize.X;
  return Columns;
#else
  return 0;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

unsigned MSFileSystemForDisk::GetConsoleOutputTextAttributes() throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (::GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    return csbi.wAttributes;
  return 0;
#else
  return 0;
#endif
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

void MSFileSystemForDisk::SetConsoleOutputTextAttributes(
    unsigned attributes) throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  ::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);
#else
  (void)attributes; // Console text attributes not supported in UWP.
#endif
#else
  assert(false && "Not implemented for Unix");
#endif
}

void MSFileSystemForDisk::ResetConsoleOutputTextAttributes() throw() {
#ifdef _WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  ::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            _defaultAttributes);
#else
  // Console text attributes not supported in UWP.
#endif
#else
  assert(false && "Not implemented for Unix");
#endif
}

int MSFileSystemForDisk::open_osfhandle(intptr_t osfhandle, int flags) throw() {
#ifdef _WIN32
  return ::_open_osfhandle(osfhandle, flags);
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

intptr_t MSFileSystemForDisk::get_osfhandle(int fd) throw() {
#ifdef _WIN32
  return ::_get_osfhandle(fd);
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

int MSFileSystemForDisk::close(int fd) throw() {
#ifdef _WIN32
  return ::_close(fd);
#else
  return ::close(fd);
#endif
}

long MSFileSystemForDisk::lseek(int fd, long offset, int origin) throw() {
#ifdef _WIN32
  return ::_lseek(fd, offset, origin);
#else
  return ::lseek(fd, offset, origin);
#endif
}

int MSFileSystemForDisk::setmode(int fd, int mode) throw() {
#ifdef _WIN32
  return ::_setmode(fd, mode);
#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

errno_t MSFileSystemForDisk::resize_file(LPCWSTR path, uint64_t size) throw() {
#ifdef _WIN32
  int fd = ::_wopen(path, O_BINARY | _O_RDWR, S_IWRITE);
  if (fd == -1)
    return errno;
#ifdef HAVE__CHSIZE_S
  errno_t error = ::_chsize_s(fd, size);
#else
  errno_t error = ::_chsize(fd, size);
#endif
  ::_close(fd);
  return error;

#else
  assert(false && "Not implemented for Unix");
  return 0;
#endif
}

int MSFileSystemForDisk::Read(int fd, void *buffer,
                              unsigned int count) throw() {
#ifdef _WIN32
  return ::_read(fd, buffer, count);
#else
  return ::read(fd, buffer, count);
#endif
}

int MSFileSystemForDisk::Write(int fd, const void *buffer,
                               unsigned int count) throw() {
#ifdef _WIN32
  return ::_write(fd, buffer, count);
#else
  return ::write(fd, buffer, count);
#endif
}

#ifndef _WIN32
int MSFileSystemForDisk::Open(const char *lpFileName, int flags,
                              mode_t mode) throw() {
  return ::open(lpFileName, flags, mode);
}

int MSFileSystemForDisk::Stat(const char *lpFileName,
                              struct stat *Status) throw() {
  return ::stat(lpFileName, Status);
}

int MSFileSystemForDisk::Fstat(int FD, struct stat *Status) throw() {
  return ::fstat(FD, Status);
}
#endif

} // end namespace fs
} // end namespace sys
} // end namespace llvm

///////////////////////////////////////////////////////////////////////////////////////////////////
// Externally visible functions.

HRESULT
CreateMSFileSystemForDisk(::llvm::sys::fs::MSFileSystem **pResult) throw() {
  *pResult = new (std::nothrow)::llvm::sys::fs::MSFileSystemForDisk();
  return (*pResult != nullptr) ? S_OK : E_OUTOFMEMORY;
}
