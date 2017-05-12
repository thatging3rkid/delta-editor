/**
 * delta.c
 *
 * A simple Unix text editor
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#define HEADER_HEIGHT 1
#define VERSION "v0.1"

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

static unsigned char linenum_width = 1;
static int max_row_len;
static int max_col_len;

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

static void draw_header(char * filename) {
    attron(COLOR_PAIR(1));

    printw("delta\t\t%s\t\t%s\n", filename, VERSION);
    
    attroff(COLOR_PAIR(1));
}

static void draw_file(FileContents * fc, int text_start) {
    
    for (int i = 1; i < 20; i += 1) {
        mvprintw(i, 0, "%i%s", 0, fc->data[i]->data);
    }
}

static bool valid_move(unsigned int x, unsigned int y, FileContents * fc) {
    return (x >= linenum_width && x < (unsigned int) fc->data[y]->len &&
            y >= HEADER_HEIGHT && y < (unsigned int) fc->len);
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

    // Make the color pairs
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    
    FileContents * fc = read_file(fp);
    draw_header(filename);
    draw_file(fc, 1);
    move(1, 1);
    refresh();
    
    int input;

    CursorPos pos = {.x = 1, .y = 1};
    while (true) {
        input = getch();
        
        if (input == KEY_UP && valid_move(pos.x, pos.y - 1, fc)) {
            pos.y -= 1;
        } else if (input == KEY_DOWN && valid_move(pos.x, pos.y + 1, fc)) {
            pos.y += 1;
        }
        if (input == KEY_LEFT  && valid_move(pos.x - 1, pos.y, fc)) {
            pos.x -= 1;
        } else if (input == KEY_RIGHT && valid_move(pos.x + 1, pos.y, fc)) {
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
