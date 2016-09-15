#pragma once

#include "resource.h"
#include <string>

#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif
