#ifndef GSSASERT_H
#define GSSASERT_H
#include <iostream>
#include <assert.h>

#define _Assert(a,de) \
	if(a==false) \
	{ \
		std::cout << "Assert:" << de << std::endl; \
		assert(a); \
	};
#endif