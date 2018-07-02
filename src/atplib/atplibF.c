
#include "atplib.h"
#include "atpreport.h"


void atp_param_declare__(const char *_pname, 
                       uint8_t *_ptype,  
                       int32_t *_deflt,
                       const char *_dname,
                       int pname_len,
                       int dname_len) 
{ 
    char *p_name = (char *)malloc(pname_len+1);
    snprintf(p_name, pname_len+1, "%s",_pname);   
    char *d_name = (char *)malloc(dname_len +1); 
    snprintf(d_name, dname_len+1, "%s",_dname);   
    atp_param_declare(p_name, *_ptype, *_deflt, d_name); 
    free(p_name);
    free(d_name);
}

void atp_param_add_values__(const char *_pname, 
                           int32_t *_vArray, 
                           uint32_t *_nValues,
                           const char *_dname, 
                           int pname_len,
                           int dname_len) 
{
    char *p_name = (char *)malloc(pname_len+1);
    snprintf(p_name, pname_len+1, "%s",_pname);   
    char *d_name = (char *)malloc(dname_len +1); 
    snprintf(d_name, dname_len+1, "%s",_dname);   
    atp_param_add_values(p_name, _vArray, *_nValues, d_name); 
    free(p_name);
    free(d_name);

}

void atp_constraint_declare__(const char *_cname, 
                              const char *_expr, 
                              const char *_dname, 
                              int cname_len, 
                              int expr_len, 
                              int dname_len) 
{ 
    char *c_name = (char *)malloc(cname_len+1);
    snprintf(c_name, cname_len+1, "%s",_cname);   
    char *expr = (char *)malloc(expr_len+1);
    snprintf(expr, expr_len+1, "%s",_expr);   
    char *d_name = (char *)malloc(dname_len +1); 
    snprintf(d_name, dname_len+1, "%s",_dname);   
    atp_constraint_declare(c_name, expr , d_name); 
    free(c_name);
    free(expr);
    free(d_name);
}
 
void atp_exploration_declare__(const char *_expList, uint32_t *_nValues, const char *_dname, int dname_len) 
{

    char *d_name = (char *)malloc(dname_len +1); 
    snprintf(d_name, dname_len+1, "%s",_dname);   
    atp_exploration_declare(_expList, *_nValues, d_name); 
    free(d_name);
}

void atp_param_get__(const char *_pname, 
                     void *_address, 
                     const char *_dname, 
                     int pname_len, 
                     int dname_len)
{

    char *p_name = (char *)malloc(pname_len+1);
    snprintf(p_name, pname_len+1, "%s",_pname);   
    char *d_name = (char *)malloc(dname_len +1); 
    snprintf(d_name, dname_len+1, "%s",_dname);   
    atp_param_get(p_name, _address, d_name); 
    free(p_name);
    free(d_name);
}



