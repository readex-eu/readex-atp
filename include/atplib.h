/** 
* @file atplib.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This header file will contain all required API functions for code 
*      annotation with the ATP library.    
* 
* This file holds the public API of the ATP library (part of ATP compoenent).
* The API provides functions to annotate the application code with. THe API
* enable to both declare application tuning parameters and get values 
* from a tuning system.
*
* Example code of API usage:
* @code
*  #include "atplib.h"
*
*
*  function foo(){
*  int atp_cv;
*
*  ...
*  ATP_PARAM_DECLARE("solver", ATP_PARAM_TYPE_RANGE, 1, "DOM1");
*  var solver_values[3] = {1,5,1};
*  ATP_ADD_VALUES("solver", solver_values, 3, "DOM1");
*  ATP_PARAM_GET("solver", &atp_cv, "DOM1");
*
*  switch (atp_cv){{
*    case 1:
*        // choose algorithm 1
*        break;
*    case 2:
*        // choose algorithm 2
*        break;
*    ...
*  }
*}
*
*function bar(){
*  int atp_ms;
*
*  ...
*  ATP_PARAM_DECLARE("mesh", ATP_PARAM_TYPE_RANGE, 40, "DOM1");
*  var mesh_values[3] = {1,80,10};
*  ATP_ADD_VALUES("mesh", mesh_values, 3, "DOM1");
*  ATP_CONSTRAINT_DECLARE("const1", "(solver = 1 && 1 <= mesh <= 40)||
*                                    (solver = 2 && 40 <= mesh <= 80)||
*                                    (solver > 2 && mesh = 120)", "DOM1");
*  ATP_PARAM_GET("mesh", &atp_ms, "DOM1");
*
*  if(atp_ms > 1 && atp_ms <= 40)
*  {
*      ...
*  }
*  if(atp_ms > 40 && atp_ms <= 80)
*  {
*      ...
*  }
*  if(atp_ms == 120)
*  {
*      ...
*  }
*  ...
*
*}
* 
* @endcode    
*
* Usage and functionning at DTA:
*   - In order to use the library in DTA mode the following environment viriable needs to be set:
*      export ATP_EXECUTION_MODE=DTA     
*   - The ATP_PARAM_DECLARE, ATP_PARAM_ADD_VALUES and ATP_CONSTRAINT_DECLARE are executed only once
*     during the first phase of the application to collect parameter details. The first function met
*     a second time triggers the end of collection and the generation of the ATP description file in json format. 
*     The description file name and location can be specified by ATP_DESCRIPTION_FILE 
*     environment varialble, if the variable is not set ATP_description_file.json will
*     be created in the same location as the application binary.  
*   - The ATP_PARAM_GET function assigns the values decided by the RRL, In cases where no value is  
*     available to the RRL, it assigns the default value. This happens automatically in the first phase 
*     of the application where parameter collection is still active.
*
* Usage and functioning during RAT:
*   - In order to use the library in RAT mode the ATP_EXECUTION_MODE environment variable previously set 
*     should be unset
*   - In RAT mode, the declaration functions are shutdown, they are called but return without doing any work.
*     The details on parameters are directly loaded from the ATP description file. Only the 
*     ATP_PARAM_GET function continues to work exactly the same way as in DTA.  
*
*
* @date 01/04/2017
**/



#ifndef __ATPLIB__
#define __ATPLIB__

#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @brief defines a range type for the values supplied for a declared parameter   */
#define ATP_PARAM_TYPE_RANGE 1

/** @brief defines an enumeration type for the values supplied for a declared parameter   */
#define ATP_PARAM_TYPE_ENUM 2

/** @brief ATP exploration of type RANDOM hint */
#define ATP_EXPLORATION_RANDOM 1
/** @brief ATP exploration of type GENETIC hint */
#define ATP_EXPLORATION_GENETIC 2


#define ATP_EXPLORATION_RANDOM_STR "random"
#define ATP_EXPLORATION_GENETIC_STR "genetic"




uint8_t ATP_INIT();

void ATP_ABORT();

void atp_init_collection();

  /** @brief ends the parameter collection and saves the ATP description file.
   *  
   *  Triggered automatically when ATP_PARAM_DECLARE sees the same parameter for the second
   *  time. Can be also invoked manually with ATP_END_COLLECTION()
   */
void atp_end_collection();


//------------- PUBLIC API -----------------------------------------------------------------

/** @brief declares a new parameter to the tuning system. It also declares
 *  a new domain in case it  did not exist already  
 *
 *  declares a new parameter with the unique name "pname" and a type "ptype",
 *  a default value for the parameter needs also to be supplied "dlt_val".
 *  Also the domain the parameter belongs to can be supplied "domain". If the
 *  domain was not already present it is created. If no domain is supplied the
 *  parameter is assigned to the "default" domain.    
 * 
 * @param char* pname parameter name
 * @param uint8_t ptype parameter type. Can be one of ATP_PARAM_TYPE_RANGE or
 *        ATP_PARAM_TYPE_ENUM
 * @param int32_t dlt_val default value of the parameter   
 * @param cons char* domain the domain the parameter belongs to
 * @return NULL 
 */
void atp_param_declare(const char *pname, uint8_t ptype, int32_t dlt_val, const char *domain);



/** @brief declares the possible values for a given parameter 
 *
 * Declares the possible values a parameter "pname" (previously declared) can
 * have. The values can be of type 'range' in which case the vArray will only
 * have three fields with the following structure {min value, max value, increment},
 * or of type 'enum' in which case all the possible values are supplied within
 * vArray. In both cases nValues holds the number of fields in vArray. Also, if
 * the name of the domain the parameter belongs too should be supplied (except
 * if the parameter belongs to the "default" domain)     
 *  
 * @param char* pname parameter name
 * @param int32_t* vArray parameter type. Can be one of ATP_PARAM_TYPE_RANGE or
 * @param int32_t nValues default value of the parameter   
 * @param const char* domain the domain the parameter belongs to
 * @return NULL 
 */
void atp_param_add_values(const char *pname, int32_t *vArray, uint32_t nValues,
                          const char *domain);

/** @brief declares a new constraint to the tuning system.  
 *
 *  declares a new constraint with the unique identified name "cname". the 
 *  constraint itself is expressed in the form of a character string "expr".
 *  A constraint involves only parameters of the same domain, That is only
 *  one "domain". 
 * 
 * @param char* cname cosntraint name
 * @param char *expr  logical expression of the constraint
 * @param const char* domain the domain the constraint is tied to
 * @return NULL 
 */
void atp_constraint_declare(const char *cname, const char *expr, const char *domain);

/** @brief declares a new exploration list of hints to the tuning system
 *
 * Provides parameter values exploration strategy hints to the tuning system. 
 * Pre-recorded exploration hints include "ATP_EXPLORATION_RANDOM" and     
 * "ATP_EXPLORATION_GENETIC". The function enables to supply an ordered list
 * of hints "expList" (nValues being just the number of field the arrya constains),
 * and also the "domain" they apply to. 
 * It is to be noted that these are only hints about how to explore parameter values,
 * the tuning system may choose to follow them or not.  
 *
 * @param const char* expList list of explorations  
 * @param uint32_t  nValues  logical expression of the constraint
 * @param const char* domain the domain the constraint is tied to
 * @return NULL 
 */
void atp_exploration_declare(const char *expList, uint32_t nValues, const char *domain);

/** @brief Assigns a value from the tuning system to a parameter. In case no value 
 *  is available to the tuning system, it can always assign the default values recorded
 *  in the atp_param_declare() function.
 *   
 * 
 * @param char*       pname parameter name
 * @param void*       address of the control variable  
 * @param const char* domain the domain the parameter belongs to
 * @return NULL 
 */
void atp_param_get(const char *pname, void *address, const char *domain);

//-----------------------------------------------------------------------------


#define ATP_PARAM_DECLARE(_pname, _ptype, _dlt_val, _domain)       \
       atp_param_declare(_pname, _ptype, _dlt_val, _domain);

#define ATP_PARAM_ADD_VALUES(_pname, _vArray, _nValues, _domain)    \
       atp_param_add_values(_pname, _vArray, _nValues, _domain);

#define ATP_CONSTRAINT_DECLARE(_cname, _expr, _domain)             \
       atp_constraint_declare(_cname, _expr, _domain);

#define ATP_EXPLOITATION_DECLARE(_expList, _nvalues, _domain)                 \
       atp_exploitation_declare(_expList, _nvalues ,_domain);

#define ATP_PARAM_GET(_pname, _address, _domain)                    \
       atp_param_get(_pname, _address, _domain);

#define ATP_END_COLLECTION()                                        \
       atp_end_collection();

#ifdef __cplusplus
}
#endif


#endif
