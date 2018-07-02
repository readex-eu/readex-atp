
/** 
* @file atpquery.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This file contains code which queries the Omega calculator,
* a constraint solver. The querying is not direct but goes through 
* LUA scripts.   
* 
*     
*
* @date 01/04/2017
**/

#ifndef __ATP_QUERY__
#define __ATP_QUERY__


#include "atpdesc.h"

/** @brief enumeration of the possible types of status
 * the query can be in
 */ 
typedef enum query_status_s{

  aq_domain_not_found, 
  aq_range_success,
  aq_enum_success,
  aq_json_error, //
  aq_lua_error,  //
  aq_oc_error    //
} query_status_t;


/**
 * @brief data structure which holds an OC query string and results.
 * The query is recorded as a string 
 * In case of a range generation query the results are tuples of points
 * which is a 2D array organized into a 1D array. The array limits are
 * numCombinations for the number of rows and combinationNumElems for the
 * number of columns.
 * In case of a valid point or not query, the result is a boolean field:
 * valid_point which is set to 1 in case the point is valid and to 0 in
 * case the point is not.
 * A status field holds the status of the request after it is submited
 */
typedef struct atp_query_s {    

  char *query_string; 
  int32_t *pointCombinations;
  int32_t numCombinations;
  int32_t combinationNumElems;

  int8_t valid_point; 
  query_status_t status;

} atp_query_t;


/**
 * @brief Launches the LUA script involved in querying the Omega Calculator
 *   
 *
 * @param atp_query_t *request the data structure holding the request string and which
 *                             will hold the results of the request    
 * @return void
 */

void oc_request_range_generation(atp_query_t *request);


#endif
