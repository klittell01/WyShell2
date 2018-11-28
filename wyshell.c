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
    // got to figure out how to access the begining of the arg_list to keep
    // putting arguments into the list
    while(headRef->arg_list != NULL){

    }
  struct Word* newNode =
          (struct Word*) calloc(1, sizeof(struct Word));
  newNode->command  = strdup(data);
  newNode->arg_list
  newNode->next = (*headRef);
  (*headRef)    = newNode;
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

/////////////////////////////////////////////////////////
//// send the node and the data to the push function ////
/////////////////////////////////////////////////////////
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

        // will be implementing in next part
        //struct Node *node;
        //node = NULL;

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
                        struct Word *word;
                        push(&node, lexeme);
                        word->command = strdup(lexeme);
                        printf(" --: %s\n", lexeme);
                        rdDefined = true;
                    }
                    break;
                case ERROR_CHAR:
                    break;
                default:
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
                        if(rdOut == true){
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        printf(" >\n");
                        rdOut = true;
                        rdDefined = false;
                    }
                    // if it is a < character
                    else if(strcmp(tokens[rtn %96], "REDIR_IN") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(rdIn == true){
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        printf(" <\n");
                        rdIn = true;
                        rdDefined = false;
                    }
                    // if it is the 2>1 character
                     else if(strcmp(tokens[rtn %96], "REDIR_ERR") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(rdErr == true){
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        printf(" 2>1\n");
                        rdErr = true;
                        rdDefined = false;
                    }
                    // if it is a >> character
                     else if(strcmp(tokens[rtn %96], "APPEND_OUT") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(rdOut == true){
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        printf(" >>\n");
                        rdOut = true;
                        rdDefined = false;
                    }
                    // if it is a 2>> character
                    else if(strcmp(tokens[rtn %96], "APPEND_ERR") == 0){
                        if(beginningOfCommand == true){
                            myError = true;
                            printf("Ambiguous redirection\n");
                            break;
                        }
                        if(rdErr == true){
                            printf("Ambiguous redirection\n");
                            myError = true;
                            break;
                        }
                        printf(" 2>>\n");
                        rdErr = true;
                        rdDefined = false;
                    }
                    // if it is only an opening quote with no close
                    else if (strcmp(tokens[rtn %96], "QUOTE_ERROR") == 0){
                        myError = true;
                        break;
                    }
                    // if it is an & character
                     else if (strcmp(tokens[rtn %96], "AMP") == 0){
                        background = true;
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

            printf(" --: EOL\n");
        }
        //reverse(&node->arg_list);
        printList(node->arg_list);

    }

}
