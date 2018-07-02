#ifndef __ATPLIB_INC_
#define __ATPLIB_INC_

#define ATP_PARAM_TYPE_RANGE 1
#define ATP_PARAM_TYPE_ENUM 2

#define ATP_EXPLORATION_RANDOM 1
#define ATP_EXPLORATION_GENETIC 2

#define ATP_EXPLORATION_RANDOM_STR "random"
#define ATP_EXPLORATION_GENETIC_STR "genetic"


#define ATP_PHASE_START()                                           \
      call      atp_phase_start();

#define ATP_PHASE_END()                                             \
      call      atp_phase_start();

#define ATP_PARAM_ADD_VALUES(_pname, _vArray, _nValues, _domain)    \
      call atp_param_add_values_(_pname, _vArray, _nValues, _domain);

#define ATP_CONSTRAINT_DECLARE(_cname, _expr, _domain)             \
      call atp_constraint_declare_(_cname, _expr, _domain);

#define ATP_EXPLOITATION_DECLARE(_expList, _domain)                 \
      call atp_exploitation_declare_(_expList, _domain);

#define ATP_PARAM_GET(_pname, _address, _domain)                    \
      call atp_param_get_(_pname, _address, _domain);



#endif /* INCLUDE_RRL_USER_PARAMETERS_INC_ */

