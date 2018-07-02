#include "atplib.h"
#include "atpcommon.h"
#include "atpreport.h"
#include "rrl/user_parameters.h"  // RRL needs to be linked to the atplib at compile time
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "atpcommon.h"
#include "atpdesc.h"

extern void ATP_ERROR(const char *msg, ...);

//------------------------------------------------------------------------------

// new parameter declaration
void atp_param_declare(const char *pname, uint8_t ptype, int32_t deflt,
                       const char *domName) {
  atp_init_collection();
  if (!ATP_collection_is_active()) return;
  atp_domain_t *domain = ATP_domain_find(domName);
  if (domain == NULL) {
    domain = ATP_domain_new(domName);
    if (domain == NULL)
      ATP_ERROR("no enough memory for allocation\n");
  }
  // parameter name is considered unique and not tied to the domain scope
  // if this changes then ATP_param_find() should be used instead as it
  // limits uniqueness checking to the given domain
  atp_param_t *param = ATP_param_find_unique(pname);
  if (param != NULL) {
    ATP_ERROR(
        "parameter name %s already in use, This is considered looping ! ending "
        "collection\n",
        pname);
    atp_end_collection();
  } else {
    if (ptype != ATP_PARAM_TYPE_RANGE && ptype != ATP_PARAM_TYPE_ENUM) {
      ATP_WARNING("parameter type %d not in range\n", ptype);
    }
    param = ATP_param_new(pname, ptype, deflt, domain);
    rrl_atp_param_declare(pname, param->defaultVal, domName);
  }
}

//-----------------------------------------------------------------------------
// add values to parameter
void atp_param_add_values(const char *pname, int32_t *vArray, uint32_t nValues,
                          const char *domName) {
  atp_init_collection();
  if (!ATP_collection_is_active()) return;
  if (pname == NULL || vArray == NULL) {
    ATP_WARNING(
        "one or more parameters for function ATP_param_add_values \
                     is a NULL pointer\n");
    return;
  }
  if (nValues == 0) {
    ATP_WARNING("number of values supplied is zero\n");
    return;
  }
  atp_domain_t *domain = ATP_domain_find(domName);
  if (domain == NULL) {
    ATP_WARNING("domain %s does not exist, \n", domName);
    return;
  }
  atp_param_t *param = NULL;
  if ((param = ATP_param_find(pname, domain)) == NULL) {
    ATP_WARNING("parameter %s does not exist in domain %s\n", pname, domName);
    return;
  }
  uint8_t paramType = param->paramType;
  // In cases of type 'range' nValues should be equal to 3
  if (paramType == ATP_PARAM_TYPE_RANGE && nValues != 3) {
    ATP_WARNING(
        "number of values supplied in function ATP_param_add_values \
                     does not match parameter type\n");
    return;
  }

  ATP_param_add_values(param, vArray, nValues);
}

//-----------------------------------------------------------------------------
void atp_constraint_declare(const char *cname, const char *expr,
                            const char *domName) {
  atp_init_collection();
  if (!ATP_collection_is_active()) return;
  if (cname == NULL || expr == NULL ) {
    ATP_WARNING(
        "one or more parameters for function ATP_param_add_values \
                     is a NULL pointeri\n");
    return;
  }
  atp_domain_t *domain = ATP_domain_find(domName);
  if (domain == NULL) {
    ATP_WARNING("domain %s does not exist, \n", domName);
    return;
  }
  atp_constraint_t *constraint = ATP_constraint_find(cname, domain);
  if (constraint != NULL) {
    ATP_ERROR(
        "constraint name %s already in use, This is considered looping, "
        "stoping collection\n",
        cname);
    atp_end_collection();
  } else {
    ATP_constraint_new(cname, expr, domain);
  }
}

//-----------------------------------------------------------------------------
void atp_exploration_declare(const char *expList, uint32_t nValues,
                             const char *domName) {
  atp_init_collection();
  if (!ATP_collection_is_active()) return;
  if (expList == NULL || domName == NULL) {
    ATP_WARNING(
        "one or more parameters for function ATP_EXPLORATION_DECLARE \
                     is a NULL pointeri\n");
    return;
  }
  atp_domain_t *domain = ATP_domain_find(domName);
  if (domain == NULL) {
    ATP_WARNING("domain %s does not exist, \n", domName);
    return;
  }

  ATP_exploration_new(expList, nValues, domain);
}

//-----------------------------------------------------------------------------
void atp_param_get(const char *pname, void *address, const char *domName) {

  int i;
  atp_param_t *param = ATP_param_find_unique(pname);
   
  if (param == NULL) {
      ATP_ERROR(
          "Please call atp_param_declare to declare the parameter before "
          "atp_param_get \n");
  } else {
      rrl_atp_param_get(pname, param->defaultVal, address, domName);
  }
}

//-----------------------------------------------------------------------------

void atp_init_collection() {

  if(ATP_check_exec_mode() == ATP_EXEC_RAT)
  {
     if(!load_desc_file())
         ATP_ERROR("ATP library Could not load the ATP description file\n");      
     ATP_collection_unset();
     ATP_TRACE("ATP is working in RAT mode\n"); 
  }else{ 
      ATP_init(); 
      if (ATP_collection_is_not_init()) {
         ATP_collection_set();
         ATP_MESSAGE("Starting ATP collection\n");
      }
     ATP_TRACE("ATP is working in DTA mode\n"); 
  }
}
//-----------------------------------------------------------------------------
void atp_end_collection() {
  if (ATP_collection_is_active()) {
    ATP_collection_unset();
    ATP_gen_descfile();
    ATP_MESSAGE("Ending ATP collection\n");
  }
}
