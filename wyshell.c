/*
 * wyshell.c
 * Author: Kevin Littell
 * Date: 11-15-2018
 * COSC 3750, program 8
 * simple shell utility
 */


#include "stdbool.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "wyscanner.h"

struct Word{
    struct Word *next, *prev;
    char *command;
};

struct Node{
        struct Node *next, *prev;
        char *command;
        int count;
        bool background;
        struct Word *arg_list;
        int in, out, err;
        char *inFile, *outFile, *errFile;
};

char *tokens[]={ "QUOTE_ERROR", "ERROR_CHAR", "SYSTEM_ERROR",
               "EOL", "REDIR_OUT", "REDIR_IN", "APPEND_OUT",
               "REDIR_ERR", "APPEND_ERR", "REDIR_ERR_OUT", "SEMICOLON",
               "PIPE", "AMP", "WORD" };


/* Function to reverse the linked list */
static int reverse(struct Node** headRef)
{
    int count = 0;
    struct Word* prev   = NULL;
    struct Word* current = (*headRef)->arg_list;
    struct Word* next = NULL;

    struct Node* nodeCurrent = *headRef;
    struct Node* nodePrev = NULL;
    struct Node* nodeNext = NULL;

    printList(nodeCurrent->arg_list);
    do{
        printf("first\n");
        while (current != NULL){
            count++;
            // Store next
            next  = current->next;

            // Reverse current node's pointer
            current->next = prev;

            // Move pointers one position ahead.
            prev = current;
            current = next;
        }

        printf("second\n");
        nodeCurrent->count = count;
        nodeNext = nodeCurrent->next;
        printf("third\n");
        nodeCurrent->arg_list = prev;
        printList(nodeCurrent->arg_list);
        nodeCurrent = nodeNext;
        printf("fourth\n");
        current = nodeCurrent->arg_list;
        printf("fifth\n");
    } while(nodeCurrent != NULL);

    //(*headRef)->arg_list = prev;
    return count;
}

/* Function to push a node */
void push(struct Node** headRef, char * data)
{
    struct Word* newWord =
        (struct Word*) calloc(1, sizeof(struct Word));
    newWord->command = strdup(data);
    newWord->next = (*headRef)->arg_list;
    (*headRef)->arg_list = newWord;
}

/* Function to print linked list */
void printList(struct Word *head)
{
  struct Word *tmp = head;
  while(tmp != NULL)
  {
      printf(" %s,", tmp->command);
      tmp = tmp->next;
  }
  printf("\n");
}

void Executer(struct Node * node, int count){

    char ** myArgv;
    myArgv = calloc((count + 2), sizeof(char*));
    myArgv[0] = strdup(node->command);
    struct Word *tmp = node->arg_list;
    for(int i = 0; i < count; i++){
        myArgv[i + 1] = strdup(tmp->command);
        tmp = tmp->next;
    }
    int status;
    pid_t childWait;
    pid_t myExec;
    pid_t frtn = fork();
    if(frtn == 0){
        // child process
        //sleep(2);
        myExec = execvp(node->command, myArgv);
        if(myExec == -1){
            printf("%s: command not found\n",node->command);
        }
        //exit(frtn);
    } else if (frtn > 0){
        if(node->background == true){
            childWait = waitpid(frtn, &status, WNOHANG);
            if(childWait == -1){
                perror("waitpid");
            }
        } else {
            childWait = waitpid(frtn, &status, 0);
        }
        // parent process
    } else if(frtn == -1){
        perror("fork");
    }
    while(tmp != NULL){
        printf("argument is: %s\n", tmp->command);
        tmp = tmp->next;
    }
    if(node->outFile != NULL){
        printf("output file is: %s\n",node->outFile);
    }
    if(node->inFile != NULL){
        printf("input file is: %s\n",node->inFile);
    }
    if(node->errFile != NULL){
        printf("error file is: %s\n",node->errFile);
    }
}

int main (int argc, char * argv[]){
    char buf[256];
    int rtn;
    char *rtpt;
    while(1){
        printf("$> ");
        rtpt = fgets(buf, 256, stdin);
        if(rtpt == NULL){
            if(feof(stdin)) {
              return 0;
            }
        }
        rtn = parse_line(buf);
        if(rtn == EOL){
            printf("not a valid input\n");
            continue;
        }

        bool rdOut = false;
        bool rdIn = false;
        bool rdErr = false;
        bool rdDefined = false;
        bool background = false;
        bool myError = false;
        bool beginningOfCommand = true;
        struct Node *node = (struct Node*) calloc(1, sizeof(struct Node));
        struct Node *head = node;
        while(rtn != EOL){
            switch(rtn){
                case WORD:
                    if(beginningOfCommand == true){
                        node->command = strdup(lexeme);
                        beginningOfCommand = false;
                        rdIn = false;
                        rdOut = false;
                        rdErr = false;
                        rdDefined = true;
                    } else {
                        push(&node, lexeme);
                        rdDefined = true;
                    }
                    break;
                case ERROR_CHAR:
                    printf("error char\n" );
                    break;
                case SYSTEM_ERROR:
                    exit(42);
                default:
                    // if it is a pipe character
                    if (strcmp(tokens[rtn %96], "PIPE") == 0){
                        if(beginningOfCommand == true){
                            break;
                            myError = true;
                        }
                        struct Node *newNode = (struct Node*) calloc(1, sizeof(struct Node));
                        newNode->prev = node;

                        //assign the input file
                        if(node->inFile == NULL ){
                            newNode->inFile = strdup(node->command);
                        } else {
                            printf("Redirection error\n");
                        }

                        // assign the output file
                        if(node->outFile == NULL ){
                            // get and assign outFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->outFile = strdup(lexeme);
                                newNode->command = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            printf("Redirection error\n");
                        }
                        node = newNode;
                        beginningOfCommand = false;
                        rdIn = false;
                        rdOut = false;
                        rdErr = false;
                        rdDefined = true;
                    }
                    // if it is a semicolon character
                    else if(strcmp(tokens[rtn %96], "SEMICOLON") == 0){
                        beginningOfCommand = true;
                        rdIn = false;
                        rdOut = false;
                        rdErr = false;
                        rdDefined = false;
                    }
                    // if it is a > character
                    else if(strcmp(tokens[rtn %96], "REDIR_OUT") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->outFile == NULL){
                            // get and assign outFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->outFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect out
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdOut = true;
                        rdDefined = true;
                    }
                    // if it is a < character
                    else if(strcmp(tokens[rtn %96], "REDIR_IN") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->inFile == NULL){
                            // get and assign inFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->inFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect in
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdIn = true;
                        rdDefined = true;
                    }
                    // if it is the 2>1 character
                     else if(strcmp(tokens[rtn %96], "REDIR_ERR_OUT") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->errFile == NULL){
                            // get and assign errFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->errFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect err
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdErr = true;
                        rdDefined = true;
                    }
                    // if it is a >> character
                     else if(strcmp(tokens[rtn %96], "APPEND_OUT") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->outFile == NULL){
                            // get and assign outFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->outFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect out
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdOut = true;
                        rdDefined = true;
                    }
                    // if it is a 2>> character
                    else if(strcmp(tokens[rtn %96], "APPEND_ERR") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->errFile == NULL){
                            // get and assign errFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->errFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect err
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdErr = true;
                        rdDefined = true;
                    }
                    // if it is a 2> character
                    else if(strcmp(tokens[rtn %96], "REDIR_ERR") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(node->errFile == NULL){
                            // get and assign errFile of the node
                            rtn = parse_line(NULL);
                            if(rtn != EOL){
                                node->errFile = strdup(lexeme);
                            } else {
                                printf("Ambiguous redirection\n");
                                myError = true;
                                break;
                            }
                        } else {
                            //if there is more than one attempt to redirect err
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        rdErr = true;
                        rdDefined = true;
                    }
                    // if it is only an opening quote with no close
                    else if (strcmp(tokens[rtn %96], "QUOTE_ERROR") == 0){
                        printf("unmatched quote.\n");
                        myError = true;
                        break;
                    }
                    // if it is an & character
                    else if (strcmp(tokens[rtn %96], "AMP") == 0){
                        rtn = parse_line(NULL);
                        //printf("my values are: %d, %s\n", rtn, tokens[rtn%96]);
                        if(rtn == EOL){
                            node->background = true;
                        } else {
                            myError = true;
                            break;
                        }

                    }
                    //printf("%d: %s\n", rtn, tokens[rtn%96]);
            }
            if(myError == true){
                break;
            }
            //push(&node, lexeme);
            if(rtn != EOL){
                rtn = parse_line(NULL);
            }
            if(rtn == EOL){
                if(rdDefined == false){
                    printf("No file defined for redirection\n");
                    myError = true;
                    break;
                }
            }
        }
        if(myError == false){
            int count = 0;
            count = reverse(&head);
            Executer(head, count);
        }
        //printList(node->arg_list);

    }

}
