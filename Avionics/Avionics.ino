#include <Arduino.h>
/***
 *              _____        _   _ _______ _    _ ______ _____
 *             |  __ \ /\   | \ | |__   __| |  | |  ____|  __ \     /\
 *             | |__) /  \  |  \| |  | |  | |__| | |__  | |__) |   /  \
 *             |  ___/ /\ \ | . ` |  | |  |  __  |  __| |  _  /   / /\ \
 *             | |  / ____ \| |\  |  | |  | |  | | |____| | \ \  / ____ \
 *      ______ |_| /_/____\_\_|_\_|__|_|__|_|__|_|______|_|  \_\/_/_ __\_\  _____
 *     |  ____| |    |  ____/ ____|__   __|  __ \ / __ \| \ | |_   _/ ____|/ ____|
 *     | |__  | |    | |__ | |       | |  | |__) | |  | |  \| | | || |    | (___
 *     |  __| | |    |  __|| |       | |  |  _  /| |  | | . ` | | || |     \___ \
 *     | |____| |____| |___| |____   | |  | | \ \| |__| | |\  |_| || |____ ____) |
 *     |______|______|______\_____|  |_|  |_|  \_\\____/|_| \_|_____\_____|_____/
 *
 */

/***
 * This file has a set of defines that will conditionally compile seperate files
 * If no defines are set the final code will be excuted
 * Otherwise tests for components of the PCB will be enabled
 */

// #define GPS_TEST
// #define GPS_RAW_TEST
// #define MPU_TEST
// #define BMP_TEST
// #define LORA_TEST

#if defined GPS_TEST
#pragma message("GPS")
#include "Tests/TEST_GPS.cpp"
#elif defined GPS_RAW_TEST
#pragma message("GPS RAW")
#include "Tests/TEST_GPS_RAW.cpp"
#elif defined MPU_TEST
#pragma message("MPU")
#include "Tests/TEST_MPU.cpp"
#elif defined BMP_TEST
#pragma message("BMP")
#include "Tests/TEST_BMP.cpp"
#elif defined LORA_TEST
#pragma message("LORA")
#include "Tests/TEST_LORA.cpp"
#else
#pragma message("Main")
#include "source/Avionics/Avionics.cpp"
#endif
