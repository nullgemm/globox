#ifndef H_GLOBOX_ERROR
#define H_GLOBOX_ERROR

#include "include/globox.h"

#ifdef GLOBOX_ERROR_LOG_DEBUG
#define globox_error_throw(globox, new_code) \
	globox_error_throw_extra(\
		globox,\
		new_code,\
		GLOBOX_ERROR_FILE,\
		GLOBOX_ERROR_LINE)
#define GLOBOX_ERROR_FILE __FILE__
#define GLOBOX_ERROR_LINE __LINE__

void globox_error_throw_extra(
	struct globox* globox,
	enum globox_error new_code,
	const char* file,
	unsigned int line);
#else
void globox_error_throw(
	struct globox* globox,
	enum globox_error new_code);
#endif

void globox_error_init(struct globox* globox);

#endif
