/* 8020 Controller V0.1.0
 * Commands List
 * First Command:
 *  -i: gets module info
 *  -ac: digital active mode
 *  -inac: digital inactive mode
 *  -m: set multiple outputs
 *  -di: get digital inputs
 *  -do: get digital outputs
 *  -a: get analog voltage          (DEPRECATED)
 *  -t: ASCII text command mode
 *  -sn: get MAC address
 *  -v: get supply voltage
 *  -p: password entry              (DEPRECATED)
 *  -u: get unlock time             (DEPRECATED)
 *  -q: log out                     (DEPRECATED)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


void error (char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    int DEFAULTPORT = 17494;           //be careful here

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
     if (argc < 2) {
       fprintf(stderr,"usage %s IP-addr command", argv[0]);
       exit(0);
    }
                portno = DEFAULTPORT;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
        error("ERROR opening socket");

		server = gethostbyname(argv[1]);
		if (server == NULL) {
		  fprintf(stderr,"ERROR, no such host\n");
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


    //begin constants
    
    char *info = "-i";               unsigned char infornum = 0x10;
    char *act = "-ac";               unsigned char activnum = 0x20;
    char *inact = "-inac";           unsigned char inactnum = 0x21;
    char *mult = "-m";               unsigned char multinum = 0x23;
    char *diggetin = "-di";          unsigned char diginnum = 0x25;
    char *diggetout = "-do";         unsigned char dgoutnum = 0x24;
    char *getanvolts = "-a"; //dep
    char *textcom = "-t"; //dep    //unsigned char textcnum = 0x3a;
    char *macadd = "-sn";            unsigned char macadnum = 0x77;
    char *suppvolts = "-v";          unsigned char suppvnum = 0x78;
    char *passent = "-p"; //dep
    char *unlcktime = "-u"; //dep  //unsigned char unlcknum = 0x7a;
    char *logoutcom = "-q"; //dep

    //end constants

    unsigned char sendcode;
    
    unsigned char *a1 = argv[2];

    //printf("%s\n", argv[2]);           //TEST CODE
    //printf("%s\n", a1);                //TEST CODE   

    if      (strcmp(a1, info) == 0)      sendcode = infornum;
    else if (strcmp(a1, act) == 0)       sendcode = activnum;
    else if (strcmp(a1, mult) == 0)      sendcode = multinum;
    else if (strcmp(a1, inact) == 0)     sendcode = inactnum;
    else if (strcmp(a1, diggetin) == 0)  sendcode = diginnum;
    else if (strcmp(a1, diggetout) == 0) sendcode = dgoutnum;
    else if (strcmp(a1, macadd) == 0)    sendcode = macadnum;
    else if (strcmp(a1, suppvolts) == 0) sendcode = suppvnum;
    else {
      printf("Unacceptable second argument. Program terminated. Bye!\n");
      exit(0);
    }


    unsigned int relnum;
    unsigned int timems;

    unsigned char sendbuff[32];

    //BEGIN IF BLOCKS


    if (strcmp(a1, act) == 0) {
      if (argc < 3) {
	printf("Insufficient number of arguments. ");
	printf("Program terminated. Bye!\n");
      }
      
      relnum = strtoul(argv[3], NULL, 10);
      
      timems = 0x00;

      //printf("sendcode = %u\n", sendcode); //TEST CODE
      //printf("relnum = %u\n", relnum);     //TEST CODE
      //printf("timems = %u\n", timems);     //TEST CODE

      int sendcodeshift;
      int relnumshift;
      int pass;

      sendcodeshift = sendcode << 16;
      relnumshift = relnum << 8;
      
      pass = htonl(sendcodeshift + relnumshift + timems);

      unsigned char finpass[32];

      finpass[0] = sendcode;
      finpass[1] = relnum;
      finpass[2] = timems;

      //printf("BEGIN FINPASS VALS\n");         //TEST CODE
      //printf("%d\n", finpass[0]);             //TEST CODE
      //printf("%d\n", finpass[1]);             //TEST CODE
      //printf("%d\n", finpass[2]);             //TEST CODE

              int q;
              size_t lenq;
              lenq = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenq, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    }

    if (strcmp(a1, inact) == 0) {
      if (argc < 3) {
	printf("Insufficient number of arguments. ");
	printf("Program terminated. Bye!\n");
      }
      
      relnum = strtoul(argv[3], NULL, 10);
      
      timems = 0x00;

      int sendcodeshift;
      int relnumshift;
      int pass;

      sendcodeshift = sendcode << 16;
      relnumshift = relnum << 8;
      
      pass = htonl(sendcodeshift + relnumshift + timems);

      unsigned char finpass[32];

      finpass[0] = sendcode;
      finpass[1] = relnum;
      finpass[2] = timems;

              int q;
              size_t lenq;
              lenq = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenq, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    }

    if(strcmp(a1, info) == 0) {
     
      sendcode = infornum;

      unsigned char finpass[32];
      finpass[0] = sendcode;

       int q;
       size_t lenx;
       lenx = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");
    int i_ = buffer[0];
    int ii_ = buffer[1];
    int iii_ = buffer[2];
    printf("Module ID: %d\n", i_);
    printf("Hardware version: %d\n", ii_);
    printf("Firmware version: %d\n", iii_);

    }

    if(strcmp(a1, macadd) == 0) {
     
      sendcode = macadnum;

      unsigned char finpass[32];
      finpass[0] = sendcode;

       int q;
       size_t lenx;
       lenx = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");
    int i_ = (0xFF & buffer[0]);
    int ii_ = (0xFF & buffer[1]);
    int iii_ = (0xFF & buffer[2]);
    int iv_ = (0xFF & buffer[3]);
    int v_ = (0xFF & buffer[4]);
    int vi_ = (0xFF & buffer[5]);

    printf("MAC address: ");
    printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", i_, ii_, iii_, iv_, v_, vi_);

    }


    if(strcmp(a1, diggetout) == 0) {
     
      sendcode = dgoutnum;

      unsigned char finpass[32];
      finpass[0] = sendcode;

       int q;
       size_t lenx;
       lenx = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");

    int i_ = (0xFF & buffer[0]);
    int ii_ = (0xFF & buffer[1]);
    int iii_ = (0xFF & buffer[2]);

    int ii_tot = ii_ << 8;
    int iii_tot = iii_ << 16;

    int tot = i_ + ii_tot + iii_tot;
    
    unsigned int bits[32];
    int index = 0;

    int ind_display = 1;

    //printf("Format: relay number above relay state\n");

    printf("Outputs:\n");

    for(ind_display = 1; ind_display < 21; ind_display++) {
      
      printf("%02d  ", ind_display);
      
    }
    
    printf("\n");

    for(index = 0; index < 20; index++) {
      
      bits[index] = (tot >> index) & 0x01;

      //printf("  %d ", bits[index]);             //left in on purpose
      
      if(bits[index] == 1) printf("ON  ");
      else printf("--  ");
      
    }
    printf("\n\n");
    }



    if(strcmp(a1, diggetin) == 0) {
     
      sendcode = diginnum;

      unsigned char finpass[32];
      finpass[0] = sendcode;

       int q;
       size_t lenx;
       lenx = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");

    int i_ = (0xFF & buffer[0]);
    int ii_ = (0xFF & buffer[1]);
    int iii_ = (0xFF & buffer[2]);

    unsigned int bits[32];
    int index = 0;

    int ind_display = 1;

    //printf("Format: relay number above relay state\n");

    printf("Inputs:\n");

    for(ind_display = 1; ind_display < 9; ind_display++) {
      
      printf("%02d  ", ind_display);
      
    }
    
    printf("\n");

    for(index = 0; index < 8; index++) {
      
      bits[index] = (iii_ >> index) & 0x01;

      //printf("  %d ", bits[index]);             //left in on purpose
      
      if(bits[index] == 1) printf("ON  ");
      else printf("--  ");
      
    }
    printf("\n\n");
    }


    if(strcmp(a1, suppvolts) == 0) {
     
      sendcode = suppvnum;

      unsigned char finpass[32];
      finpass[0] = sendcode;

       int q;
       size_t lenx;
       lenx = 3 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");

    int i_ = (0xFF & buffer[0]);
    //int ii_ = (0xFF & buffer[1]);
    //int iii_ = (0xFF & buffer[2]);

    float i_f = ((float)i_)/10;

    printf("Input voltage: %2.1f V DC\n", i_f);

    }
    

    if(strcmp(a1, mult) == 0) {

      printf("WARNING: This mode only sends a pulse. Avoid use.\n");
     
      sendcode = multinum;

      long unsigned int initinput;
      unsigned int fixedinput;

      if (strlen(argv[3]) != 5) {
	printf("Enter exactly 5 hexadecimal digits.\n");
	exit(0);
      }
      
      initinput = strtol(argv[3], NULL, 16);

      fixedinput = initinput * 0x10;

      unsigned char i_1;
      unsigned char i_2;
      unsigned char i_3;

      //printf("%X\n", initinput);               //TEST CODE
      //printf("%X\n", fixedinput);              //TEST CODE

      i_1 = fixedinput >> 16;
      i_2 = ((fixedinput >> 8) << 8)/256;
      i_3 = (fixedinput << 16)/65536;
      
      //printf("OI M8\n");        //TEST CODE

      //printf("%X\n", i_1);    //TEST CODE
      //printf("%X\n", i_2);    //TEST CODE
      //printf("%X\n", i_3);    //TEST CODE
     
      unsigned char finpass[32];
      finpass[0] = sendcode;
      finpass[1] = i_1;
      finpass[2] = i_2;
      finpass[3] = i_3;

      
      finpass[1] = 0xFF;        //TEST CODE
      finpass[2] = 0x0F;        //TEST CODE
      finpass[3] = 0xF0;        //TEST CODE

       int q;
       size_t lenx;
       lenx = 5 * sizeof(char);

	      q = send(sockfd, finpass, lenx, 0);
    
          if (q < 0){
          error("ERROR writing to socket");
          }
    bzero(buffer,256);
    q = read(sockfd,buffer,255);
    if (q < 0)
         error("ERROR reading from socket");

    printf("\n");

    }
    


    else {

      exit(0); //broken without -args

      //sendcode = atoi(argv[3]);
      /*
    int m;
    size_t len;
    len = sizeof(sendcode);

    m = send(sockfd, sendbuff, len, 0);
    
      if (m < 0){
      error("ERROR writing to socket");

      }
      
      */                 //BLOCK ABOVE TO BE UTILIZED LATER

   }

}
