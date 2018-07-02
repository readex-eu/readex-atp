/** ATP library test.
 * Tests the atp library function - ATP_PARAM_DECLARE, ATP_PARAM_ADD_VALUES, APT_PARAM_GET,
 * ATP_CONSTRAINT_DECLARE and ATP_EXPLORATION_DECLARE.
 * Creates multiple domains each containing multiple parameters. Defains multiple constraints
 * and explorations for each domain. Writes the ATP description file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atplib.h"
#include "atpcommon.h"

void printHelp() {
  printf("\nATP library test case\n");
  printf("Creates multiple ATP domains each with multiple parameters.\n");
  printf("Adds constraints and explorations to each domain.\n");
  printf("Different parameter types ('range' and 'enum') are used.\n\n");
  printf("To change the number of domains, use -d <integer>\n");
  printf("To change the number of parameters, use -p <integer>\n");
  printf("To see help, use -h\n");
}

int main(int argc, char **argv) {

  int i,dom,par;
  const char *domain_prefix = "domain";
  const char *parameter_prefix = "param";
  const char *constraint_prefix = "constraint";

  int numDomains = 3;
  int numParams = 3;

  for (i = 1; i < argc; i++) {
    //print help message
    if (strncmp(argv[i], "-h", 2) == 0) {
      printHelp();
      return 0;
    }
    //set number of domains
    if (strncmp(argv[i], "-d", 2) == 0 && i + 1 < argc)
      numDomains = atoi(argv[i + 1]);

    //set number of parameters
    if (strncmp(argv[i], "-p", 2) == 0 && i + 1 < argc)
      numParams = atoi(argv[i + 1]);
     
  }

  //force ATP_EXECUTION_MODE=DTA environment variable
  putenv("ATP_EXECUTION_MODE=DTA");

  printf("Running ATP library test with %d domains and %d parameters per domain\n", 
	 numDomains, numParams);

  //loop trough the domains and parameters and create a parameterd for each
  for (dom = 0; dom < numDomains; dom++) {
    //create a domain name
    char dname[100];
    snprintf(dname, 100, "%s_%d", domain_prefix, dom);

    //half of the parameters are type range
    for (par = 0; par < numParams / 2 + 1; par++) {
      int r_values[3] = {1,4,1};
      char pname[100];
      snprintf(pname, 100, "%s_%d_%d", parameter_prefix, dom, par);
      ATP_PARAM_DECLARE(pname, ATP_PARAM_TYPE_RANGE, 1, dname);
      ATP_PARAM_ADD_VALUES(pname, r_values, 3, dname);
    }

    //other half is type enum
    for (par = numParams / 2 + 1; par < numParams; par++) {
      int e_values[6] = {1, 2, 3, 5, 7, 12};
      char pname[100];
      snprintf(pname, 100, "%s_%d_%d", parameter_prefix, dom, par);
      ATP_PARAM_DECLARE(pname, ATP_PARAM_TYPE_ENUM, 1, dname);
      ATP_PARAM_ADD_VALUES(pname, e_values, 6, dname);
    }

    //create a constraint for all the parameters in domain that p > p+1 (skip for last param)
    for (par = 0; par < numParams - 1; par++) {
      char constraint[256];
      char cname[100], p1name[100], p2name[100];
      snprintf(cname, 100, "%s_%d_%d", constraint_prefix, dom, par);
      snprintf(p1name, 100, "%s_%d_%d", parameter_prefix, dom, par);
      snprintf(p2name, 100, "%s_%d_%d", parameter_prefix, dom, par + 1);
      
      if (dom % 2 == 0)
	snprintf(constraint, 256, "%s > %s", p1name, p2name);
      else
	snprintf(constraint, 256, "%s < %s", p1name, p2name);
      ATP_CONSTRAINT_DECLARE(cname, constraint, dname);
    }
  }

  //generate the atp description file
  atp_end_collection();

  printf("ATP test finished\n");

  return 0;
}
