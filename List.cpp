#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "colors.h"

#define LIST_DEBUG

#define LIST_LEN   5
#define POISON   -666

#ifdef LIST_DEBUG
    #define DEBUG_DUMP(list, data) Dump(list, data);
#else
    #define DEBUG_DUMP(...)
#endif

#define GONDON(condition)                                                    \
    if (!(condition)) {                                                      \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON %sHELP%s YOU\n",  \
            __FILE__, __LINE__, __FUNCTION__, RED, RES);                     \
        abort();                                                             \
    }
#define ERR_TEXT(...)                                                                   \
    fprintf(stderr, "Function: %s, file: %s:%d\n", __FUNCTION__, __FILE__, __LINE__);   \
    fprintf(stderr, __VA_ARGS__);
#define LIST_VERIFY GONDON(ListVerify(list, data) != FATAL_ERR)

#define LIST_DATA list, &data

enum Errors {
    NON_ERR   = 0,
    FATAL_ERR = 1,
};

struct Elem {
    int value;
    ssize_t next;
    ssize_t prev;
};

struct ListData {
    size_t size;
    ssize_t tail;
    ssize_t head;
    ssize_t free;
    ssize_t last_free;
};

void ListInit(Elem *list, ListData *data);

void Insert(Elem *list, ListData *data, ssize_t index, int number);     // index of element before
void Erase(Elem *list, ListData *data, ssize_t index);                  // index of element for erase

bool CorrectIndex(Elem *list, ListData *data, ssize_t index);

void PrintList(Elem *list, ListData *data);
int ListVerify(Elem *list, ListData *data);

void Dump(Elem *list, ListData *data);

int main()
{
    Elem list[LIST_LEN] = {};
    ListData data = {};
    ListInit(LIST_DATA);

    Insert(LIST_DATA, 0, 10);
    Insert(LIST_DATA, 1, 20);
    Insert(LIST_DATA, 2, 30);
    Insert(LIST_DATA, 3, 40);
    Erase(LIST_DATA, 4);
    Erase(LIST_DATA, 2);

    Dump(LIST_DATA);
    PrintList(LIST_DATA);
    return 0;
}

void ListInit (Elem *list, ListData *data)
{
    GONDON(list)
    GONDON(data)

    for (ssize_t i = 1; i < LIST_LEN; i++) {
        list[i].value = POISON;
        list[i].next = i + 1;
        list[i].prev = -1;
    }

    data->last_free = LIST_LEN - 1;
    list[data->last_free].next = -1;
    data->free = 1;

    list[0].value = POISON;         // Fiction element with index [0]
    data->tail = 0;
    data->head = 0;
}

void Insert(Elem *list, ListData *data, ssize_t index, int number)
{
    GONDON(list)
    GONDON(data)

    LIST_VERIFY

    if (!CorrectIndex(list, data, index))
        return;
    if (data->free == -1) {
        ERR_TEXT("LIST IS %sFULL%s\n", RED, RES)
        DEBUG_DUMP(list, data);
        return;
    }
    ssize_t new_free = list[data->free].next;

    if (index == data->head) {
        data->head = data->free;
    }
    if (index == 0) {
        data->tail = data->free;
    }
    list[data->free].value = number;
    list[data->free].next = list[index].next;
    list[index].next = data->free;

    list[0].next = data->tail;
    list[0].prev = data->head;
    list[data->head].next = 0;
    list[data->free].prev = index;

    data->free = new_free;
    data->size++;
    LIST_VERIFY
}

void Erase(Elem *list, ListData *data, ssize_t index)
{
    GONDON(list)
    GONDON(data)

    LIST_VERIFY

    if (!CorrectIndex(list, data, index))
        return;
    if (index == 0) {
        ERR_TEXT("CAN'T %sERASE%s FICTION [0] ELEMENT\n", RED, RES)
        DEBUG_DUMP(list, data);
        return;
    }

    if (index == data->head) {
        data->head = list[index].prev;
    }
    if (index == data->tail) {
        data->tail = list[index].next;
    }
    list[list[index].prev].next = list[index].next;
    list[list[index].next].prev = list[index].prev;
    
    list[index].value = POISON;
    list[index].prev = -1;
    list[data->head].next = 0;

    list[data->last_free].next = index;
    data->last_free = index;
    list[data->last_free].next = -1;
    if (data->free == -1)
        data->free = data->last_free;

    data->size--;
    LIST_VERIFY
}

bool CorrectIndex(Elem *list, ListData *data, ssize_t index)
{
    GONDON(list)
    GONDON(data)

    bool answer = true;
    if (index < 0 && index >= LIST_LEN) {
        ERR_TEXT("INDEX <%zd> %sOUT OF RANGE%s\n", index, RED, RES)
        answer = false;
    }
    if (list[index].prev == -1) {
        ERR_TEXT("ELEMENT WITH INDEX <%zd> %sDOESN'T EXIST%s\n", index, RED, RES)
        answer = false;
    }
    if (!answer)
        DEBUG_DUMP(list, data);
    return answer;
}

void PrintList(Elem *list, ListData *data) // base for verificator
{
    GONDON(list)
    GONDON(data)

    ssize_t index = data->tail;
    while (index != 0) {
        printf("%d\t", list[index].value);
        index = list[index].next;
    }
    printf("\n");
}

int ListVerify(Elem *list, ListData *data)
{
    GONDON(list)
    GONDON(data)

    int err = NON_ERR;
    ssize_t index = data->tail;

    for(ssize_t i = 0; i < data->size; i++) {
        if (index != 0) {
            index = list[index].next;
            continue;
        }
        break;
    }
    if (index != 0) {
        DEBUG_DUMP(list, data);
        ERR_TEXT("%sERROR%s OF VERIFICATION\n", RED, RES)
        err = FATAL_ERR;
    }

    return err;
}

void Dump(Elem *list, ListData *data)
{
    GONDON(list)
    GONDON(data)

    printf("Struct List {\nTAIL = %zd;\nHEAD = %zd;\nFREE = %zd;\n}\n____LISTDUMP____\n\n",
            data->tail, data->head, data->free);

    printf("%sINDEX%s:\n", MASLO, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        if (i == data->tail || i == data->head)
            printf("%s%6zd%s\t", ORANGE_MASLO, i, RES);
        else
            printf("%s%6zd%s\t", MASLO, i, RES);
    }
    printf("\n");
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        char end_ind[2] = {' ', ' '};
        if (i == data->tail && i == data->head) {
            end_ind[0] = 'T';
            end_ind[1] = 'H';
        }
        else if (i == data->head)
            end_ind[1] = 'H';
        else if (i == data->tail)
            end_ind[1] = 'T';
        printf("%s%6s%s\t", ORANGE_MASLO, end_ind, RES);
    }
    printf("\n");

    printf("%sVALUE%s:\n", BLUE_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6d%s\t", BLUE, list[i].value, RES);
    }
    printf("\n\n");

    printf("%sNEXT%s:\n", GREEN_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6zd%s\t", GREEN, list[i].next, RES);
    }
    printf("\n\n");

    printf("%sPREV%s:\n", RED_F, RES);
    for (ssize_t i = 0; i < LIST_LEN; i++) {
        printf("%s%6zd%s\t", RED, list[i].prev, RES);
    }
    printf("\n\n");
}