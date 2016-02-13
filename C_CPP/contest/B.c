#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define STACK_SIZE 50
#define SET_SIZE 10
#define ELEM_SIZE 10

#define NOERR 0  /* no error */
#define EMTSTK 1 /* empty stack */
#define ALLORR 2 /* allocation error */
#define SYNERR 3 /* syntax error */

typedef enum {
    DEF,  // default
    LQU, // left quote, '"'
    RQU, // right quote, '"'
    COMM, // comma, ','
    LSB,  // left square bracket, '['
    RSB,  // right square bracket, ']'
    LRB,  // left round bracket, '('
    RRB,  // right round bracket, ')'
    UNI,  // union, 'U'
    INTS, // intersection, '^'
    DIFF, // difference, '\'
    ERR   // error
} states;

int isSetCleared;

// set of strings
typedef struct Set {
    char **data;
    int size;
    int error; // alloc's error
    int *cleared;
} Set;

void initSet(Set *s);
void destrSet(Set *s);


// stack for sets
typedef struct SetStack {
    Set *data;
    int size;
    int error;
} SetStack;

void initSetStack(SetStack *s);
Set popSetStack(SetStack *s);
void pushSetStack(SetStack *s, Set d);
void destrSetStack(SetStack *s);

// stack for chars
typedef struct CharStack {
    char *data;
    int size;
    int error;
} CharStack;

void initCharStack(CharStack *s);
char popCharStack(CharStack *s);
void pushCharStack(CharStack *s, char d);
void destrCharStack(CharStack *s);


void getSet(Set *s);
Set intersectSets(Set *s1, Set *s2);
Set unionSets(Set *s1, Set *s2);
Set diffSets(Set *s1, Set *s2);
void compute(SetStack *ss, CharStack *cs, int tillLRB);

void throwErr(SetStack *ss, CharStack *cs, Set *set) {
    if (isSetCleared == 0)
        destrSet(set);
    destrSetStack(ss);
    destrCharStack(cs);
    printf("[error]\n");
    exit(0);
}

int compare_function(const void *a, const void *b) {
    return strcmp(*((char **) a), *((char **) b));
}

int main()
{
    Set set;
    set.data = NULL;
    isSetCleared = 0;

    CharStack cs;
    initCharStack(&cs);

    SetStack ss;
    initSetStack(&ss);

    char c = 0, prevCh = 0;

    while ((c = getchar()) != EOF) {
        if (c == '\n')
            break;

        switch (c) {
            case '[':
                if (prevCh != 0 && prevCh != '(' && prevCh != 'U' && prevCh != '^' && prevCh != '\\') {
                    throwErr(&ss, &cs, &set);
                }

                initSet(&set);
                isSetCleared = 0;
                getSet(&set);
                if (set.error != NOERR) {
                    throwErr(&ss, &cs, &set);
                }
                if (prevCh == '^') {
                    Set set2 = popSetStack(&ss);
                    if (ss.error != NOERR)
                        throwErr(&ss, &cs, &set);

                    Set tmp = intersectSets(&set, &set2);
                    isSetCleared = 1;
                    if (tmp.error != NOERR) {
                        destrSet(&tmp);
                        throwErr(&ss, &cs, &set);
                    }
                    pushSetStack(&ss, tmp);
                } else {
                    pushSetStack(&ss, set);
                    isSetCleared = 1;
                }

                break;

            case 'U':
            case '\\':
            case '^':
                // actually, if prevCh == '[', then prevCh == ']' (look getSet)
                if (prevCh != '[' && prevCh != ')') {
                    throwErr(&ss, &cs, &set);
                }
                if (c != '^')
                    pushCharStack(&cs, c);
                break;

            case '(':
                if (prevCh != 'U' && prevCh != '\\' && prevCh != '^' && prevCh != 0) {
                    throwErr(&ss, &cs, &set);
                }
                pushCharStack(&cs, c);
                break;

            case ')':
                if (prevCh != '[') { // same to case 'U'
                    throwErr(&ss, &cs, &set);
                }
                compute(&ss, &cs, 1);
                if (ss.error != NOERR || cs.error != NOERR) {
                    throwErr(&ss, &cs, &set);
                }
                break;

            default:
                throwErr(&ss, &cs, &set);
                break;
        }
        prevCh = c;
    }

    compute(&ss, &cs, 0);
    if (ss.error != NOERR || cs.error != NOERR) {
        throwErr(&ss, &cs, &set);
    }

    if (ss.size != 1 || cs.size != 0) {
        throwErr(&ss, &cs, &set);
    }
    set = popSetStack(&ss);

    qsort(set.data, set.size, sizeof(char *), compare_function);

    if (set.size == 0) {
        printf("[]");
    } else {
        printf("[\"%s", set.data[0]);
        for (int i = 1; i < set.size; ++i) {
            printf("\",\"%s", set.data[i]);
        }
        printf("\"]");
    }

    

    destrSet(&set);
    destrSetStack(&ss);
    destrCharStack(&cs);

    return 0;
}

/**
    Set funcs
*/
void initSet(Set *s)
{
    s->data = malloc(SET_SIZE * sizeof(char *));
    if (s->data == NULL) {
        s->error = ALLORR;
    }
    s->size = 0;
    s->error = NOERR;
}

void destrSet(Set *s)
{
    if (s->data == NULL) {
        return;
    }

    for (int i = 0; i < s->size; ++i) {
        free(s->data[i]);
        s->data[i] = NULL;
    }
    free(s->data);
    s->data = NULL;
}


/**
    Set Stack funcs
*/
void initSetStack(SetStack *s)
{
    s->size = 0;
    s->data = (Set *) malloc(STACK_SIZE * sizeof(Set));
    if (s->data == NULL) {
        s->error = ALLORR;
        return;
    } else {
        s->error = NOERR;
    }
}

Set popSetStack(SetStack *s)
{
    if (s->size == 0) {
        s->error = EMTSTK;
        return s->data[0];
    }

    return s->data[--s->size];;
}

void pushSetStack(SetStack *s, Set d)
{
    if (s->size != 0 && s->size % STACK_SIZE == 0) {
        Set *tmp = (Set *) realloc(s->data, (s->size + STACK_SIZE) * sizeof(Set));
        if (tmp == NULL) {
            s->error = ALLORR;
            return;
        }
        s->data = tmp;
    }
    s->data[s->size++] = d;
}

void destrSetStack(SetStack *s)
{
    if (s->data == NULL) {
        return;
    }

    for (int i = 0; i < s->size; ++i) {
        destrSet(&s->data[i]);
    }
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->error = NOERR;
}


/**
    Char Stack funcs
*/
void initCharStack(CharStack *s)
{
    s->size = 0;
    s->data = (char *) malloc(STACK_SIZE * sizeof(char));
    if (s->data == NULL) {
        s->error = ALLORR;
        return;
    } else {
        s->error = NOERR;
    }
}

char popCharStack(CharStack *s)
{
    if (s->size == 0) {
        s->error = EMTSTK;
        return s->data[0];
    }
    return s->data[--s->size];
}

void pushCharStack(CharStack *s, char d)
{
    if (s->size != 0 && s->size % STACK_SIZE == 0) {
        char *tmp = (char *) realloc(s->data, (s->size + STACK_SIZE) * sizeof(char));
        if (tmp == NULL) {
            s->error = ALLORR;
            return;
        }
        s->data = tmp;
    }
    s->data[s->size++] = d;
}

void destrCharStack(CharStack *s)
{
    if (s->data == NULL)
        return;

    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->error = NOERR;
}


/**
    Other funcs
*/
void getSet(Set *s)
{
    char *buf = NULL;
    char c = 0;
    int i = 0;
    states state = DEF;
    int errtype = NOERR;

    while (7) {
        c = getchar();
        switch (state) {
            case DEF:
                if (c == '"') {
                    state = LQU;

                } else if (c == ']') {
                    return;

                } else {
                    state = ERR;
                    errtype = SYNERR;
                }
                break;

            case LQU:
                if (isalnum(c) || c == ' ') {
                    if (i % ELEM_SIZE == 0) {
                        char *tmp = realloc(buf, (i + ELEM_SIZE + 1));
                        if (tmp == NULL) {
                            state = ERR;
                            errtype = ALLORR;
                        }
                        buf = tmp;
                    }
                    buf[i++] = c;

                } else if (c == '"') {
                    state = RQU;

                    buf[i] = '\0';

                    int isIn = 0;

                    // Check is buf in set
                    for (int i = 0; i < s->size; ++i) {
                        if (strcmp(s->data[i], buf) == 0) {
                            isIn = 1;
                        }
                    }

                    if (isIn == 0) {
                        if (s->size != 0 && s->size % SET_SIZE == 0) {
                            char **tmp = realloc(s->data, (s->size + SET_SIZE) * sizeof(char *));
                            if (tmp == NULL) {
                                state = ERR;
                                errtype = ALLORR;
                            }
                            s->data = tmp;
                        }
                        s->data[s->size++] = buf;       
                    } else {
                        free(buf);
                        buf = NULL;
                    }

                    buf = NULL;
                    i = 0;

                } else {
                    state = ERR;
                    errtype = SYNERR;
                }

                break;

            case RQU:
                if (c == ',')
                    state = DEF;

                else if (c == ']')
                    return;

                else {
                    state = ERR;
                    errtype = SYNERR;
                }

                break;

            case ERR:
                free(buf);
                s->error = errtype;
                return;

            default:
                break;
        }
    }
}

Set intersectSets(Set *s1, Set *s2)
{

    Set ret;
    initSet(&ret);
    free(ret.data);
    int max = s1->size > s2->size ? s2->size : s1->size;
    ret.data = malloc((max / SET_SIZE + 1) * SET_SIZE * sizeof(char *));

    for (int i = 0; i < s1->size; ++i) {
        for (int j = 0; j < s2->size; ++j) {
            if (strcmp(s1->data[i], s2->data[j]) == 0) {
                ret.data[ret.size] = malloc(strlen(s1->data[i]) + 1);
                if (ret.data[ret.size] == NULL) {
                    destrSet(&ret);
                    ret.error = ALLORR;
                    destrSet(s1);
                    destrSet(s2);
                    return ret;
                }
                strcpy(ret.data[ret.size++], s1->data[i]);
                break;
            }
        }
    }
    destrSet(s1);
    destrSet(s2);

    return ret;
}

Set unionSets(Set *s1, Set *s2)
{

    for (int j = 0; j < s2->size; ++j) {

        int isIn = 0;

        for (int i = 0; i < s1->size; ++i) {
            if (strcmp(s1->data[i], s2->data[j]) == 0) {
                isIn = 1;
            }
        }

        if (isIn == 0) {
            if (s1->size != 0 && s1->size % SET_SIZE == 0) {
                char **tmp = realloc(s1->data, (s1->size + SET_SIZE) * sizeof(char *));
                if (tmp == NULL) {
                    destrSet(s1);
                    s1->error = ALLORR;
                    destrSet(s2);
                    return *s1;
                }
                s1->data = tmp;
            }
            s1->data[s1->size] = malloc(strlen(s2->data[j]) + 1);
            if (s1->data[s1->size] == NULL) {
                destrSet(s1);
                s1->error = ALLORR;
                destrSet(s2);
                return *s1;
            }
            strcpy(s1->data[s1->size++], s2->data[j]);
        }
        
    }
    destrSet(s2);

    return *s1;
}

Set diffSets(Set *s1, Set *s2)
{
    Set ret;
    initSet(&ret);
    free(ret.data);
    int max = s1->size > s2->size ? s2->size : s1->size;
    ret.data = malloc((max / SET_SIZE + 1) * SET_SIZE * sizeof(char *));

    for (int i = 0; i < s1->size; ++i) {
        int isIn = 0;
        for (int j = 0; j < s2->size; ++j) {
            if (strcmp(s1->data[i], s2->data[j]) == 0) {
                isIn = 1;
            }
        }
        if (isIn == 0) {
            ret.data[ret.size] = malloc(strlen(s1->data[i]) + 1);
            if (ret.data[ret.size] == NULL) {
                destrSet(&ret);
                ret.error = ALLORR;
                destrSet(s1);
                destrSet(s2);
                return ret;
            }
            strcpy(ret.data[ret.size++], s1->data[i]);
            break;
        }
    }
    destrSet(s1);
    destrSet(s2);

    return ret;
}

void compute(SetStack *ss, CharStack *cs, int tillLRB)
{
    char c = 0;
    Set set1, set2, tmp;

    while (cs->size != 0) {
        c = popCharStack(cs);

        if (c == '(' && tillLRB == 1)
            return;

        if (cs->error != NOERR)
            return;

        switch (c) {
            case 'U':
                if (ss->size < 2) {
                    ss->error = EMTSTK;
                    return;
                }
                set1 = popSetStack(ss);
                set2 = popSetStack(ss);

                tmp = unionSets(&set1, &set2);
                if (tmp.error != NOERR) {
                    destrSet(&tmp);
                    ss->error = tmp.error;
                    return;
                }
                pushSetStack(ss, tmp);
                break;

            case '\\':
                if (ss->size < 2) {
                    ss->error = EMTSTK;
                    return;
                }
                set1 = popSetStack(ss);
                set2 = popSetStack(ss);
                
                tmp = diffSets(&set2, &set1);
                if (tmp.error != NOERR) {
                    destrSet(&tmp);
                    ss->error = tmp.error;
                    return;
                }
                pushSetStack(ss, tmp);
                break;

            default:
                ss->error = SYNERR;
                cs->error = SYNERR;
                return;
        }
    }
    return;
}