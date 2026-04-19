#pragma once

#include <stdio.h>

#ifndef NDEBUG
#define LOG_INFO(msg, ...) printf("[INFO]  " msg "\n", ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) printf("[ERROR]  " msg "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(msg, ...) ((void)0)
#define LOG_ERROR(msg, ...) ((void)0)
#endif