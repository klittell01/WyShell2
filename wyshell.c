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
        int num;
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
static void reverse(struct Word** headRef)
{
  struct Word* prev   = NULL;
  struct Word* current = *headRef;
  struct Word* next = NULL;
  while (current != NULL)
  {
      // Store next
      next  = current->next;

      // Reverse current node's pointer
      current->next = prev;

      // Move pointers one position ahead.
      prev = current;
      current = next;
  }
  *headRef = prev;
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

void Executer(struct Node * node){
    printf("Command is: %s\n", node->command);
    struct Word *tmp = node->arg_list;
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

//////////////////////////////////////////////////////////////////////////////////
//// getting a weird QUOTE_ERROR return when i use an "&" without a ";" after ////
//////////////////////////////////////////////////////////////////////////////////

//TODO: get forking and execvp-ing working


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

        while(rtn != EOL){
            switch(rtn){
                case WORD:
                    if(beginningOfCommand == true){
                        node->command = strdup(lexeme);
                        printf(":--: %s\n", lexeme);
                        beginningOfCommand = false;
                        rdIn = false;
                        rdOut = false;
                        rdErr = false;
                        rdDefined = true;
                    } else {
                        push(&node, lexeme);
                        printf(" --: %s\n", lexeme);
                        rdDefined = true;
                    }
                    break;
                case ERROR_CHAR:
                    printf("Syntax error\n" );
                    break;
                case SYSTEM_ERROR:
                    exit(42);
                default:
                    printf("my rd is: %s\n", tokens[rtn %96]);
                    // if it is a pipe character
                    if (strcmp(tokens[rtn %96], "PIPE") == 0){
                        if(beginningOfCommand == true){
                            break;
                            myError = true;
                        }
                        printf(" |\n");
                        beginningOfCommand = true;
                        rdIn = false;
                        rdOut = false;
                        rdErr = false;
                        rdDefined = false;
                    }
                    // if it is a semicolon character
                    else if(strcmp(tokens[rtn %96], "SEMICOLON") == 0){
                        printf(" ;\n");
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
                        printf("Syntax error\n" );
                        myError = true;
                        break;
                    }
                    // if it is an & character
                    else if (strcmp(tokens[rtn %96], "AMP") == 0){
                        rtn = parse_line(NULL);
                        printf("my values are: %d, %s\n", rtn, tokens[rtn%96]);
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
            rtn = parse_line(NULL);
            if(rtn == EOL){
                if(rdDefined == false){
                    printf("No file defined for redirection\n");
                    myError = true;
                    break;
                }
            }
        }
        if(myError == false){
            reverse(&node->arg_list);
            Executer(node);
        }
        //printList(node->arg_list);

    }

}
