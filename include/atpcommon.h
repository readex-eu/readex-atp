/** 
* @file atpcommon.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This file contains the code for the base data structures which holds
* the details on the parameters, domains, constraints and explorations.
*
* Holds the declaration of base data structures to store ATP details. The used
* data structures are:
*   - atp_param_t
*   - atp_constraint_t 
*   - atp_domain_t    
*   - atp_desc_t
*
* Functions to handle and query the data sturctures are also implemented
*
* @date 01/04/2017
**/

#ifndef __ATPCOMMON__
#define __ATPCOMMON__


       
#define DOMAIN_MARGIN 10
#define PARAMS_MARGIN 10
#define CONSTRAINTS_MARGIN 10
#define EXPLORATIONS_MARGIN 10
  
#define ATP_CHECK_DOMAIN 1
#define ATP_CHECK_PARAM_NAME 2
#define ATP_CHECK_PARAM_TYPE 3

//@brief name of the default domain (used if no domain is specified)
#define ATP_DFLT_DOMAIN "default"

#define ATP_EXEC_DTA 1
#define ATP_EXEC_RAT 2

// @brief data structure holding details about parameters:
// name, type, default value, and values array
typedef struct atp_param_desc_s{

  const char *paramName; 
  uint8_t paramType;
  int32_t defaultVal; 
  int32_t *valueArray;
  uint32_t nValues;
   
} atp_param_t;

//@brief data structure holding details about constraints:
// name and expression
typedef struct atp_constraint_s{

  char *constName;
  char *constExpr;

} atp_constraint_t;

//@brief data sturcture holding details about domains:
// domain name, arrays of the parameters, constraints and
// explorations contained in it
typedef struct atp_domain_s{

 const char *domName;

 atp_param_t *paramsArray;
 uint32_t numParams;
 uint32_t maxNumParams;

 atp_constraint_t *constraintsArray;
 uint32_t numConstraints;
 uint32_t maxNumConstraints;

 uint8_t *explorationArray;
 uint32_t numExplorations;
 uint32_t maxNumExplorations;  

} atp_domain_t;

//@brief data structure holding the list of domains declared  
typedef struct atp_desc_s {

  atp_domain_t *domains;
  uint32_t numDomains; 
  uint32_t maxNumDomains; 
     
  int8_t collection;  

} atp_desc_t;


//Internal API functions

int ATP_create_lockfile();

void ATP_remove_lockfile();

uint8_t ATP_init();

int ATP_collection_is_active();

int ATP_collection_is_not_init();

void ATP_collection_set();

void ATP_collection_unset();

int ATP_check_exec_mode();


//@brief attampts to find a domain and return it if found
atp_domain_t *ATP_domain_find(const char *domName);

//@brief attempts to find a parameter within a given domain and returns it if found   
atp_param_t *ATP_param_find(const char *pname, atp_domain_t *domain);

//@brief attempts to find a parameter accross all domains (READEX considers currently
//that a parameter name should be unique accross domains, hence this function is
//used instead of the ATP_param_find functions  
atp_param_t *ATP_param_find_unique(const char *pname);

//@brief attempts to find a constraint inside a given domain and return it if found
atp_constraint_t *ATP_constraint_find(const char *cname, atp_domain_t *domain);

//@brief creates a new domain if not already present
void *ATP_domain_new(const char *domName);

//@brief creates a new parameter if not present yet  
void *ATP_param_new(const char *pname, uint8_t ptype, int32_t deflt,
                    atp_domain_t *domain);

//@brief creates a new constraint if not present yet
void *ATP_constraint_new(const char *cname, const char *expr, 
                         atp_domain_t *domain);

//@brief adds values range to a specific parameter
void *ATP_param_add_values(atp_param_t *param, int32_t *vArray,uint32_t nValues);

//@brief creates a new epxloration
void ATP_exploration_new(const char *expList, int nValues, atp_domain_t *domain);

//@brief generate the ATP description fil from the ATP internal data structures.
//The file name and location can be specified by ATP_DESCRIPTION_FILE environment variable,
//if variable is not set ATP_description_file.json will be created in current working
//directory
void ATP_gen_descfile();

//@brief returns the total number of parameters
int32_t ATP_get_total_parameter_number();

//@brief return the total number of values for the parameters (aggregated for all
//parameters)
int32_t ATP_get_total_parameter_values_number();

//@brief returns an array of ints with the size of number of domains. Each int contains
//the number of parameters in each domain
int32_t *ATP_get_total_domain_details();

//@brief returns an array of strings each being 100 characters long. each string 
//constains a parameter name
char *ATP_get_total_parameter_names();

//@brief returns an array of string each being 100 charactes long. each string
//contains a domain name
char *ATP_get_total_domain_names();

//@brief returns details on parameters to respons to PTF request (see response format for "getatpspecs" request   
int32_t *ATP_get_total_parameter_details();

//@brief builds an OC request. The request corresponds to the generation of a valid 
//set of values (to be submitted as a parameter to the codegen lua function)         
char *ATP_build_oc_request(const char *domName);

//@brief returns an array containing the domain names for all domains   
char **ATP_get_domain_names();

//@brief returns the number of domains declared 
int ATP_get_num_domains();
#endif
