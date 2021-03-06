// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include "stdint.h"
#include "string"

#include "Wincrypt.h"
#include "Md5.h"
#include "time.h"

#include "assert.h"
#include <mutex>
#include <random>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "boost/asio/io_service.hpp"  
#include "boost/asio/ip/address.hpp"  
#include "redisclient/redissyncclient.h"  
#include "redisclient/redisasyncclient.h" 
using namespace std;