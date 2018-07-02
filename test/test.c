#include <stdio.h>
#include <stdlib.h>
#include "atplib.h"



#define NUM_PHASES 6



int main(int argc, char **argv)
{

  int i;
  for(i = 0; i<NUM_PHASES; i++)
  {
    printf("loooooping !\n");
   
    //ATP_PARAM_DECLARE("pe1",ATP_PARAM_TYPE_RANGE,1,"dom1");   
    //int32_t values[3] = {1,10,2};    
    //ATP_PARAM_ADD_VALUES("pe1",values,3,"dom1"); 
    int p2 = 0;
     

    ATP_PARAM_DECLARE("p2",ATP_PARAM_TYPE_RANGE, 1,NULL);   
    int p2_values[3] = {0,3,1};
    ATP_PARAM_ADD_VALUES("p2", p2_values,3,NULL);  
    ATP_PARAM_GET("p2", &p2, NULL);

    printf("Value for p2:%d\n",p2);
   
    int p1 = 0;

    ATP_PARAM_DECLARE("p1",ATP_PARAM_TYPE_RANGE,  2,NULL);   
    int p1_values[3] = {1,10,2};
    ATP_PARAM_ADD_VALUES("p1", p1_values, 3, NULL); 
    ATP_PARAM_GET("p1", &p1 ,NULL);
  
    printf("Value for p1:%d\n",p1);

    int dec = 0;
    printf("stalling .. \n");
    scanf("%d",&dec); 
     
  }



  return EXIT_SUCCESS;
} 
