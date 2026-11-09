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

int main(void) {
    printf("## RALLY PACE NOTES ##\n");
    return 0;
}