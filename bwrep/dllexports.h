#ifndef __DLLEXPORTS_H
#define __DLLEXPORTS_H

#ifdef BWREP_EXPORTS
#define DllExport __declspec( dllexport )
#elif IS_EMBEDDED
#define DllExport
#else
#define DllExport __declspec( dllimport )
#endif

#endif
