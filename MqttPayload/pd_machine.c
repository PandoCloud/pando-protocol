#include <time.h>
#include <sys/time.h>


#include "pd_machine.h"

#ifdef ESP8266_PLANTFORM

uint64_t FUNCTION_ATTRIBUTE pd_get_timestamp()
{
	uint64_t time_now;
	time_now = pando_get_system_time();
	return time_now;
}

#else
uint64_t FUNCTION_ATTRIBUTE pd_get_timestamp()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return (now.tv_sec * 1000 + now.tv_usec/1000);
}

#endif
