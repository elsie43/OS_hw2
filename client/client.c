#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdlib.h>

#include "sock.h"

int main(int argc, char **argv)
{
    int opt;
    char *server_host_name = NULL, *server_port = NULL;
    char* scan(char *string)
    {
        int c; //as getchar() returns `int`
        string = malloc(sizeof(char)); //allocating memory
        string[0]='\0';
        for(int i=0; i<100 && (c=getchar())!='\n' && c != EOF ; i++)
            //for(int i=0; i<100 && (c=scanf("%c"))!='\n' && c != EOF ; i++)
        {
            string = realloc(string, (i+2)*sizeof(char));
            //reallocating memory
            string[i] = (char) c; //type casting `int` to `char`
            string[i+1] = '\0'; //inserting null character at the end
        }
        return string;
    }


    /* Parsing args */
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            server_host_name = malloc(strlen(optarg) + 1);
            strncpy(server_host_name, optarg, strlen(optarg));
            break;
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
        case '?':
            fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ?
                    optopt : '#');
            return 0;
        }
    }

    if (!server_host_name)
    {
        fprintf(stderr, "Error!, No host name provided!\n");
        exit(1);
    }

    if (!server_port)
    {
        fprintf(stderr, "Error!, No port number provided!\n");
        exit(1);
    }

    /* Open a client socket fd */
    int clientfd __attribute__((unused)) = open_clientfd(server_host_name, server_port);

    /* Start your coding client code here! */

    /* //receive
     * int r;
    char buf[100];
    r = recv(clientfd, buf, sizeof(buf), 0);
    fprintf(stderr, "%s", buf);
    printf("%d", r);*/

    char *msg = "Beej was here!";
    while(1)
    {
        //scanf("%s\n", msg);
        //scan(msg);
        //*msg = "new mes";
        //printf("%s",msg);

        //read from console and send to server
        char buff[101];
        gets(buff);
        //printf("%s\n",buff);
        msg = buff;
        int len, bytes_sent;
        len = strlen(msg);
        bytes_sent = send(clientfd, msg, len, 0);
        if(bytes_sent <0)
            printf("client sent failed");


        // receive msg from server
        int r;
        char buf[101];
        memset(buf, 0, 101);
        r = recv(clientfd, buf, sizeof(buf), 0);
        fprintf(stderr, "%s\n", buf);
        //printf("%d", r)
    }
    return 0;
}
