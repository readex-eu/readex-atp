
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <sys/file.h>
#include "atplib.h"
#include "atpcommon.h"
#include "atpreport.h"

static atp_desc_t domArray;

/** Create a lockfile atp.lck from 1 process/thread.
 *  Only one process/thread creates the lock file, the process that creates it
 *  writes the ATP description file
 */
int ATP_create_lockfile() {
  int lockFd;
  //create and open file atp.lck, if the file already exists return error code
  if ((lockFd = open("atp.lck", O_CREAT | O_EXCL, 0666)) < 0 )
    return -1;

  close(lockFd);
  return 1;
}

/** Remove the lockfile atp.lck.
 *  Deletes the atp.lck file, called after the end of atp collection phase, when
 *  description file is writen to allow description file to be written in the next
 *  application execution. Called also in the atplib init, to make sure the atp
 *  description file will be written in case the last execution of ATP did not exit
 *  properly.
 */
void ATP_remove_lockfile() {
  remove("atp.lck");
}


uint8_t ATP_init()
{
    static uint8_t atp_is_initialized = 0;

    if(atp_is_initialized)
        return (1);
    domArray.domains = malloc(sizeof(atp_domain_t) * DOMAIN_MARGIN);
    if(domArray.domains == NULL){
        ATP_ERROR("no enough memory for allocation");  
        return (0);
    }      
    domArray.maxNumDomains = DOMAIN_MARGIN;
    domArray.numDomains = 0; 
    //create a "defualt" domain
    ATP_domain_new(ATP_DFLT_DOMAIN);
    domArray.collection = -1;
    atp_is_initialized = 1;

    ATP_remove_lockfile();

    return (1);

}

//-----------------------------------------------------------------------------
int ATP_collection_is_active()
{
  return  (domArray.collection == 1)? 1:0;
}

int ATP_collection_is_not_init()
{
  return (domArray.collection == -1) ? 1:0;
}

void ATP_collection_set()
{
  domArray.collection = 1;
}

void ATP_collection_unset()
{
  domArray.collection = 0;
}

int ATP_check_exec_mode()
{
    const char *atp_exec_mode = getenv("ATP_EXECUTION_MODE");
    if (atp_exec_mode == NULL)
        return ATP_EXEC_RAT; 

    if(strcmp(atp_exec_mode,"DTA") == 0)
    {
        return ATP_EXEC_DTA;
    }else{
        return ATP_EXEC_RAT;
    }     
}

//-----------------------------------------------------------------------------
atp_domain_t *ATP_domain_find(const char *domName)
{

    int i;
    atp_domain_t *domain = NULL;
    if(domName == NULL)
    {
       domain = ATP_domain_find(ATP_DFLT_DOMAIN);
       return domain; 
    }
    ATP_TRACE("finding domain %s among %d domains\n", domName, domArray.numDomains); 
    for (i=0; i<domArray.numDomains; i++)
    { 
        atp_domain_t *domTemp = &(domArray.domains[i]); 
        if (strcmp(domTemp->domName, domName) == 0)
        {
            ATP_TRACE(">> domain match found\n");
            domain = domTemp;  
            break;
        }
        ATP_TRACE(">> domain not matched with '%s'\n", domTemp->domName);
    }
    return domain;
}

//-----------------------------------------------------------------------------
atp_param_t *ATP_param_find(const char *pname, atp_domain_t *domain)
{

    int i;
    atp_param_t *param = NULL;
    for(i=0; i< domain->numParams; i++)
    {
        atp_param_t *paramTemp = &(domain->paramsArray[i]);
        if (strcmp(paramTemp->paramName, pname) == 0)
        {  
            param = paramTemp;
            break; 
        }
    }
    return param;
}
//-----------------------------------------------------------------------------

atp_param_t *ATP_param_find_unique(const char *pname)
{

    int i;
    atp_domain_t *domain = NULL; 
    atp_param_t  *param = NULL;
    for (i=0; i < domArray.numDomains; i++)
    {

        domain = &(domArray.domains[i]); 
        int j;
        for(j=0; j < domain->numParams; j++)
        {
            atp_param_t *paramTemp = &(domain->paramsArray[j]);
            if (strcmp(paramTemp->paramName, pname) == 0)
            {  
                param = paramTemp;
                break; 
            }
        }
    }
    return param;
}

//-----------------------------------------------------------------------------
atp_constraint_t *ATP_constraint_find(const char *cname, atp_domain_t *domain)
{

    int i;
    atp_constraint_t *constraint = NULL;
    for(i=0; i< domain->numConstraints; i++)
    {
        atp_constraint_t *constTemp = &(domain->constraintsArray[i]);
        if (strcmp(constTemp->constName, cname) == 0)
        {  
            constraint = constTemp;
            break; 
        }
    }

    return constraint;
}

//-----------------------------------------------------------------------------
//new domain declaration
void *ATP_domain_new(const char *domName)
{
   
    atp_domain_t *domain =  ATP_domain_find(domName);
    if( domain != NULL )
        return domain;
 
    uint32_t numDomains = domArray.numDomains;
    
    if ((numDomains+1) > domArray.maxNumDomains)
    {
        uint32_t maxNumDomains =  domArray.maxNumDomains + DOMAIN_MARGIN; 
        domArray.domains = realloc(domArray.domains, sizeof(atp_domain_t) * maxNumDomains);
        if (domArray.domains == NULL)
            ATP_ERROR("no enough memory for allocation"); 
        domArray.maxNumDomains = maxNumDomains;
    }                          
    domain =  &(domArray.domains[numDomains]);
    domain->domName = strdup(domName); 
    domain->paramsArray = NULL;
    domain->numParams = 0;
    domain->maxNumParams = PARAMS_MARGIN;
    domain->constraintsArray = NULL;
    domain->numConstraints = 0;
    domain->maxNumConstraints = CONSTRAINTS_MARGIN;
    domain->explorationArray = NULL;
    domain->numExplorations = 0;
    domain->maxNumExplorations = EXPLORATIONS_MARGIN;
    domArray.numDomains = numDomains + 1;
    return domain;     
}


//-----------------------------------------------------------------------------
//new parameter creation
void *ATP_param_new(const char *pname, uint8_t ptype, int32_t deflt,
                    atp_domain_t *domain) 
{
    uint32_t numParams = domain->numParams;  
    if( ((numParams + 1) > domain->maxNumParams) || (numParams == 0) )
    {
        uint32_t maxNumParams = domain->maxNumParams + PARAMS_MARGIN;
        domain->paramsArray = realloc(domain->paramsArray, sizeof(atp_param_t)*maxNumParams);
        if(domain->paramsArray == NULL)
            ATP_ERROR("no enough memory for allocation"); 
        domain->maxNumParams = maxNumParams;
    }    
    atp_param_t *param = &(domain->paramsArray[numParams]);
    param->paramName = strdup(pname);
    param->paramName = strdup(pname);
    param->paramName = strdup(pname);
    param->paramType = ptype;
    param->defaultVal = deflt;
    domain->numParams = numParams + 1;
    return param;
}

//-----------------------------------------------------------------------------
void *ATP_constraint_new(const char *cname, const char *expr, 
                         atp_domain_t *domain)
{

    uint32_t numConstraints = domain->numConstraints;
    if( ((numConstraints + 1) > domain->maxNumConstraints) 
        || (numConstraints == 0))
    {
        uint32_t maxNumConstraints = domain->maxNumConstraints + PARAMS_MARGIN;
        domain->constraintsArray = realloc(domain->constraintsArray, sizeof(atp_constraint_t)*maxNumConstraints);
        if(domain->constraintsArray == NULL)
            ATP_ERROR("no enough memory for allocation"); 
        domain->maxNumConstraints = maxNumConstraints;
    }    
    atp_constraint_t *constraint = &(domain->constraintsArray[numConstraints]);
    constraint->constName = strdup(cname);
    constraint->constExpr = strdup(expr);        
    domain->numConstraints = numConstraints + 1;
    return constraint;
}


//-----------------------------------------------------------------------------
void *ATP_param_add_values(atp_param_t *param, int32_t *vArray,uint32_t nValues)
{
    int *valueArray = malloc(sizeof(int32_t) * nValues);
    if (valueArray == NULL) 
        ATP_ERROR("no enough memory for allocation\n");      
    int i;
    for(i=0; i < nValues; i++)
        valueArray[i] = vArray[i]; 
    param->valueArray = valueArray;
    param->nValues = nValues;
    return param;
}


void ATP_exploration_new(const char *expList, int nValues, atp_domain_t *domain)
{ 

    uint8_t *explorationArray = malloc(sizeof(uint8_t) * nValues);
    if(explorationArray == NULL)
        ATP_ERROR("no enough memory for allocation\n");      
    int i;
    for (i=0; i<nValues; i++)
        explorationArray[i] = expList[i];
    domain->explorationArray = explorationArray;          
}

//-----------------------------------------------------------------------------
void ATP_gen_descfile()
{

    char file_path[200];
    const char *atp_path = getenv("ATP_DESCRIPTION_FILE");
    if (atp_path == NULL)
      sprintf(file_path,"ATP_description_file.json");
    else
      sprintf(file_path,"%s",atp_path);

    //create a lockfile - only one process/thread will suceed and write the desc file
    if (ATP_create_lockfile() < 0)
      return;
    
    FILE *desc = fopen(file_path,"w");
    if (desc == NULL)
    {
        ATP_ERROR("cannot write to description file in %s", file_path);
        return;
    }
    
    int i;
    fprintf(desc,"{\n");
    for(i=0; i < domArray.numDomains; i++)
    { 
        atp_domain_t *domain = &(domArray.domains[i]);
        fprintf(desc,"\"%s\":{\n",domain->domName);
        fprintf(desc,"\"params\":{\n");
        int j;
        for (j=0; j<domain->numParams; j++)
        {
            atp_param_t *param = &(domain->paramsArray[j]);
            int k;
            fprintf(desc,"\"%s\":{\"range\":[",param->paramName); 
            if(param->nValues > 0)
                fprintf(desc,"%d",(int32_t)param->valueArray[0]); 
            for(k=1; k<param->nValues; k++)
            {        
                fprintf(desc,",%d",(int32_t)param->valueArray[k]); 
            }
            fprintf(desc,"],\"type\":%d",param->paramType);
            fprintf(desc,",\"default\":%d}",param->defaultVal);
            if(j!= domain->numParams -1)
            {
                 fprintf(desc,",\n");
            }
        }
        fprintf(desc, "\n},\n");
        fprintf(desc, "\"constraints\":{\n");
        for(j=0; j< domain->numConstraints; j++)
        {
            atp_constraint_t *constraint = &(domain->constraintsArray[j]);
            fprintf(desc,"\"%s\":{\"expr\":\"%s\"}",constraint->constName,
                                                    constraint->constExpr);
           if(j != domain->numConstraints - 1)
               fprintf(desc,",\n");
        }
        fprintf(desc, "},\n");
        fprintf(desc, "\"explorations\":[\n");
        if(domain->numExplorations > 0)
             fprintf(desc, "%d",domain->explorationArray[j]);
        for(j=0; j< domain->numExplorations; j++)
        { 
            fprintf(desc, ",\"%d\"",domain->explorationArray[j]);
        }
        fprintf(desc, "]\n");
        if(i != domArray.numDomains -1)
        {
            fprintf(desc,"},\n");
        }else{
            fprintf(desc,"}\n");
        }
    } 
    fprintf(desc,"}\n");
    fclose(desc);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int32_t ATP_get_total_parameter_number()
{
    int32_t total_param_num = 0;
    int i;
    for(i=0; i < domArray.numDomains; i++)
    { 
        atp_domain_t *domain = &(domArray.domains[i]);
        total_param_num = total_param_num + domain->numParams;
    }

    return total_param_num;
}

int32_t ATP_get_total_domain_number()
{
    return domArray.numDomains;
}

int32_t ATP_get_total_parameter_values_number()
{

    int32_t total_param_values_num = 0;
    int i;
    for(i=0; i < domArray.numDomains; i++)
    { 
        atp_domain_t *domain = &(domArray.domains[i]);
        int j;
        for(j=0; j<domain->numParams; j++)
        {
            atp_param_t *param = &(domain->paramsArray[j]);
            total_param_values_num = total_param_values_num + param->nValues;
        }
    }

    return total_param_values_num;
}

int32_t *ATP_get_total_domain_details() 
{
    int32_t *domain_details = (int32_t *)malloc(sizeof(int) * domArray.numDomains);
    if(domain_details == NULL)
    {
        ATP_ERROR("ATP could not allocate memory in 'ATP_get_total_domain_details()'\n");
        return NULL;
    }
    
    int i = 0;
    for(i=0; i < domArray.numDomains; i++)
      domain_details[i] = domArray.domains[i].numParams;
    return domain_details; 
}

char *ATP_get_total_parameter_names()
{
    int32_t param_nums = ATP_get_total_parameter_number();
    int32_t buf_size = param_nums * 100;
    char *param_names = (char *)malloc(param_nums * 100);
    if(param_names == NULL)
    {
        ATP_ERROR("ATP could not allocate memory in 'ATP_get_total_parameter_names()'\n");
        return NULL;
    }
    memset(param_names, '\0', buf_size); 
    
    int i, index = 0;
    for(i=0; i < domArray.numDomains; i++)
    { 
        atp_domain_t *domain = &(domArray.domains[i]);
        int j;
        for (j=0; j<domain->numParams; j++)
        {
            atp_param_t *param = &(domain->paramsArray[j]);
            strcpy(&(param_names[index]),param->paramName);
	    index = index + 100; 
        }
    }
    return param_names; 
}

char *ATP_get_total_domain_names()
{
   int32_t domain_nums = domArray.numDomains;
   int32_t buf_size = domain_nums * 100;
   char *domain_names = (char *)malloc(buf_size);
   if(domain_names == NULL)
   {
     ATP_ERROR("ATP could not allocate memory in 'ATP_get_total_domain_names()'\n");
     return NULL;
   }
   memset(domain_names, '\0', buf_size); 
    
   int i, index = 0;
   for(i=0; i < domArray.numDomains; i++)
   { 
     strcpy(&(domain_names[index]),domArray.domains[i].domName);
     index = index + 100; 
   }
   return domain_names;
}


int32_t *ATP_get_total_parameter_details()
{
    int32_t param_nums = ATP_get_total_parameter_number();
    int32_t param_values_num = ATP_get_total_parameter_values_number();
    int32_t buf_size = ((param_nums * 3 + param_values_num )* sizeof(int32_t));
    int32_t *param_details = (int32_t *)malloc(buf_size);
    if(param_details == NULL)
    {
        ATP_ERROR("ATP could not allocate memory in 'ATP_get_total_parameter_details()'\n");
        return NULL;
    }
    ATP_WARNING("PREPARING ...\n");    
    int i, index = 0;
    for(i=0; i < domArray.numDomains; i++)
    { 
        atp_domain_t *domain = &(domArray.domains[i]);
        int j;
        for (j=0; j<domain->numParams; j++)
        {
            atp_param_t *param = &(domain->paramsArray[j]);
            param_details[index] = param->paramType;
            param_details[index + 1] = param->defaultVal;
            if(param->paramType == 2)  //Need to use a MACRO here (2 refers to a type) the corresponding macro is in atplib.h
            {  
                param_details[index + 2] = param->nValues;
            }else{
                param_details[index + 2] = (param->valueArray[1] - param->valueArray[0]) / param->valueArray[2];
            }
            index = index + 3;

            int k; 
            for(k=0; k<param->nValues; k++)
            {
            
                param_details[index + k] = param->valueArray[k];
 
            }
            index = index + param->nValues;
        }
    }
    return param_details; 
}


//-----------------------------------------------------------------------------

char *ATP_build_oc_request(const char *domName)
{

    int i,j, size;
    //request range size = 50 bytes for every range param + 50 * nValues for every enum
    atp_domain_t *domain = ATP_domain_find(domName); 
    for (i = 0; i < domain->numParams; i++) {
      if (domain->paramsArray[i].paramType == ATP_PARAM_TYPE_RANGE)
	size += 50;
      else
	size += 50 * domain->paramsArray[i].nValues;
    }
    
    //allocate tmp memory
    char *request_param_1 = malloc(domain->numParams * 20);
    char *request_param_2 = malloc(domain->numParams * 20);
    char *request_range = malloc(size);  
    char *request_constraint = malloc(domain->numConstraints * 100); 

    sprintf(request_param_1,"[ "); 
    sprintf(request_param_2,"-> [ ");
    sprintf(request_range,": "); 
    sprintf(request_constraint," "); 
    ATP_TRACE("Building OC request for domain '%s' with %d parameters\n", 
              domName, domain->numParams);

    //bult oc query for the first parameter
    if(domain->numParams)
    {
        atp_param_t param = domain->paramsArray[0];

	if (param.paramType == ATP_PARAM_TYPE_RANGE) {
	  strcat(request_param_1,param.paramName);
	  char str[50];
	  sprintf(str,"R0 ");
	  strcat(request_param_2, str);
	  sprintf(str,"(%d <= R%d <= %d) && ( R%d = %s * %d ) ", 
		  param.valueArray[0], 0, param.valueArray[1],
		  0,param.paramName, param.valueArray[2]); 
	  ATP_TRACE("Adding `%s` to the OC request\n", str);
	  strcat(request_range,str);            
	} else if (param.paramType == ATP_PARAM_TYPE_ENUM) {
	  //take the first value as the start of the range nad last value as end of range
	  strcat(request_param_1,param.paramName);
	  char str[50];
	  char *str_enum = malloc(50 * param.nValues);
	  sprintf(str,",R0 ");
	  strcat(request_param_2, str); 
	  
	  //creating ranges
	  sprintf(str_enum,"( (R%d = %d) ", 0, param.valueArray[0]);
	  for (j = 1; j < param.nValues; j++) {
	    sprintf(str, "|| (R%d = %d) ", 0, param.valueArray[j]);
	    strcat(str_enum, str);
	  }
	  sprintf(str, " ) && ( R%d = %s * %d )", 0, param.paramName, 1);
	  strcat(str_enum, str);

	  ATP_TRACE("Adding `%s` to the OC request\n", str_enum);
	  strcat(request_range,str_enum);

	  free(str_enum);
	}
    }

    //build oc query for the rest of the parameters
    for(i=1; i< domain->numParams; i++)
    {
        //creating tuple expression
        atp_param_t param = domain->paramsArray[i];

	if (param.paramType == ATP_PARAM_TYPE_RANGE) {
	  strcat(request_param_1,",");
	  strcat(request_param_1,param.paramName);

	  char str[50];

	  sprintf(str,",R%d ",i);
	  strcat(request_param_2, str); 
	  //creating ranges
	  sprintf(str,"&& (%d <= R%d <= %d) && ( R%d = %s * %d ) ", 
		  param.valueArray[0], i, param.valueArray[1],
		  i,param.paramName, param.valueArray[2]);      
	  ATP_TRACE("Adding `%s` to the OC request\n", str);
	  strcat(request_range,str);
	} else if (param.paramType == ATP_PARAM_TYPE_ENUM) { 

	  //take the first value as the start of the range nad last value as end of range
	  strcat(request_param_1,",");
	  strcat(request_param_1,param.paramName);
	  char str[50];
	  char *str_enum = malloc(50 * param.nValues);
	  sprintf(str,",R%d ",i);
	  strcat(request_param_2, str); 
	  
	  //creating ranges
	  sprintf(str_enum,"&& ( (R%d = %d) ", i, param.valueArray[0]);
	  for (j = 1; j < param.nValues; j++) {
	    sprintf(str, "|| (R%d = %d) ", i, param.valueArray[j]);
	    strcat(str_enum, str);
	  }
	  sprintf(str, " ) && ( R%d = %s * %d )",i, param.paramName, 1);
	  strcat(str_enum, str);

	  ATP_TRACE("Adding `%s` to the OC request\n", str_enum);
	  strcat(request_range,str_enum);

	  free(str_enum);
	}            
    }          

    strcat(request_param_1,"]"); 
    strcat(request_param_2,"]"); 

    if(domain->numConstraints)
    {
       atp_constraint_t constraint = domain->constraintsArray[0]; 
       sprintf(request_constraint," && "); 
       char str[100];
       sprintf(str,"( %s ) ", constraint.constExpr);
       ATP_TRACE("Adding `%s` to the OC request\n", str);
       strcat(request_constraint, str);
    }

    for(i=1; i< domain->numConstraints; i++)
    {
       atp_constraint_t constraint = domain->constraintsArray[i]; 
       char str[100];
       sprintf(str,"&& ( %s ) ", constraint.constExpr);
       ATP_TRACE("Adding `%s` to the OC request\n", str);
       strcat(request_constraint, str);
    }

    char *full_request = malloc(strlen(request_param_1) + 
                                strlen(request_param_2) +
                                strlen(request_range) + 2000);
    sprintf(full_request,"R := { ");
    strcat(full_request,request_param_1);
    strcat(full_request,request_param_2);
    strcat(full_request,request_range);
    strcat(full_request,request_constraint);
    strcat(full_request," };\n");
    strcat(full_request,"B := range R;\n");
    strcat(full_request,"codegen B;\n"); 
    ATP_TRACE("Full OC request constructed  : %s\n", full_request); 

    free(request_param_1);
    free(request_param_2);
    free(request_range);
    free(request_constraint);

    return full_request;
}

//-----------------------------------------------------------------------------

char **ATP_get_domain_names()
{
 
    if(domArray.numDomains == 0)
        return NULL;

    char **domList = malloc(domArray.numDomains * sizeof(char *));
    int i;
    for(i=0; i < domArray.numDomains; i++)
    { 
        domList[i] = (char *)domArray.domains[i].domName;
    }
    return domList;
}

int ATP_get_num_domains()
{

  return domArray.numDomains;

}
