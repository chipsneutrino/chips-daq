/* USER'S GUIDE - IA-3k-RelayControl V1.0.1
 * Command line arguments required to run:
 * *In -s (single) mode:
 * **IPv4-address, -s, number of relay to be turned on (1-32), 1/0 for on/off
 * *In -m (multiple) mode:
 * **IPv4-address, -m, eight hexadecimal digits (reference: pg. 22 of manual)
 * Known bugs: only first char of feedback is displayed on some computers
 * *********** Help does not work
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>

void help()
{
      printf("\nHELP:\n\n");
      printf("IARelayControl can be used in single relay mode, -s, and multiple relay mode, -m.\n");
      printf("Except when using -h for help, the IP address of the board is always the first arg.\n\n");
      printf("Usage examples:\n");
      printf("192.168.1.142 -s 1 1 turns on Relay 1 of the board at 192.168.1.142\n");
      printf("192.168.1.142 -m FFFFFFFF turns on all relays of the board at 192.168.1.142\n\n");
      printf("Multiple relay mode accepts eight hexadecimal digits as arguments.\n");
      printf("Each stands for four bits, and all eight represent the relay.\n");
      printf("Example: 0F000000 turns on relays 5, 6, 7 and 8, as 0xF = 1111.\n\n");
      printf("Manual reversion mode for advanced tasks:\n");
      printf("192.168.1.142 '<command>' sends <command> to the board at 192.168.1.142\n");
      printf("The single quotes are required if the command begins with ! in order to escape the exclamation point.\n");
      printf("Commands are found in the IA-3174-E manual, available from Intelligent Appliance.\n\n");
      printf("The port number is hardcoded, default value is 23. It is recommended that all boards be set up to use port 23.\n");
      printf("However, it can be found and edited as int DEFAULTPORT in line 29 of the program.\n");
      printf("Thank you for using IARelayControl version 1.0.1!\n\n");
      exit(0);
}

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    int DEFAULTPORT = 23;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
     if (argc < 2) {
       fprintf(stderr,"usage %s [IPv4] [-s/-m] [number/hex] [1/0 in -s mode]", argv[0]);
       printf("\n");
       help();
       exit(0);
    }


                portno = DEFAULTPORT;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
        error("ERROR opening socket");

		server = gethostbyname(argv[1]);
		if (server == NULL) {
		        fprintf(stderr,"ERROR, no such host\n");
			help();
        exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    int len;

    char *escape1 = "\r";
    char *initialon = "!003\0";
    char *initialoff = "!004\0";
    char *initialmanual = "!002\0";
    char *inp_sett = malloc(32 * sizeof(char));

    char *pass_prelim = malloc(32 * sizeof(char));
    pass_prelim[0] = '\0';
    char *pass = malloc(64 * sizeof(char));
    pass[0] = '\0';
    
    char *teststr[2];

    /*

    int optind = 0;                              //option index

    char *opop = NULL;

    while((optind = getopt(argc, argv, "m:s:") != -1) {
      }

    */

    if(strcmp(argv[2], "-s") == 0)
      {
	int relnum;

	char *relhex = malloc(4 * sizeof(char));
	
	relnum = atoi(argv[3]);
	--relnum;

	sprintf(relhex, "%02X", relnum);

	int onoffchoice;
	onoffchoice = atoi(argv[4]);

	if(onoffchoice == 1) {

	      strcpy(pass_prelim, initialon);
	      strncat(pass_prelim, relhex, 2);
	      strncat(pass_prelim, escape1, 2);
	      strcpy(pass, pass_prelim);
	}

	if(onoffchoice == 0) {

	      strcpy(pass_prelim, initialoff);
	      strncat(pass_prelim, relhex, 2);
	      strncat(pass_prelim, escape1, 2);
	      strcpy(pass, pass_prelim);
	}


	if((onoffchoice != 0) && (onoffchoice != 1)) {
	  printf("ERROR: 4th argument in -s mode must be 1 or 0\n");
	  help();
	  exit(0);
	}

      }
    
    int testargc;
    testargc = argc;

    if(testargc == 2) {


      
      char* arg_man;
      int commandlen = strlen(argv[2]);
      arg_man = malloc(commandlen * sizeof(char));
      arg_man = argv[2];
 
      strncat(arg_man, escape1, 2);

      pass = arg_man;

    }

    if(testargc > 5) {
      printf("Invalid argument count for either -s/-m mode or manual reversion mode.\n");
      help();
      exit(0);
    }

    if(strcmp(argv[2], "-m") == 0) {

      strcpy(inp_sett, argv[3]);
      
      if(strlen(inp_sett) != 8) {
	printf("Error: Enter precisely eight hexadecimal digits.\n");
	help();
	exit(0);
      }

      strcpy(pass_prelim, initialmanual);
      strncat(pass_prelim, inp_sett, 9);
      strncat(pass_prelim, escape1, 2);
      strcpy(pass, pass_prelim);
    }

    len = strlen(pass);

    n = send(sockfd, pass, len, 0);

    if (n < 0)
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    return 0;
}

//Stefano Germani, Lucas Rogers for CHIPS
