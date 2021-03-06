// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "node.h"

#ifdef _WIN32
int wmain(int argc, wchar_t *wargv[]) {
  // Convert argv to to UTF8
  char** argv = new char*[argc];
  for (int i = 0; i < argc; i++) {
    // Compute the size of the required buffer
    DWORD size = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     wargv[i],
                                     -1,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL);
    if (size == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
    // Do the actual conversion
    argv[i] = new char[size];
    DWORD result = WideCharToMultiByte(CP_UTF8,
                                       0,
                                       wargv[i],
                                       -1,
                                       argv[i],
                                       size,
                                       NULL,
                                       NULL);
    if (result == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
  }
  // Now that conversion is done, we can finally start.
  return node::Start(argc, argv);
}
#elif __ebbrt__
#include <ebbrt/native/Acpi.h>
#include <ebbrt/native/Debug.h>
#include <ebbrt/native/StaticIds.h>

#include <ebbrt-filesystem/FileSystem.h>
ebbrt::EbbRef<FileSystem> node_fs_ebb;

#ifndef EBBRT_NATIVE_ONLY
#include <ebbrt-cmdline/CmdLineArgs.h>

enum : ebbrt::EbbId {
  kCmdLineArgsId = ebbrt::kFirstStaticUserId,
  kFileSystemId
};
#endif

void AppMain() {
    putenv(const_cast<char *>("TZ=EST5EDT4,M3.2.0,M11.1.0"));
#ifndef EBBRT_NATIVE_ONLY
    node_fs_ebb =
        FileSystem::Create(&FileSystem::CreateRep(kFileSystemId), kFileSystemId);
    auto cmdlineargs = ebbrt::EbbRef<CmdLineArgs>(kCmdLineArgsId);
    auto cmd_argc = cmdlineargs->argc();
    auto cmd_argv = cmdlineargs->argv();
    auto i = node::Start(cmd_argc, cmd_argv);
#else
    int argc = 0;
#ifndef __JA_V8_PROFILE_HACK__
    const char *argv[] = { "node" };
    argc += 1;
#else
    // "--trace" full trace of execution  produces a lot of info
    const char *argv[] = { "node", "--logfile", "-", "--log_code" };
    argc += 4;
#endif
    auto i =node::Start(argc, const_cast<char **>(argv));

#endif
    ebbrt::kprintf("Return Code: %d\n", i);
    ebbrt::acpi::PowerOff();
}
#else
// UNIX
int main(int argc, char *argv[]) {
  return node::Start(argc, argv);
}
#endif
