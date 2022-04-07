#include<sys/socket.h>
#include<netdb.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<fcntl.h>
#define FAIL -1
char hostname[20];
char input[100]; 
void parse(char*);
int OpenConnection(const char *hostname,int port){
	int sd;
	struct hostent *host;
	struct sockaddr_in addr;
	if ( (host= gethostbyname(hostname)) == NULL){
		perror(hostname);
		abort();
	}
	sd= socket(PF_INET, SOCK_STREAM,0);
	memset((void*)&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr= *(long*)(host->h_addr);
	if(connect(sd,(struct sockaddr*)&addr,sizeof(addr)) !=0){
		close(sd);
		perror(hostname);
		abort();
	}
	return sd;
}
SSL_CTX* InitCTX(void){     /*creating and setting up ssl context structure*/
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
  SSL_load_error_strings();  /* Bring in and register error messages */
  method=TLS_client_method();
  ctx = SSL_CTX_new(method);   /* Create new context */
  return ctx;
}
int main(int argc, char* argv[]){
	SSL_CTX *ctx;
	int server;
	SSL *ssl;
	char buf[20480];
	if(argc != 2){
		printf("Usage https_h <hostname>\n");
		exit(1);
	}
	SSL_library_init();
	ctx= InitCTX();
	server = OpenConnection(argv[1],443);
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl,server);
	int bytes;
	if( SSL_connect(ssl) == FAIL)
		ERR_print_errors_fp(stderr);
	sprintf(input,"GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",argv[1]); 
	printf("%s",input);
	SSL_write(ssl, input, strlen(input));   /* encrypt & send message */
	bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
	if ( bytes > 0 ){
		buf[bytes]='\0';
		parse(buf);
		printf("%s",buf);
	}
}
void parse(char *str){
	char *data =strstr(str,"\r\n\r\n");
	if(data!=NULL){
		data+=4;
		*data='\0';
	}
}
