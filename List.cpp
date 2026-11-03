#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define LIST_LEN 15
#define POISON   666

#define RED   "\033[31m"
#define BLUE  "\033[35m"
#define GREEN "\033[1;32m"
#define ORANGE "\033[1;34m"
#define MASLO "\033[1;37m"
#define RED_F   "\033[41m"
#define BLUE_F  "\033[45m"
#define GREEN_F "\033[42m"
#define MASLO "\033[1;37m"
#define RES   "\033[0m"

#define GONDON(condition)                                                    \
    if (!(condition)) {                                                      \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON %sHELP%s YOU\n",  \
            __FILE__, __LINE__, __FUNCTION__, RED, RES);                     \
        abort();                                                             \
    }

struct Elem {
    int value;
    ssize_t next;
    ssize_t prev;
};

struct List {
    Elem list[LIST_LEN];
    ssize_t tail;
    ssize_t head;
    ssize_t free;
};

void Append(List *inventory, ssize_t index, int number);     // index of element before
void Pop(List *inventory, ssize_t index);        // index of element for pop
void PrintList(List *inventory);

void Dump(List *inventory);

int main()
{
    List inventory = {};
    inventory.list[0].value = POISON;
    for (ssize_t i = 0; i < LIST_LEN; i++)
        inventory.list[i].next = -1;
    inventory.list[0].next = 0;

    for (ssize_t i = 1; i < 10; i++) {
        inventory.list[i].value = i * 10;
        inventory.list[i].next = i + 1;
    }
    inventory.free = 10;
    inventory.tail = 1;
    inventory.head = 9;
    inventory.list[inventory.head].next = 0;

    Dump(&inventory);
    PrintList(&inventory);
    
    Append(&inventory, 3, 35);

    Dump(&inventory);
    PrintList(&inventory);
    return 0;
}

void Append(List *inventory, ssize_t index, int number)
{
    GONDON(inventory)

    inventory->list[inventory->free].value = number;
    inventory->list[inventory->free].next = inventory->list[index].next;
    inventory->list[index].next = inventory->free;
}

void PrintList(List *inventory) // base for verifycator
{
    GONDON(inventory)
    ssize_t index = inventory->tail;
    while (index != 0) {
        printf("%d\t", inventory->list[index].value);
        index = inventory->list[index].next;
    }
    printf("\n");
}

void Dump(List *inventory)
{
    GONDON(inventory)
    printf("Struct List {\nTAIL = %zd;\nHEAD = %zd;\nFREE = %zd;\n}\n____LISTDUMP____\n\n",
            inventory->tail, inventory->head, inventory->free);
    printf("%sINDEX%s:\n", MASLO, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        if (i == inventory->tail || i == inventory->head)
            printf("%s%6zd%s\t", ORANGE, i, RES);
        else
            printf("%s%6zd%s\t", MASLO, i, RES);
    }
    printf("\n");
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        if (i == inventory->tail)
            printf("%s%6c%s\t", ORANGE, 'T', RES);
        else if (i == inventory->head)
            printf("%s%6c%s\t", ORANGE, 'H', RES);
        else
            printf("%6c\t", ' ');
    }
    printf("\n\n");
    printf("%sVALUE%s:\n", BLUE_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6d%s\t", BLUE, inventory->list[i].value, RES);
    }
    printf("\n\n");
    printf("%sNEXT%s:\n", GREEN_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6zd%s\t", GREEN, inventory->list[i].next, RES);
    }
    printf("\n\n");
    printf("%sPREV%s:\n", RED_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6zd%s\t", RED, inventory->list[i].prev, RES);
    }
    printf("\n\n");
}