#pragma once

#define ID_BUTTON_START_TEST 0x8001
#define ID_BUTTON_START_END  0x8002

#define LOGGER_SIZE 100000
#include "resource.h"

typedef struct LoggerCell_t_
{
	int  visited;
	char LogLine[128];
} LoggerCell_t, *pLoggerCell_t;

