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
#define PAGE_JUMP 60
#define ARROW_JUMP 30

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
    free(fc->data);
    free(fc);
    fc = NULL;
}

static void fc_insert(FileContents * fc, int x, int y, char ins_char) {
    if (y < 0 || y > fc->len || x < 0 || x > fc->data[y]->len - 2) {
        return;
    }
    
    fc->data[y]->len += 1;
    char * temp = realloc(fc->data[y]->data, fc->data[y]->len);
    if (temp == NULL) {
        fprintf(stderr, "delta: an error has occured");
        endwin();
        exit(EXIT_FAILURE);
    }
    fc->data[y]->data = temp;

    if (fc->data[y]->len == 3) {
        fc->data[y]->data[0] = ins_char;
        fc->data[y]->data[1] = '\n';
        fc->data[y]->data[2] = '\0';
        return;
    }
    
    char temp_c = fc->data[y]->data[x];
    fc->data[y]->data[x] = ins_char;
    for(int h = x + 1; h < fc->data[y]->len - 4; h += 1) {
        fc->data[y]->data[h] = temp_c;
        temp_c = fc->data[y]->data[h + 1];
    }
    fc->data[y]->data[fc->data[y]->len - 3] = temp_c;
    fc->data[y]->data[fc->data[y]->len - 2] = '\n';
    fc->data[y]->data[fc->data[y]->len - 1] = '\0';
    
}

static void draw_header(char * filename) {
    attron(COLOR_PAIR(1));

    printw("Delta");
    for (int i = 0; i < (int) (max_pos.x - (8 + strlen(filename))); i += 1) {
        printw(" ");
    }
    printw("e: %s", filename);
    printw("\n");
    
    attroff(COLOR_PAIR(1));
}

static void draw_file(FileContents * fc, int text_start) {    
    int text_end;
    if (fc->len <= (text_start + max_pos.y)) {
        text_end = fc->len - 1;
    } else {
        text_end = text_start + max_pos.y;
    }

    linenum_width = log10(fc->len + 1) + 1;
    
    for (int i = text_start; i < text_end; i += 1) {
        mvprintw((i - text_start) + HEADER_HEIGHT, 0, "%*d%s", linenum_width, i + 1, fc->data[i]->data);
    }
    clrtobot();
}

static bool valid_move(int x, int y, FileContents * fc) {
    return (x >= 0 && y + 1 >= HEADER_HEIGHT &&
            y < fc->len && x + 1 < fc->data[y]->len);
}

static void update_max() {
    getmaxyx(stdscr, max_pos.y, max_pos.x);
}

static bool at_eol(int x, int y, FileContents * fc) {
    return (y < fc->len && x == (fc->data[y]->len -2));
}

static bool at_bol(int x, int y, FileContents * fc) {
    return (x == 0);
}

static void write_file(FileContents * fc, char * filename) {
    FILE * fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("delta");
        endwin();
        return;
    }

    for (int i = 0; i < fc->len; i += 1) {
        fwrite(fc->data[i]->data, sizeof(char), fc->data[i]->len - 1, fp);
    }

    fclose(fp);
}

static int edit_file(char * filename) {
    FILE * fp = fopen(filename, "r+");
    if (fp == NULL) {
        perror("delta");
        endwin();
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
    fclose(fp);
    update_max();
    draw_header(filename);
    draw_file(fc, 0);
    move(1, 1);
    refresh();
    
    int input;
    int start_line = 0;

    CursorPos pos = {.x = 0, .y = 0};
    move(pos.y + HEADER_HEIGHT, pos.x + linenum_width);
    while (true) {
        input = getch();

        if (input == KEY_LEFT && at_bol(pos.x, pos.y, fc)) {
            if (valid_move(0, pos.y - 1, fc)) {
                pos.y -= 1;
                pos.x = (fc->data[pos.y]->len - 2);
            }
        } else if (input == KEY_RIGHT && at_eol(pos.x, pos.y, fc)) {
            if (valid_move(0, pos.y + 1, fc)) {
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

        if (input == KEY_NPAGE) {
            if (fc->len > pos.y + start_line + PAGE_JUMP) {
                pos.y += PAGE_JUMP;
                start_line += PAGE_JUMP;

                refresh();
            }
        } else if (input == KEY_PPAGE) {
            if (0 <= pos.y + start_line - PAGE_JUMP) {
                pos.y -= PAGE_JUMP;
                start_line -= PAGE_JUMP;

                refresh();
            }
        }        

        if (32 <= input && input <= 126) {
            fc_insert(fc, pos.x, pos.y, (char) input);
            pos.x += 1;
        }
        
        draw_file(fc, start_line);
        move(pos.y + HEADER_HEIGHT - start_line, pos.x + linenum_width);       
        refresh();
        
        // CTRL^s to save
        if (input == 19) {
            write_file(fc, filename);
        }

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
