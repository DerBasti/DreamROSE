#pragma once

#ifndef __ROSE_DEFS__
#define __ROSE_DEFS__

//Defines whether or not everything is read from the VFS structure
#define __ROSE_USE_VFS__

//Defines whether or not we want debug messages sent to the player [e.g. ATTACK POSSIBLE! DESTINATION REACHED!]
//Also: adds variables which store results before returning them which enables easier value-return debugging
#define __ROSE_DEBUG__

#define __ROSE_MULTI_THREADED__

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t;
typedef unsigned long long qword_t;

#endif //__ROSE_DEFS__

#define NOMINMAX