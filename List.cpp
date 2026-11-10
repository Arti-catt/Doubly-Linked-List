#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "colors.h"
#include <sys/wait.h>

#define LIST_DEBUG

#define FOLDER_NAME     "DumpPictures"
#define DUMP_FILE       "dump.html"
#define GRAPH_FILE_NAME "graph.dot"

#define POISON -666

const FILE *graph = fopen(GRAPH_FILE_NAME, "w");

#define GONDON(condition)                                                    \
    if (!(condition)) {                                                      \
        fprintf(stderr, "Function: %s, file: %s:%d\nGONDON %sHELP%s YOU\n",  \
            __FUNCTION__, __FILE__, __LINE__, RED, RES);                     \
        DEBUG_DUMP(lipstik)                                                  \
        abort();                                                             \
    }

#ifdef LIST_DEBUG
    #define DEBUG_DUMP(lipstik) Dump(lipstik, __FUNCTION__, __FILE__, __LINE__);
    FILE *dump_file = fopen(DUMP_FILE, "a");
#else
    #define DEBUG_DUMP(...)
    const FILE *dump_file = NULL;
#endif

#define ERR_TEXT(...)                                                                   \
    fprintf(stderr, "Function: %s, file: %s:%d\n", __FUNCTION__, __FILE__, __LINE__);   \
    fprintf(stderr, __VA_ARGS__);
#define LIST_VERIFY GONDON(ListVerify(lipstik) == NON_ERR)

#define LIST lipstik->list

#define LEN_COMMAND 100
#define NUMBER_ERRORS 11

enum Errors {
    NON_ERR          =   0,
    NON_CONNECT      =   1,
    INCORRECT_ENDS   =   2,
    NEGATIVE_TAIL    =   4,
    NEGATIVE_HEAD    =   8,
    FICTION_ELEM_ERR =  16,
    INCORRECT_INDEX  =  32,
    NON_VALID_INDEX  =  64,
    LIST_IS_FULL     = 128,
    FATAL_ERR        = 256,
    SYSTEM_ERR       = 512,
};

enum Parametres {
    INSERT = 0,
    ERASE  = 1,
};

struct ErrMessages {
    int number;
    const char *message;
};

// struct Dumpik {
//     ErrMessages *err_list;
//     int numerator_of_pictures;
//     char *command;
// };

struct Elem {
    int value;
    ssize_t next;
    ssize_t prev;
};

struct ListData {
    Elem *list;
    size_t capacity;
    size_t size;

    ssize_t tail;
    ssize_t head;

    ssize_t free;
    int err;
};

void ListInit(ListData *lipstik, size_t capacity);

void Insert(ListData *lipstik, ssize_t index, int number);     // index of element before
void Erase(ListData *lipstik, ssize_t index);                  // index of element for erase

int CorrectParametres(ListData *lipstik, ssize_t index, int mode);

void PrintList(ListData *lipstik);
void ListDestroy(ListData *lipstik);
int ListVerify(ListData *lipstik);

void DumpPicture(ListData *lipstik, int numerator_of_pictures);
void Dump(ListData *lipstik, const char *function, const char *file, int line);
void ErrorShredder(ListData *lipstik);
ErrMessages *StructsErrCreator();

void Old_Dump(ListData *lipstik);

int main()
{
    ListData lipstik = {};
    ListInit(&lipstik, 4);

    Insert(&lipstik, 0, 10);
    Insert(&lipstik, 1, 20);
    Insert(&lipstik, 2, 30);
    // Erase(&lipstik, 3);
    // Erase(&lipstik, 1);
    // Dump();
    PrintList(&lipstik);

    ListDestroy(&lipstik);
    return 0;
}

void ListInit (ListData *lipstik, size_t capacity)
{
    GONDON(lipstik)

    lipstik->list = (Elem *) calloc(capacity + 1, sizeof(Elem));
    GONDON(lipstik->list)

    lipstik->capacity = capacity + 1;       // + fiction element

    for (ssize_t i = 1; i < lipstik->capacity; i++) {
        LIST[i].value = POISON;
        LIST[i].next = i + 1;
        LIST[i].prev = -1;
    }

    LIST[lipstik->capacity - 1].next = -1;
    lipstik->free = 1;

    LIST[0].value = POISON;         // Fiction element with index [0]
    lipstik->tail = 0;
    lipstik->head = 0;

    lipstik->err = 0;
}

void Insert(ListData *lipstik, ssize_t index, int number)
{
    GONDON(lipstik)
    LIST_VERIFY
    if (lipstik->err != NON_ERR)
        return;
    if (CorrectParametres(lipstik, index, INSERT) != NON_ERR)
        return;

    ssize_t new_free = LIST[lipstik->free].next;

    if (index == lipstik->head)
        lipstik->head = lipstik->free;
    if (index == 0)
        lipstik->tail = lipstik->free;

    LIST[lipstik->free].value = number;
    LIST[lipstik->free].next = LIST[index].next;
    LIST[index].next = lipstik->free;
    LIST[lipstik->free].prev = index;
    LIST[LIST[lipstik->free].next].prev = lipstik->free;

    LIST[0].next = lipstik->tail;
    LIST[0].prev = lipstik->head;
    LIST[lipstik->head].next = 0;

    lipstik->free = new_free;
    lipstik->size++;
    LIST_VERIFY
}

void Erase(ListData *lipstik, ssize_t index)
{
    GONDON(lipstik)
    LIST_VERIFY
    if (lipstik->err != NON_ERR)
        return;
    if (CorrectParametres(lipstik, index, ERASE) != NON_ERR)
        return;

    if (index == lipstik->head)
        lipstik->head = LIST[index].prev;
    if (index == lipstik->tail)
        lipstik->tail = LIST[index].next;

    LIST[LIST[index].prev].next = LIST[index].next;
    LIST[LIST[index].next].prev = LIST[index].prev;
    
    LIST[index].value = POISON;
    LIST[index].prev = -1;
    LIST[lipstik->head].next = 0;

    LIST[index].next = lipstik->free;
    lipstik->free = index;

    lipstik->size--;
    LIST_VERIFY
}

int CorrectParametres(ListData *lipstik, ssize_t index, int mode)
{
    GONDON(lipstik)

    if (mode == INSERT && lipstik->free == -1) {
        // ERR_TEXT("LIST IS %sFULL%s\n", RED, RES)
        lipstik->err += LIST_IS_FULL;
    }
    if (mode == ERASE && index == 0) {
        // ERR_TEXT("CAN'T %sERASE%s FICTION [0] ELEMENT\n", RED, RES)
        lipstik-> err += NON_VALID_INDEX;
    }
    if (index < 0 && index >= lipstik->capacity) {
        // ERR_TEXT("INDEX <%zd> %sOUT OF RANGE%s\n", index, RED, RES)
        lipstik->err += INCORRECT_INDEX;
    }
    else if (LIST[index].prev == -1 && (lipstik->err & NON_VALID_INDEX) == 0) {
        // ERR_TEXT("ELEMENT WITH INDEX <%zd> %sDOESN'T EXIST%s\n", index, RED, RES)
        lipstik->err += NON_VALID_INDEX;
    }

    if (lipstik->err != NON_ERR)
        DEBUG_DUMP(lipstik)
    return lipstik->err;
}

void PrintList(ListData *lipstik) // base for verificator
{
    GONDON(lipstik)
    if (lipstik->err != NON_ERR)
        return;
    LIST_VERIFY

    ssize_t index = lipstik->tail;
    while (index != 0) {
        printf("%d\t", LIST[index].value);
        index = LIST[index].next;
    }
    printf("\n");
}

void ListDestroy(ListData *lipstik) // Cleaner of the structure
{
    GONDON(lipstik)
    LIST_VERIFY

    DEBUG_DUMP(lipstik)

    free(lipstik->list);
    char *cleaner = (char *) lipstik;
    for (int i = 0; i < (sizeof(ListData) / sizeof(char)); i++) {
        *(cleaner++) = 0;
    }
    lipstik = NULL;
}

int ListVerify(ListData *lipstik)
{
    GONDON(lipstik)
    ssize_t index = lipstik->tail;

    for(size_t i = 0; i < lipstik->size; i++) {
        if (LIST[LIST[index].prev].next == index) {
            index = LIST[index].next;
            continue;
        }
        lipstik->err += NON_CONNECT;
        break;
    }
    if (index != 0)
        lipstik->err += INCORRECT_ENDS;

    if (lipstik->tail < 0)
        lipstik->err += NEGATIVE_TAIL;
    if (lipstik->head < 0)
        lipstik->err += NEGATIVE_HEAD;
    if (LIST[0].value != POISON)
        lipstik->err += FICTION_ELEM_ERR;

    DEBUG_DUMP(lipstik)
    return lipstik->err;
}

void Dump(ListData *lipstik, const char *function, const char *file, int line)
{
    GONDON(lipstik)
    static int numerator_of_pictures = 0;
    numerator_of_pictures++;

    fprintf(dump_file,
        "<h1>Dumping...</h1>\n"
        "<p>\n"
        "<span style=\"color: green; font-weight: bold;\">Function: %s, file: %s, line: %d</span>\n",
        function, file, line
    );
    ErrorShredder(lipstik);
    DumpPicture(lipstik, numerator_of_pictures);
    fprintf(dump_file, "\n");
}

void ErrorShredder(ListData *lipstik)
{
    GONDON(lipstik)

    ErrMessages *err_list = StructsErrCreator();

    if (lipstik->err == NON_ERR) {
        fprintf(dump_file, "%s\n", err_list[0].message);
        return;
    }
    for (int i = 0; i < NUMBER_ERRORS; i++) {
        if (lipstik->err & err_list[i].number)
            fprintf(dump_file, "<span style=\"color: red;\">%s</span>\n", err_list[i].message);
    }
}

ErrMessages *StructsErrCreator()
{
    static ErrMessages err_list[NUMBER_ERRORS] = {
        {NON_ERR,           "Errcode = 0, non errors in program"                 },
        {NON_CONNECT,       "Incorrect connection between elements"              },
        {INCORRECT_ENDS,    "Incorrect ENDS of list (tail or head)"              },
        {NEGATIVE_TAIL,     "Negative tail"                                      },
        {NEGATIVE_HEAD,     "Negative head"                                      },
        {FICTION_ELEM_ERR,  "Value of the fiction element is incorrect"          },
        {INCORRECT_INDEX,   "Index as a parameter of the function is incorrect"  },
        {NON_VALID_INDEX,   "Element with this index is incorrect for operation" },
        {LIST_IS_FULL,      "Size of the list is critical"                       },
        {FATAL_ERR,         "Exotic error like problem in switch"                },
        {SYSTEM_ERR,        "Function system worked incorrect"                   },
    };
    return err_list;
}

void DumpPicture(ListData *lipstik, int numerator_of_pictures)
{
    GONDON(lipstik)

    static int open_check = mkdir(FOLDER_NAME, 0777);

    FILE *graph_file = fopen(GRAPH_FILE_NAME, "w");
    GONDON(graph_file)

    fprintf(graph_file,
        "digraph G {\n"
        "\trankdir = LR;\n"
        "\tnode [shape = Mrecord, color = red, style = filled, fillcolor = pink];\n"
        "\tedge [color = darkred];\n"
    );
    fprintf(graph_file, "\n");

    ssize_t index = 0;
    for (size_t i = 0; i < lipstik->size + 1; i++) {
        fprintf(graph_file, "\tnode%zd [label = \"{%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = pink];\n",
                index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        index = LIST[index].next;
    }
    fprintf(graph_file, "\n");

    index = 0;
    for (size_t i = 0; i < lipstik->size; i++) {
        fprintf(graph_file, "\tnode%zd-> node%zd;\n", index, LIST[index].next);
        index = LIST[index].next;
    }
    fprintf(graph_file, "\tnode%zd:s -> node%d:s;\n", LIST[0].prev, 0);
    fprintf(graph_file, "}");

    fclose(graph_file);

    char command[LEN_COMMAND] = "/usr/bin/dot -Tpng graph.dot -o DumpPictures/";
    char graph_name[LEN_COMMAND] = "graph";
    char numer[LEN_COMMAND] = "";
    snprintf(numer, sizeof(graph_name), "%d", numerator_of_pictures);
    strncat(graph_name, numer, sizeof(numer) / sizeof(char));
    strncat(graph_name, ".png", 4);
    strncat(command, graph_name, sizeof(graph_name) / sizeof(char));

    int status = system(command);
    if (status != 0)
        lipstik->err += SYSTEM_ERR;

    fprintf(dump_file,
        "\n"
        "<span style=\"font-weight: bold;\">__GRAPHIC_DUMP__</span>"
        "</p>"
        "<img src=\"/home/solo/Documents/GitHub/Doubly-Linked-List/DumpPictures/%s\">\n",
        graph_name
    );
}

////////////////////////////////////////////////////////////////////////

void Old_Dump(ListData *lipstik)
{
    GONDON(lipstik)

    printf("Struct List {\nTAIL = %zd;\nHEAD = %zd;\nFREE = %zd;\n}\n____LISTDUMP____\n\n",
            lipstik->tail, lipstik->head, lipstik->free);

    printf("%sINDEX%s:\n", MASLO, RES);
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        if (i == lipstik->tail || i == lipstik->head)
            printf("%s%6zd%s\t", ORANGE_MASLO, i, RES);
        else
            printf("%s%6zd%s\t", MASLO, i, RES);
    }
    printf("\n");
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        char end_ind[2] = {' ', ' '};
        if (i == lipstik->tail && i == lipstik->head) {
            end_ind[0] = 'T';
            end_ind[1] = 'H';
        }
        else if (i == lipstik->head)
            end_ind[1] = 'H';
        else if (i == lipstik->tail)
            end_ind[1] = 'T';
        printf("%s%6s%s\t", ORANGE_MASLO, end_ind, RES);
    }
    printf("\n");

    printf("%sVALUE%s:\n", BLUE_F, RES);
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        printf("%s%6d%s\t", BLUE, LIST[i].value, RES);
    }
    printf("\n\n");

    printf("%sNEXT%s:\n", GREEN_F, RES);
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        printf("%s%6zd%s\t", GREEN, LIST[i].next, RES);
    }
    printf("\n\n");

    printf("%sPREV%s:\n", RED_F, RES);
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        printf("%s%6zd%s\t", RED, LIST[i].prev, RES);
    }
    printf("\n\n");
}