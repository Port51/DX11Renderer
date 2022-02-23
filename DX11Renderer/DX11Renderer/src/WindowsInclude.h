#pragma once

// target Windows 7 or later
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
/*#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE*/

#ifndef STRICT
#define STRICT
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

/*#ifndef CP_UTF8
#define CP_UTF8 65001
#endif*/

#include <Windows.h>
//#include <stringapiset.h> // for CP_UTF8, WideCharToMultiByte(), etc.

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
//#if defined(CreateWindow)
//#undef CreateWindow
//#endif