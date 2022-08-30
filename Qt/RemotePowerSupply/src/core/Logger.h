#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <QPlainTextEdit>

#include <spdlog/spdlog.h>
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types

/* Logger instance - multi-threaded */
extern spdlog::logger *logger;

/* Logger viewer inside application */
extern QPlainTextEdit *loggerContentGui;

void setup_logger();

#endif // _LOGGER_H_