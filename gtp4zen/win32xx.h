#ifndef __GOCLIENT_WIN32XX_65043676432063352_H__
#define __GOCLIENT_WIN32XX_65043676432063352_H__

#include "library/wxx_shared_ptr.h"
#include "library/wxx_exception.h"
#include "library/wxx_textconv.h"

using namespace std;
using namespace Win32xx;

#define W2U(str)	CWtoA(str, CP_UTF8)
#define U2W(str)	CAtoW(str, CP_UTF8)

#endif
