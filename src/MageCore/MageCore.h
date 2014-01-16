#pragma once

// STD C Headers
#include "std_headers.h"

// STD C++ Headers
#include "stl_headers.h"

#ifdef USE_MEMORY_MANAGER
#	define new new( __FILE__, __LINE__ )
#endif

// Typedefs for core types
#include "MageTypes.h"
#include "Color.h"

// Debug
#include "DebugIO.h"
#include "Assertion.h"

// Memory
#include "MageMemory.h"

// IO
#include "Console.h"

#include "Dictionary.h"
#include "CircularBuffer.h"
#include "Event.h"
#include "Clock.h"
#include "ProfilingSystem.h"

// Utility
#include "BitHacks.h"
#include "HashUtil.h"
#include "StringUtil.h"
#include "FileSystem.h"
#include "XmlReader.h"
#include "XmlWriter.h"
#include "CommandArgs.h"

// Threads
#include "Mutex.h"
#include "Thread.h"
#include "Job.h"
#include "JobManager.h"