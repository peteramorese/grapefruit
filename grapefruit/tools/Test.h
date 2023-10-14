
#include "tools/Logging.h"

#define TEST_ASSERT(condition, msg) {if (!(condition)) {ERROR("[Test failure] " << msg);}}
#define TEST_ASSERT_FATAL(condition, msg) {if (!(condition)) {ERROR("[Fatal test failure] " << msg); exit(1);}}