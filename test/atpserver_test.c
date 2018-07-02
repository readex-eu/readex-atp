/** ATP server test.
 * Tests the atpserver queries - getatpspecs and getvalidparams.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <ctype.h>

void printHelp() {
  printf("\nATP server test case\n");
  printf("Launch aptserver in a seperate process.\n");
  printf("Connect to the atpserve andr check the atpserver queries.");
  printf("Test getatpspec query to see if correct parameter info is sent.\n");
  printf("Test getvalidparams query to see if correct combinations are sent\n");
  printf("To change the port for aptserve, use -p <integer> (default 20000)");
  printf("To see help, use -h\n");
}

int startATPServer(const char *logfile) {

  char *cmd = (char *)malloc(2000*sizeof(char));
  sprintf(cmd, "atpserver > %s 2>&1", logfile);
  system(cmd);
  free(cmd);
}

int connectToATP(int port, FILE *fp) {
  
  int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1) {
    printf("Cannot create socket!\n");
    return 0;
  }

  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( port );
 
  //Connect to remote server
  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    fprintf(fp, "Cannot connect to server!\n");
    return 0;
  }
  fprintf(fp, "Connected to atpserver\n");

  return socket_desc;
}

void closeATP(int socket_desc, FILE *fp) {
  close(socket_desc);
  fprintf(fp, "Connection to server closed!\n");
}

void getATPSpecs(int socket_desc, FILE *fp) {
  
  int i;
  //send getatpspecs query to server
  char *message = "getatpspecs;";
  if (send(socket_desc, message, strlen(message), 0) < 0) {
    fprintf(fp, "Send message failed\n");
    return;
  }

  //receive param num details response from server
  int32_t param_num_details[3];
  if (recv(socket_desc, param_num_details, sizeof(int32_t)*3, 0) < 0) {
    fprintf(fp, "Receive param num details failed\n");
    return;
  }
  fprintf(fp, "received num domains: %d, param nums: %d, and param values num: %d\n", 
	  param_num_details[0], param_num_details[1], param_num_details[2]);

  //receive int array with number of parameters per domian
  int32_t domain_nums = param_num_details[0];
  int32_t *domain_details = malloc(sizeof(int32_t) * domain_nums);
  if (recv(socket_desc, domain_details, sizeof(int32_t) * domain_nums, 0) < 0) {
    fprintf(fp, "Receive domain details failed\n");
    return;
  }
  for(i = 0; i < domain_nums; i++)
    fprintf(fp, "received num of parameters per domain: %d\n", domain_details[i]);

  //receive domain names
  char *domain_names = malloc(100 * domain_nums);
  if (recv(socket_desc, domain_names, 100 * domain_nums, 0) < 0) {
    fprintf(fp, "Receive domain names failed\n");
    return;
  }
  for(i = 0; i < domain_nums; i++)
    fprintf(fp, "received domain names: %s\n", &(domain_names[i*100]));

  //receive parameter names
  int32_t param_nums = param_num_details[1]; 
  char *param_names = malloc(100 * param_nums);
  if (recv(socket_desc, param_names, 100 * param_nums, 0) < 0) {
    fprintf(fp, "Receive param names failed\n");
    return;
  }
  for(i = 0; i < param_nums; i++)
    fprintf(fp, "received param names: %s\n", &(param_names[i*100]));

  //receive parameter details
  int32_t param_values_nums = param_num_details[2];
  int details_size = ((param_nums * 3) + param_values_nums) * sizeof(int32_t);
  int32_t *param_details = malloc(details_size);
  if (recv(socket_desc, param_details, details_size, 0) < 0) {
    fprintf(fp, "Receive param details failed\n");
    return;
  }
  int index = 0;
  for(i = 0; i < param_nums; i++) {
    fprintf(fp, "PARAM: %d\n", i);
    fprintf(fp, "type [%d][%d] = %d\n", i, 0, param_details[index]);
    fprintf(fp, "default [%d][%d] = %d\n", i, 1, param_details[index + 1]);
    fprintf(fp, "max values [%d][%d] = %d\n", i, 2, param_details[index + 2]);

    int k;
    int values_nums = (param_details[index] == 1) ? 3 : param_details[index+2];
    fprintf(fp, "number of values in values tuple %d\n", values_nums);
    for(k = 0; k < values_nums; k++)
      fprintf(fp, "[%d][%d] = %d\n", i, (3 + k), param_details[index+3+k]);
    index = index + 3 + values_nums;
  }

  free(domain_details);
  free(domain_names);
  free(param_names);
  free(param_details);
}

void getValidParams(int socket_desc, FILE *fp, const char *domain_name) {
 
  int i;
  //send getatpspecs query to server
  char *message = (char *)malloc(200);
  sprintf(message, "getvalidparams,%s;",domain_name);
  if (send(socket_desc, message, strlen(message), 0) < 0) {
    fprintf(fp, "Send message failed\n");
    return;
  }

  //receive info array
  int32_t info_array[3];
  if (recv(socket_desc, info_array, sizeof(int32_t)*3, 0) < 0) {
    fprintf(fp, "Receive info array failed\n");
    return;
  }
  fprintf(fp, "Received info: %d, num_combinations: %d, elements per combination: %d\n", 
	  info_array[0], info_array[1], info_array[2]);
  
  //receive combinations
  int num_combinations = info_array[1] * info_array[2];
  int32_t *combinations = (int32_t*)malloc(sizeof(int32_t) * num_combinations);
  if (recv(socket_desc, combinations, sizeof(int32_t) * num_combinations, 0) < 0) {
    fprintf(fp, "Receive combinations failed\n");
    return;
  }
  fprintf(fp, "Received combinations: ");
  for (i = 0; i < num_combinations; i++)
    fprintf(fp, "%d ", combinations[i]);
  fprintf(fp, "\n");

}

int main(int argc, char **argv) {

  int i;
  int loop = -1;
  int port = 20000;
  
  for (i = 1; i < argc; i++) {
    
    if (strncmp(argv[i], "-h", 2) == 0) {
      printHelp();
      return 0;
    }

    if (strncmp(argv[i], "-p", 2) == 0 && i + 1 < argc)
      port = atoi(argv[++i]);

    if (strncmp(argv[i], "-l", 2) == 0) {
      if (isdigit(argv[i+1]))
	loop = atoi(argv[++i]);
      else
	loop = 100;
    }
  }

  int pid = fork();

  if (pid == 0) {
    //parent process launches ATP server
    startATPServer("atpserver.log");
  } else {
    //wait while ATP server is started
    sleep(1);

    //child process connects to ATP server
    FILE *fp = fopen("atpreceive.out", "w");
    int socket_desc = connectToATP(port, fp);

    //test getATPSpecs query
    getATPSpecs(socket_desc, fp);

    //test getValid params query
    getValidParams(socket_desc, fp, "domain_0");
    getValidParams(socket_desc, fp, "domain_1");
    
    
    //test large amount of queries
    if (loop > 0) {
      FILE *fp_loop = fopen("atpserver_loop.out", "w");
      for (int i = 0; i < loop; i++) {
	getValidParams(socket_desc, fp_loop, "domain_0");
	getValidParams(socket_desc, fp_loop, "domain_1");
      }
    }

    //close the atp server
    closeATP(socket_desc, fp);
    fclose(fp);
  }

}
