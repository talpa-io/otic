#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 13
#define MAX_HASH 10
#define M 31

struct plz
{
    char ort[MAX];
    unsigned int postleit;
    struct plz* next;
};

struct plz* hash_tabelle[MAX_HASH];

int hash_funktion(char* string)
{
    unsigned int hash_addresse = 0;
    unsigned char* pointer = (unsigned char*)string;
    while(*pointer)
    {
        hash_addresse = M * hash_addresse + *pointer;
        pointer++;
    }
    return hash_addresse % MAX_HASH;
}

struct plz* insert(char* o, unsigned int p)
{
    struct plz* pointer;
    int hash_addresse = hash_funktion(o);
    pointer = hash_tabelle[hash_addresse];
    while (pointer != NULL)
    {
        if (strcmp(o, pointer->ort) == 0)
            if (pointer->postleit == p) {
                return pointer;
            }
        pointer = pointer->next;
    }
    pointer = malloc(sizeof(struct plz));
    if (pointer == NULL){
        return NULL;
    }
    strcpy(pointer->ort, o);
    pointer->postleit = p;
    pointer->next = hash_tabelle[hash_addresse];
    hash_tabelle[hash_addresse] = pointer;
    return pointer;
}

void search_in_hash(char* o)
{
    struct plz* pointer;
    int hash_addresse = hash_funktion(o);
    pointer = hash_tabelle[hash_addresse];
    int counter = 0;
    while (pointer) {
        printf("Counter: %d\n", counter);
        if (strcmp(pointer->ort, o) == 0){
            printf("PLZ fuer %s ist %d\n", o, pointer->postleit);
            return;
        }
        pointer = pointer->next;
    }
    printf("Nix gefunden!");
}


int main() {
    insert("Friedberg", 86316);
    insert("Augsburg", 83136);
    insert("Stuttgart", 71345);

    search_in_hash("Augsburg");
    search_in_hash("Ausgburg");
    search_in_hash("Friedberg");


    return 0;
}