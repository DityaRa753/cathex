#include "string.h"

/* sign of success greater than zero */

int compare(const char *str1, const char *str2)
{
	if(*str1 == *str2) {
		if(*str1 == '\0' && *str2 == '\0') return 1;
		return compare(++str1, ++str2);
	}
	else {
		return 0;
	}
}
