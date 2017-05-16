/**
 * delta.c
 *
 * A simple Unix text editor
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#define HEADER_HEIGHT 1

typedef struct {
    int x;
    int y;
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
static CursorPos max_pos = {.x = 1, .y = 1};

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

    printw("delta");
    for (int i = 0; i < (int) (max_pos.x - (8 + strlen(filename))); i += 1) {
        printw(" ");
    }
    printw("e: %s", filename);
    printw("\n");
    
    attroff(COLOR_PAIR(1));
}

static void draw_file(FileContents * fc, int text_start) {    
    int text_end;
    if (fc->len <= max_pos.y) {
        text_end = fc->len - 1;
    } else {
        text_end = text_start + max_pos.y;
    }

    linenum_width = log10(fc->len + 1) + 1;
    
    for (int i = text_start; i < text_end; i += 1) {
        mvprintw(i + HEADER_HEIGHT, 0, "%*d%s", linenum_width, i + 1, fc->data[i]->data);
    }
}

static bool valid_move(int x, int y, FileContents * fc) {
    return (x >= 0 && y + 1 >= HEADER_HEIGHT &&
            x + 1 < fc->data[y]->len && y < fc->len);
}

static void update_max() {
    getmaxyx(stdscr, max_pos.y, max_pos.x);
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
    update_max();
    draw_header(filename);
    draw_file(fc, 0);
    move(1, 1);
    refresh();
    
    int input;

    CursorPos pos = {.x = 0, .y = 0};
    move(pos.y + HEADER_HEIGHT, pos.x + linenum_width);
    while (true) {
        input = getch();

        if (input == KEY_LEFT && at_eol(pos.x, pos.y, fc)) {
            if (valid_move(0, pos.y - 1)) {
                pos.x = 0;
                pos.y -= 1;
            }
        } else if (input == KEY_RIGHT && at_eol(pos.x, pos.y, fc)) {
            if (valid_move(0, pos.y + 1)) {
                pos.x = 0;
                pos.y += 1;
            }
        } else {
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
        }

        move(pos.y + HEADER_HEIGHT, pos.x + linenum_width);       
        
        // CTRL^e to exit
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
