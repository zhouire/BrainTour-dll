// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define WIN32_LEAN_AND_MEAN				// Exclude rarely-used stuff from Windows headers
#include <winsock2.h>
#include <Windows.h>

#include "targetver.h"


//boost files
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/set.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
//boost files over


#include <stdio.h>
#include <tchar.h>
#include <set>

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/freeglut.h>

#include "MathHelpers.h"

// TODO: reference additional headers your program requires here
