/** 
* @file atpreport.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This file contains the code that handle reporting for the 
* ATP component 
* 
* Reporting is divided into three categories:
*   - ATP messages and warnings preinted respectively into stdout and 
*   stderr
*   - ATP error messages reported in the atp.log file
*   - ATP tracing messages which can be activated if the code is built in
*   Debug mode.
*
* @date 01/04/2017
**/
#ifndef __ATPREPORT__
#define __ATPREPORT__

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>


#define STD_ERROR "unknown error"
#define STD_MESSAGE "unknown message"
#define ATP_REPORT_FILE "atp.log"

/** brief prints a message to stdout */
#define ATP_MESSAGE(format, ...) \
       fprintf(stdout,"ATP MESSAGE: "format, ##__VA_ARGS__); fflush(stdout);
   
/** brief prints a warning message to stderr */
#define ATP_WARNING(format, ...) \
       fprintf(stderr,"ATP WARNING: "format, ##__VA_ARGS__);


/**
 * @brief Records an error message in the atp.log file. If the file cannot  
 * be written the message is logged in stderr.
 *
 * @param const char* msg the message to write 
 * @return void
 */

void ATP_ERROR(const char *msg, ...);



/** brief prints a debug message to stderr */
#ifdef DEBUG
    #define ATP_TRACE(format, args...) fprintf(stderr, "ATP DEBUG: " format, \
     ##args)
#else
    #define ATP_TRACE(format, args...) do {} while (0)
#endif

/** brief prints a message to stderr */
#ifdef DEBUG
    #define ATP_PRINT(format, args...) fprintf(stderr, format, \
     ##args)
#else
    #define ATP_PRINT(format, args...) do {} while (0)
#endif



/** brief prints a detailed debug message to stderr (with line number, 
 * function name and file name details)*/
#ifdef DEBUG
    #define ATP_VTRACE(format, args...) fprintf(stderr, "ATP DEBUG: %s:%d:%s(): " format, \
    __FILE__, __LINE__, __func__, ##args)
#else
    #define ATP_VTRACE(format, args...) do {} while (0)
#endif



#endif 
