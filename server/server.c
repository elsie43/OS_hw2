#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>

#include <semaphore.h>
#include "types.h"
#include "sock.h"

typedef struct node
{
    char key[101];
    char value[101];
    struct node *next;
} Node;
Node *head = NULL;

void add_node(Node **start, char *key, char *value);
void delete_node(Node **start, char *key);
void print_list(Node *node);
void free_list(Node *node);
char *get_node(Node **start, char *key);

void *thread_routine (void *argv)
{
    pthread_detach(pthread_self());
    int connfd = *((int*)argv);
    int r;
    sem_t mutex;
    while(1)
    {
        // listen from client
        pthread_mutex_lock(&mutex);

        char buffer[101] = {0};
        char result[200] = {0}; // for sending result to client

        memset(result,0,200);
        memset(buffer,0,101);
        r = recv(connfd, buffer, sizeof(buffer), 0);
        if(r<0)
        {
            printf("receive failed");
            pthread_exit(0);
        }
        else if(r == 0)
        {
            printf("Client Leave\n\n");
            pthread_exit(0);
        }

        fprintf(stderr, "%s\n", buffer);
        // insert judge message sent from client
        char *key;
        char *value;

        if (strncmp(buffer, "SET ", 4) == 0)
        {
            char *token;
            char *rest = buffer;
            char *arr[3];

            int i = 0;

            while ((token = strtok_r(rest, " ", &rest)))
            {
                //printf("%s\n", token);
                if (i < 3)
                    arr[i] = token;
                i++;
            }
            if (i == 3)
            {
                key = arr[1];
                value = arr[2];
                if (strncmp(get_node(&head, key), "Failed", 6) == 0)
                {
                    //adding after get failed(not exist)
                    printf("set key: %s\n", arr[1]);
                    printf("set value: %s\n", arr[2]);
                    add_node(&head, key, value);
                    //result = "SET success!\n";
                    memset(result,0,200);
                    strcpy(result,"SET success!");
                }
                else
                {
                    printf("ERROR: key already exist\n");
                    //result = "ERROR: key already exist";
                    memset(result,0,200);
                    strcpy(result,"ERROR: key already exist");
                    //printf("set key: %s\n", arr[1]);
                    //printf("set value: %s\n", arr[2]);
                    //add_node(&head, key, value);
                }
            }

            else
            {
                printf("SET but format illegal\n");
                memset(result,0,200);
                strcpy(result, "ERROR: SET wrong format");

            }

        }
        else if (strncmp(buffer, "GET ", 4) == 0)
        {
            char *token;
            char *rest = buffer;
            char *arr_get[2];

            int i = 0;

            while ((token = strtok_r(rest, " ", &rest)))
            {
                //printf("%s\n", token);
                if (i < 2)
                    arr_get[i] = token;
                i++;
            }
            if (i == 2)
            {
                key = arr_get[1];
                value = get_node(&head, arr_get[1]);
                printf("Get key: %s, value: %s \n", key, value);

                //printf("Get key: %s, value: %s \n", arr_get[1], get_node(&head, arr_get[1]));
                if(strncmp(value, "Failed", 6)==0)
                {
                    // result = "GET Failed";
                    memset(result,0,200);
                    strcpy(result,"GET Failed");

                }
                else
                {
                    //result = "GET success, value = ";
                    memset(result,0,200);
                    strcpy( result,"GET success, value = ");
                    strncat(result,value, strlen(value));

                }
                //strncat(result,value, strlen(value));
            }
            else
            {
                printf("GET but format illegal\n");
                //result = "GET wrong format";
                memset(result,0,200);
                strcpy( result, "GET wrong format");

            }
            //printf("GET but format illegal\n");
        }

        else if (strncmp(buffer, "DELETE ", 7) == 0)
        {
            char *token;
            char *rest = buffer;
            char *arr_del[2];

            int i = 0;

            while ((token = strtok_r(rest, " ", &rest)))
            {
                //printf("%s\n", token);
                if (i < 2)
                    arr_del[i] = token;
                i++;
            }
            if (i == 2)
            {
                key = arr_del[1];
                //delete_node(&head, key);
                // result = "DELETE success\n";
                value = get_node(&head, arr_del[1]);

                if(strncmp(value, "Failed", 6)==0)
                {
                    // result = "GET Failed";
                    memset(result,0,200);
                    strcpy(result,"DELETE Failed");

                }
                else
                {
                    memset(result,0,200);
                    strcpy(result,"DELETE success");
                }
                delete_node(&head, key);
            }
            else
            {
                // result = "DELETE wrong format";
                memset(result,0,200);
                strcpy(result,"DELETE wrong format");
                printf("DELETE but format illegal\n");
            }
            //printf("DELETE but format illegal\n");
        }
        else
        {
            printf("Neither SET, GET, or DELETE\n");
            //result = "ERROR: Command wrong";
            memset(result,0,200);
            strcpy(result,"ERROR: Command wrong");

        }

        //add_node(&head, buffer, "7"); // SET
        print_list(head);

        // send success msg to client
        //char msg[101];
        //strncpy(msg, result, strlen(result));
        int len, bytes_sent;
        len = strlen(result);
        bytes_sent = send(connfd, result, len, 0);
        pthread_mutex_unlock(&mutex);
        memset(result,0,200);
    }
    //pthread_detach(pthread_self());

}

int main(int argc, char **argv)
{
    char *server_port = 0;
    int opt = 0;

    /* Parsing args */
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
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

    if (!server_port)
    {
        fprintf(stderr, "Error! No port number provided!\n");
        exit(1);
    }

    /* Open a listen socket fd */
    int listenfd __attribute__((unused)) = open_listenfd(server_port);

    /* Start coding your server code here! */
    fprintf(stderr, "Server ready, port = %s\n\n", server_port);
    int *connfd;
    struct sockaddr_storage their_addr;
    pthread_t tid;
    socklen_t addr_size;

    //Node *head = NULL;


    while (1)
    {
        addr_size = sizeof their_addr;
        connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr *)&their_addr, &addr_size);
        if(pthread_create(&tid, NULL, thread_routine, connfd)<0)
        {
            perror("pthread_create");
            //printf("<0!");
            return -1;
        }
        /*
        int r;
        	char buf[100];
        	r = recv(*connfd, buf, sizeof(buf), 0);
        	if(r<0)
        	printf("receive failed");
        fprintf(stderr, "%s\n", buf);
        */

        //printf("%d", r);
        //return 0;

        /*char *msg = "Beej was here!";
        int len, bytes_sent;
        len = strlen(msg);
        bytes_sent = send(*connfd, msg, len, 0); */
    }
    free_list(head);

    return 0;
}
void add_node(Node **start, char *newkey, char *newvalue)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    strcpy(new_node->key, newkey);
    strcpy(new_node->value,newvalue);
    new_node->next = NULL;

    if (*start == NULL)
    {
        *start = new_node;
        return;
    }
    else
    {
        Node *current;
        current = *start;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
        return;
    }
}

void delete_node(Node **start, char *key)
{
    Node *current = *start;
    Node *temp;
    if(current == NULL)
    {
        printf("Delete failed!\n");
        return ;
    }

    if (strcmp(key, current->key) == 0)
    {
        printf("delete key: %s\n", current->key);
        *start = current->next;
        free(current);
        return;
    }

    while (current != NULL)
    {
        if (strcmp(current->next->key, key) == 0)
        {
            temp = current->next;
            printf("delete key: %s\n", temp->key);
            current->next = current->next->next;
            free(temp);
            return;
        }
        if (current->next->next == NULL)
        {
            printf("delete failed(%s)\n", key);
            return;
        }

        current = current->next;
    }
}

char *get_node(Node **start, char *key)
{
    Node *current = *start;
    Node *temp;
    if (current == NULL)
    {
        return "Failed";
    }
    if (strcmp(key, current->key) == 0)
    {
        return current->value;
    }
    for (; current->next != NULL; current = current->next)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
    }
    if (strcmp(current->key, key) == 0)
    {
        return current->value;
    }

    return "Failed";
}

void print_list(Node *node)
{
    printf("in DB: ");
    while (node != NULL)
    {
        printf("%s ", node->key);
        node = node->next;
    }
    printf("\n\n");
}

void free_list(Node *node)
{
    Node *current, *temp;
    current = node;
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }
}


