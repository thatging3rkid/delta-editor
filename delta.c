/**
 * delta.c
 *
 * A simple Unix text editor
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

typedef struct {
    unsigned int x;
    unsigned int y;
} CursorPos;

typedef struct {
    char * data;
    int len;
} FileLine;

typedef struct {
    FileLine ** data;
    int len;
} FileContents;

static FileContents * read_file(FILE * fp) {
    fseek(fp, 0, SEEK_SET);
    FileContents * output = malloc(sizeof(FileContents));
    output->data = NULL;
    output->len = 0;
    
    bool eof = false;
    while (true) {
        FileLine * entry = malloc(sizeof(FileLine));
        entry->len = 1;
        entry->data = malloc(sizeof(char));

        char input;
        while (true) {
            input = fgetc(fp);

            if (input == EOF) {
                eof = true;
                break;
            } else {
                entry->len += 1;
                char * temp = realloc(entry->data, sizeof(char) * entry->len);
                if (temp == NULL) {
                    fprintf(stderr, "delta: error allocating memory, exiting\n");
                    endwin();
                    exit(EXIT_FAILURE);
                } else {
                    entry->data = temp;
                    entry->data[entry->len - 2] = input;
                    entry->data[entry->len - 1] = '\0';
                }
                if (input == '\n') {
                    break;
                }
            }
        }

        if (output->len == 0) {
            output->data = malloc(sizeof(FileLine *));
        }
        output->len += 1;
        FileLine ** temp = realloc(output->data, sizeof(FileLine*) * output->len);
        if (temp == NULL) {
            fprintf(stderr, "delta: error allocating memory, exiting\n");
            endwin();
            exit(EXIT_FAILURE);
        } else {
            output->data = temp;
            output->data[output->len - 1] = entry;
        }

        if (eof) {
            break;
        }
    }
    return output;
}

static void fc_cleanup(FileContents * fc) {
    for (int i = 0; i < fc->len; i += 1) {
        free(fc->data[i]->data);
        free(fc->data[i]);
        fc->data[i] = NULL;
    }
    free(fc);
    fc = NULL;
}

static void draw_file(FileContents * fc, int text_start) {
    int row, col;
    getmaxyx(stdscr, row, col);
    const int max_len = text_start + row - 15;
    for (int i = 0; i < fc->len || i < max_len; i += 1) {
        printw("%s", fc->data[i]->data);
    }
}

static int edit_file(char * filename) {
    FILE * fp = fopen(filename, "r+");
    if (fp == NULL) {
        perror("delta");
        return EXIT_FAILURE;
    }

    initscr(); // Initalize ncurses
    raw();     // Get raw input
    noecho();  // Don't echo characters to the terminal
    keypad(stdscr, TRUE); // Enable reading of all keys

    FileContents * fc = read_file(fp);
    draw_file(fc, 1);
    move(1,1);
    refresh();
    
    int input;



    CursorPos pos = {.x = 1, .y = 1};
    while (true) {
        input = getch();

        if (input == KEY_UP) {
            pos.y -= 1;
        } else if (input == KEY_DOWN) {
            pos.y += 1;
        }
        if (input == KEY_LEFT) {
            pos.x -= 1;
        } else if (input == KEY_RIGHT) {
            pos.x += 1;
        }

        move(pos.y, pos.x);

        
        if (input == 5) {
            break;
        }
    }

    fc_cleanup(fc);
    endwin();
    return EXIT_SUCCESS;
    
}




int main(int argc, char * argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Usage: delta file\n");
        return EXIT_FAILURE;
    } else {
        for (int i = 1; i < argc; i += 1) {
            int file_status;
            if ((file_status = edit_file(argv[i])) != EXIT_SUCCESS) {
                return file_status;
            }
        }
        return EXIT_SUCCESS;
    }
    
}
