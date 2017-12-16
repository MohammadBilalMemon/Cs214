/*
Made by:
Mohammad Bilal Memon && Muhammed Rizwan Khan 

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>




// made a struct to hold multiple variables together to use for the Linked list data structure. 
struct Node{    
    char * str;
    struct Node * next;
};

struct Node * head = NULL;

// method to compare 2 char strings will be applied in the linked list 
int compare(char * string1, char * string2){ 
    int result = strcmp(string1, string2);
    if(result < 0)
        return 1;
    else
        return 2;
}
// insert/SORT method of the token/word inside the linked list. It allocates for memory and traverses through the Linked list. 
void insert(char * token){
    
    struct Node * p = NULL;	// Struct node is a ptr
    if(head->str == NULL){
        head->str = token;
        head->next = NULL;
        return;
    }
    struct Node * prev = NULL;
    prev = head;
    p = head;
    struct Node * temp = NULL;
    temp = malloc(sizeof(struct Node));
    if(temp == NULL){
        printf("malloc did not allocate memory");
    }
    temp->str = token;
    int result = compare(p->str, token);
    if(result == 2){
        temp->next = head;
        head = temp;
        return;
        
        
    }
    
    p = p->next;
    while(p != NULL){
        int result = compare(p->str, token);
        if(result == 2){
            temp->next = p;	// if the condition is true then p goes to temp next
            prev->next = temp;
            return;
        }
        prev = p;
        p = p->next;
        
    }
    prev->next = temp;
    temp->next = NULL;
    return;
    
}

int main(int argc, char *argv[]) // start of main method. 

{
    int len = (int)strlen(argv[1]); // length of argument/string
    if(argc != 2){ // if user inputs wrong number of input, print the following
        printf("Invalid number of arguments\n");
        return 0;
    }
    char *d = malloc (strlen(argv[1])); // allocate memory here
    strcpy(d,argv[1]); // puts the argument into d
    
    head = malloc(sizeof(struct Node));
    if(head == NULL){
        printf("malloc did not allocate memory\n");
        return 0;
    }
    char * ptr = d;
   
    int startOfToken = 0;
    int endOfToken = 0;
    while(startOfToken <= len) // while we dont fall of the string. 
    {
        
        while(isalpha(ptr[startOfToken])) // using isalpha on the start of character
        {
            startOfToken++;
        }
        
        if(!isalpha(ptr[startOfToken])){ 
            char * token = NULL;
            token = calloc(1+(startOfToken-endOfToken),sizeof(char));
            if(token == NULL){
                printf("calloc did not give memory\n");
                return 0;
            }
            memcpy(token, ptr + endOfToken,startOfToken-endOfToken);
            token[startOfToken-endOfToken] = '\0'; 
			//endoOfToken,startOfToken and the difference gets you position of the delimeter
            insert(token);
        }
        
        startOfToken++;
        endOfToken = startOfToken;
        
    }
    struct Node * temp = head;
int i = 0;
    while(temp != NULL){ // traverse the linked list. 
if (strlen(argv[1]) != 0) { 
        printf("%s\n",temp->str);
        temp = temp->next;
}
else if (i < temp->str - 1) { // prints nothing when just empty string.
break;
}
    }
    return 0;
}