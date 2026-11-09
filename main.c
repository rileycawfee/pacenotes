/* Rally Pace Notes - Milestone 1
   A stage is just a text file: stage_NAME.txt, with one note per line (like "LEFT 3"). */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#ifdef _WIN32
#include <conio.h>
#endif

#define MAX 500
#define LEN 16

char notes[MAX][LEN];   /* the notes of the stage being used */
int count = 0;

int read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) return 0;
    buf[strcspn(buf, "\n\r")] = '\0';
    return 1;
}

int is_back(const char *s) {
    return (s[0] == 'b' || s[0] == 'B') && s[1] == '\0';
}

/* Turns "L3" or "Left 3" into "LEFT 3", and a plain number like "150" into "STRAIGHT 150".
   Returns 1 if valid, 0 if not. */
int parse_note(const char *text, char *out) {
    char word[16], extra;
    int num, i;

    if (sscanf(text, " %d %c", &num, &extra) == 1 && num > 0 && num <= 9999) {
        sprintf(out, "STRAIGHT %d", num);
        return 1;
    }
    if (sscanf(text, " %15[A-Za-z] %d %c", word, &num, &extra) != 2) return 0;
    if (num < 1 || num > 6) return 0;
    for (i = 0; word[i]; i++) word[i] = tolower((unsigned char)word[i]);

    if (strcmp(word, "l") == 0 || strcmp(word, "left") == 0) {
        sprintf(out, "LEFT %d", num);
    } else if (strcmp(word, "r") == 0 || strcmp(word, "right") == 0) {
        sprintf(out, "RIGHT %d", num);
    } else {
        return 0;
    }
    return 1;
}

/* Loads the stage file NAME.txt into notes[]. Returns 0 if the stage doesn't exist. */
int load(const char *name) {
    char path[100], line[100];
    FILE *f;

    count = 0;
    snprintf(path, sizeof(path), "stage_%s.txt", name);
    f = fopen(path, "r");
    if (f == NULL) return 0;
    while (count < MAX && fgets(line, sizeof(line), f) != NULL) {
        if (parse_note(line, notes[count])) count++;
    }
    fclose(f);
    return 1;
}

/* Saves notes[] to NAME.txt. */
void save(const char *name) {
    char path[100];
    FILE *f;
    int i;

    snprintf(path, sizeof(path), "stage_%s.txt", name);
    f = fopen(path, "w");
    if (f == NULL) {
        printf("Could not save %s\n", path);
        return;
    }
    for (i = 0; i < count; i++) fprintf(f, "%s\n", notes[i]);
    fclose(f);
}

/* Prints the saved stages (the files named stage_NAME.txt). */
void list_stages(void) {
    DIR *d = opendir(".");
    struct dirent *e;
    int len, found = 0;

    if (d == NULL) return;
    printf("Saved stages:\n");
    while ((e = readdir(d)) != NULL) {
        len = strlen(e->d_name);
        if (len > 10 && strncmp(e->d_name, "stage_", 6) == 0 && strcmp(e->d_name + len - 4, ".txt") == 0) {
            printf("  - %.*s\n", len - 10, e->d_name + 6);
            found++;
        }
    }
    closedir(d);
    if (found == 0) printf("  (none yet)\n");
}

/* Asks for a stage name. Returns 0 if the user went back. */
int ask_stage(char *name) {
    list_stages();
    while (1) {
        printf("Stage name (B to go back): ");
        if (!read_line(name, 64) || is_back(name)) return 0;
        if (name[0] != '\0' && strpbrk(name, "/\\:*?\"<>|.") == NULL) return 1;
        printf("Please type a name without / \\ : * ? \" < > | or dots.\n");
    }
}

/* Waits for one key. If the terminal can't send single keys, the user types it and presses Enter. */
int get_key(void) {
    char line[100];

#ifdef _WIN32
    if (isatty(0)) return _getch();
#endif
    if (!read_line(line, sizeof(line))) return 27;
    return line[0] == '\0' ? ' ' : line[0];
}

void show_help(void) {
    char line[100];

    printf("\n--- HELP (optional) ---\n");
    printf("Notes can be typed short or long: L3   L 3   left 3   R5   right 5\n");
    printf("The number is 1 (tightest, like a hairpin) to 6 (slight bend).\n");
    printf("A plain number is a straight: type 150 for STRAIGHT 150.\n");
    printf("Type B and press Enter to go back. Notes are saved as .txt files in this folder.\n");
    printf("Press Enter to return: ");
    read_line(line, sizeof(line));
}

void create_view(void) {
    char name[64], line[100], note[LEN];

    printf("\n--- CREATE ---\n");
    printf("Type pace notes and they are saved to a stage (a text file in this folder).\n");
    printf("Steps:\n  1. Type a stage name\n  2. Type a note like L3, Left 3, or 150 (a 150 straight) and press Enter\n  3. Type B to finish\n");
    printf("Mistake? Type undo to remove the last note.\n\n");
    if (!ask_stage(name)) return;
    load(name);

    while (1) {
        printf("\n\"%s\" has %d notes. Last: %s\nNote> ", name, count, count ? notes[count - 1] : "(none)");
        if (!read_line(line, sizeof(line)) || is_back(line)) return;

        if (tolower((unsigned char)line[0]) == 'u') {
            if (count == 0) {
                printf("Nothing to undo.\n");
            } else {
                printf("Removed %s.\n", notes[--count]);
                save(name);
            }
        } else if (parse_note(line, note)) {
            if (count < MAX) {
                strcpy(notes[count++], note);
                save(name);
                printf("Saved %s.\n", note);
            } else {
                printf("This stage is full.\n");
            }
        } else {
            printf("Could not read that. Try L3, Left 3 (1 to 6), or 150 for a straight.\n");
        }
    }
}

void display_view(void) {
    char name[64];
    int i = -1, key, k;

    printf("\n--- DISPLAY ---\n");
    printf("Shows one pace note at a time in large text, so you can read it while driving.\n");
    if (!ask_stage(name)) return;
    if (!load(name)) {
        printf("There is no stage called \"%s\". Create it or import it first.\n", name);
        return;
    }
    if (count == 0) {
        printf("That stage has no notes yet.\n");
        return;
    }

    while (1) {
        for (k = 0; k < 30; k++) printf("\n");
        printf("%s    (Space or Enter = next, B = back one, Q = stop)\n\n\n\n", name);
        if (i < 0) {
            printf("    Press Space to show the first note.\n");
        } else if (i >= count) {
            printf("    End of stage.\n");
        } else {
            printf("    %s\n\n    (%d of %d)\n", notes[i], i + 1, count);
        }

        key = get_key();
        if (key == ' ' || key == '\n' || key == '\r') {
            if (i < count) i++;
        } else if (key == 'b' || key == 'B') {
            if (i > 0) i--;
        } else if (key == 'q' || key == 'Q' || key == 27) {
            printf("\n");
            return;
        }
    }
}

void import_view(void) {
    char path[100], name[64], line[100];
    char found[MAX][LEN];
    int n = 0, bad = 0, i;
    FILE *f;

    printf("\n--- IMPORT ---\n");
    printf("Load pace notes from a .txt file, one note per line (L3 and Left 3 both work).\n");
    printf("Cost: nothing is deleted. The notes go into a NEW stage; your other stages are not changed.\n");
    printf("Steps:\n  1. Type the file name\n  2. Type a name for the new stage\n  3. Check the count and answer Y\n");
    printf("No file? Type B, then use Create to type notes by hand.\n");

    while (1) {
        printf("\nFile name (B to go back): ");
        if (!read_line(path, sizeof(path)) || is_back(path)) return;
        f = fopen(path, "r");
        if (f != NULL) break;
        printf("Could not open \"%s\". Nothing was changed. Check the name and try again.\n", path);
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n\r")] = '\0';
        if (line[0] == '\0') continue;
        if (n < MAX && parse_note(line, found[n])) {
            n++;
        } else {
            bad++;
            printf("Skipped: \"%s\"\n", line);
        }
    }
    fclose(f);

    if (n == 0) {
        printf("No readable notes in that file. Nothing was changed.\n");
        return;
    }
    while (1) {
        if (!ask_stage(name)) return;
        if (!load(name)) break;
        printf("A stage called \"%s\" already exists. Pick a new name.\n", name);
    }

    printf("%d notes found, %d lines skipped. Create stage \"%s\" with them? (Y/N): ", n, bad, name);
    if (!read_line(line, sizeof(line)) || tolower((unsigned char)line[0]) != 'y') {
        printf("Cancelled. Nothing was changed.\n");
        return;
    }
    count = 0;
    for (i = 0; i < n; i++) strcpy(notes[count++], found[i]);
    save(name);
    printf("Imported. New stage \"%s\" has %d notes.\n", name, count);
}

int main(void) {
    char line[100];
    char c;

    while (1) {
        printf("\n=== RALLY PACE NOTES ===\n");
        printf("Make pace notes and read them back one at a time.\n");
        printf("Works offline. Nothing leaves your computer.\n\n");
        printf("1. Create   type pace notes and save them to a stage\n");
        printf("2. Display  step through a stage's notes one at a time\n");
        printf("3. Import   load notes from a .txt file\n");
        printf("?. Help     Q. Quit\n");
        printf("Choose (number or first letter): ");
        if (!read_line(line, sizeof(line))) break;
        c = tolower((unsigned char)line[0]);

        if (c == '1' || c == 'c') {
            create_view();
        } else if (c == '2' || c == 'd') {
            display_view();
        } else if (c == '3' || c == 'i') {
            import_view();
        } else if (c == '?' || c == 'h') {
            show_help();
        } else if (c == 'q') {
            printf("Quit? Your notes are already saved. (Y/N): ");
            if (read_line(line, sizeof(line)) && tolower((unsigned char)line[0]) == 'y') break;
        } else {
            printf("Please type 1, 2, 3, ? or Q.\n");
        }
    }
    printf("Goodbye.\n");
    return 0;
}