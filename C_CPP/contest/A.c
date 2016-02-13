#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char **filter(char **s, int lines) // filters lines on condition of problem
{
    for (int i = 0; i < lines; i++) {
        if (strncmp("Date:", s[i], 5) != 0 &&
            strncmp("From:", s[i], 5) != 0 &&
            strncmp("To:", s[i], 3) != 0 &&
            strncmp("Subject:", s[i], 8) != 0) {
            free(s[i]);
            s[i] = NULL;
        }
    }
    for (int i = 0; i < lines; i++) { // replace all none Null lines up
        if (s[i] == NULL) {
            for (int j = i + 1; j < lines; j++) {
                if (s[j] != NULL) {
                    s[i] = s[j];
                    s[j] = NULL;
                    break;
                }
            }
        }
    }
    return s;
}


int getLine(char **s, int charsNum) // reads line from stdin to *s, and returns indicator of EOF
{
    *s = (char *) malloc((charsNum + 1) * sizeof(char)); // +1 for \0
    if (*s == NULL) {
        printf("[error]");
        return 2;
    }
    int i = 0; // current char number in array
    char c; // current read char
    while ((c = getchar())) {
        if (c == '\n' || c == EOF) {
            (*s)[i] = '\0';
            if (c == EOF) {
                return 1;
            }
            break;
        }

        if (i != 0 && i % charsNum == 0) {
            char *tmp = (char *) realloc(*s, (charsNum + i + 1) * sizeof(char)); // +1 for \0
            if (tmp == NULL) {
                // for (int j = 0; j < i; j++) {
                //     free(*s[j]);
                // }
                free(*s);
                printf("[error]");
                return 2;
            }
            *s = tmp;
        }
        (*s)[i] = c;
        i++;
    }

    return 0;
}



int main(int argc, char const *argv[])
{
    char **s = NULL; // array of lines
    char *buf = NULL;
    int charsNum = 64;
    int linesNum = 2;
    s = (char **) malloc((linesNum + 1) * sizeof(char *)); // +1 for NULL str
    if (s == NULL) {
        printf("[error]");
        return 1;
    }
    int i = 0; // current line number
    int isEOF = 0;
    while (7) { // 7 is a lucky number =)
        isEOF = getLine(&buf, charsNum);

        if (isEOF == 2) { // error of alloc in getLine func
            return 1;
        }

        if (isEOF && strlen(buf) == 0) {
            free(buf);
            s[i] = NULL;
            break;
        }

        if (i != 0 && i % linesNum == 0) {
            char **tmp = (char **) realloc(s, (linesNum + i + 1) * sizeof(char *)); // +1 for NULL str
            if (tmp == NULL) {
                for (int j = 0; j < i; j++) {
                    free(s[j]);
                }
                free(s);
                printf("[error]");
                free(buf);
                return 1;
            }
            s = tmp;
        }
        if (strlen(buf) > 0) { // copy only non-empty liness
            s[i] = (char *) malloc((strlen(buf) + 1) * sizeof(char));
            if (s == NULL) {
                printf("[error]");
                free(buf);
                return 1;
            }
            memcpy(s[i], buf, (strlen(buf) + 1) * sizeof(char));
            free(buf);
            i++;
        }
        if (isEOF) {
            s[i] = NULL;
            break;
        }
    }

    s = filter(s, i);
    i = 0;
    while (s[i] != NULL) {
        printf("%s\n", s[i]);
        free(s[i]);
        i++;
    }
    free(s);
    return 0;
}