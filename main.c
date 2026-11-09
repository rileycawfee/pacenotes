#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_NOTES 500
#define MAX_STAGES 20

typedef struct {
    char dir;
    int severity;
} Note;

typedef struct {
    char name[64];
    Note notes[MAX_NOTES];
    int count;
} Stage;

int parse_note(const char *text, Note *out) {
    char word[16];
    int len = 0;
    int i = 0; 
    char dir;
    int severity;

    while (isspace((unsigned char)text[i])) i++;

    while (isalpha((unsigned char) text[i]) && len < 15) {
        word[len++] = tolower((unsigned char) text[i]);
        i++;
    }

    word[len] = '\0';

    if (strcmp(word, "l") == 0 || strcmp(word, "left") == 0) {
        dir = 'L';
    }
    else if (strcmp(word, "r") == 0 || strcmp(word, "right") == 0) {
        dir = 'R';
    }
    else {
        return 0;
    }

    while (isspace((unsigned char) text[i])) i++;

    if (text[i] < '1' || text[i] > '6') return 0;
    severity = text[i] - '0';
    i++;

    while (isspace((unsigned char) text[i])) i++;
    
    if (text[i] != '\0') {
        return 0;
    }

    out->dir = dir;
    out->severity = severity;
    return 1;
}

int main(void) {
    char line[100];
    Note n;

    printf("Type a note (blank line to quit):\n");
    while (fgets(line, sizeof(line), stdin) != NULL && line[0] != '\n') {
        if (parse_note(line, &n)) {
            printf("OK: dir=%c severity=%d\n", n.dir, n.severity);
        } else {
            printf("NOT A VALID NOTE\n");
        }
    }
    return 0;
}