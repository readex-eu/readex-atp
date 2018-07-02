/** 
* @file atpdesc.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This file contains the code to parse a JSON file 
*     with the structure defined for the ATP description file.
* 
* The file holds the code to parse the ATP description file.
* The JSON parser itself is in json.c file. The code in here uses, 
* the JSON parser to parse a file structure compliant with that of 
* the ATP description file. At the end of parsing, all the 
* data are populated in the data structures declared in 
* atpcommon.h.   
*
* @date 01/04/2017
**/

#ifndef __ATPDESC__
#define __ATPDESC__

#include "json.h"
#include "atpcommon.h"
#include "atpreport.h"




/** @brief a temporary data structure which holds the data parsed from
 *  the ATP description file. The final data structures are those
 *  located in atpcommon.h file
 */
typedef struct data_container_s{

  atp_domain_t *current_domain;
  atp_param_t *current_param;
  atp_constraint_t *current_constraint;

  char *pname;
  char *cname;
  int32_t *range;
  int32_t nvalues;
  int8_t *explorations;
  int32_t nexplorations;
  int32_t cursor;
  int8_t type;
  int32_t dflt;
  char *expr;

  int in_domain;
  int in_param;
  int in_range;
  int in_type;
  int in_default;
  int in_constraint;
  int in_exploration;

  int param_counter;
  int constraint_counter;
} data_container_t;

 
/** @brief loads the ATP description file content into the internal 
 *  data structures provided in the atpcommon.h file. In case of success
 *  the function returns 1 and in case of failure 0. 
 *  the assumed location of the file is the current folder (execution folder) 
 *  The path to ATP description file can be set by environment variable ATP_DESCRIPTION_FILE
 *  if it is not set ATP_description_file.json from cwd will be loaded. 
 *
 * @return 1 if the file is loaded successfully, 0 otherwise 
 */
int 
load_desc_file();


/**
 * @brief Launches the parsing of the ATP description file file.
 * It returns 1 in case of success and 0 in case of failure. 
 *
 * @param char *filename name of the file to parse 
 * @return 1 if the file was parsed successfully, 0 otherwise
 */
int 
parse_atp_desc(const char *filename);

/**
 * @brief Processes the next token of the parsed JSON file by 
 * determining its type and calling the appropriate handler 
 * function.
 *
 * @param data_container_t* data_buf a pointer to the temporary structure
 * which holds the extracted data.       
 * @param json_value* value pointer to the current token of the parsed file
 * @param int depth depth within the Json structure (helps to determine which
 * actions to conduct)
 * @return void
 */
void 
process_value(data_container_t *data_buf, json_value* value, int depth);




#endif
