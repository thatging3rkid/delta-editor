/**
 * delta.c
 *
 * A simple Unix text editor
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#define FOOTER_HEIGHT 1
#define PAGE_JUMP 60
#define ARROW_JUMP 30
#define STATUS_LEN 46

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
static char status[STATUS_LEN];
static bool error_status = false;

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
        fprintf(stderr, "delta: an error has occured\n");
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

    char * temp_s = malloc(sizeof(char) * (fc->data[y]->len - x));
    strncpy(temp_s, fc->data[y]->data + x, fc->data[y]->len - 1 - x);
    fc->data[y]->data[x] = ins_char;
    strncpy(fc->data[y]->data + x + 1, temp_s, fc->data[y]->len - 1 - x);
    free(temp_s);
}

static void fc_remove(FileContents * fc, int x, int y) {
    if (y < 0 || y > fc->len || x < 0 || x > fc->data[y]->len - 1) {
        beep(); // for debugging right now, might become a feature
        return;
    }

    if (x == fc->data[y]->len - 2) {
        // Deleting the newline character
        int new_len = fc->data[y]->len + fc->data[y + 1]->len - 2;

        char * temp = realloc(fc->data[y]->data, new_len);
        if (temp == NULL) {
            fprintf(stderr, "delta: an error has occured\n");
            endwin();
            exit(EXIT_FAILURE);
        }
        fc->data[y]->data = temp;
        strncpy(fc->data[y]->data + fc->data[y]->len - 2, fc->data[y + 1]->data, fc->data[y + 1]->len);

        free(fc->data[y + 1]->data);
        free(fc->data[y + 1]);

        fc->len -= 1;
        
        for (int i = y + 1; i < fc->len - 1; i += 1) {
            fc->data[i] = fc->data[y + 1];
        }
    } else {
        char * temp_s = malloc(sizeof(char) * fc->data[y]->len);
        strncpy(temp_s, fc->data[y]->data, fc->data[y]->len);

        fc->data[y]->len -= 1;
        char * temp = realloc(fc->data[y]->data, fc->data[y]->len);
        if (temp == NULL) {
            fprintf(stderr, "delta: an error has occured\n");
            endwin();
            exit(EXIT_FAILURE);
        }
        fc->data[y]->data = temp;
        x += 1;
        strncpy(fc->data[y]->data, temp_s, x);
        strncpy(fc->data[y]->data + x - 1, temp_s + x, fc->data[y]->len - x);
        free(temp_s);
    }
}

static void fc_newline(FileContents * fc, int x, int y) {
    beep();
    
}

static void clear_status() {
    for (int i = 0; i < STATUS_LEN; i += 1) {
        status[i] = '\0';
    }
    error_status = false;
}

static void set_status(char * new_status) {
    error_status = false;
    strncpy(status, new_status, STATUS_LEN);
}

static void set_status_err(char * new_status) {
    set_status(new_status);
    error_status = true;
}

static void fileset_status(int errsv) {
    switch(errsv) {
    case ENOENT:
        set_status_err("No such file or directory");
        break;
    case EINTR:
        set_status_err("Interrupted system call");
        break;
    case EIO:
        set_status_err("I/O error");
        break;
    case EAGAIN:
        set_status_err("Try again"); // I guess this is an error?
        break;
    case EACCES:
        set_status_err("Permission denied");
        break;
    case EBUSY:
        set_status_err("Resource busy");
        break;
    case EISDIR:
        set_status_err("Is a directory");
        break;
    case ETXTBSY:
        set_status_err("Text file busy");
        break;
    case EFBIG:
        set_status_err("File too large");
        break;
    case ENOSPC:
        set_status_err("No space left on device");
        break;
    case EROFS:
        set_status_err("Read-only file system");
        break;
    default:
        set_status_err(sprintf("Error %i, consult internet", errno));
        break;
    }
}

static void draw_footer(char * filename, int x, int y, bool changed) {
    attron(COLOR_PAIR(1) | A_BLINK);

    move(max_pos.y - 1, 0);
    if (changed) {
        printw("*");
    } else {
        printw(" ");
    }
    printw("%s:%*d:%d        ", filename, linenum_width, y + 1, x + 1);
    move(max_pos.y - 1, linenum_width + 7 + strlen(filename));
    printw("Delta ");
    
    attroff(COLOR_PAIR(1) | A_BLINK);

    if (error_status) {
        // Print message a different color
        attron(COLOR_PAIR(3) | A_BLINK);
        
        printw(" ");
        for (int i = 0; i < STATUS_LEN; i += 1) {
            char temp = status[i];
            if (temp == '\0') {
                printw("%c", ' ');
            } else {
                printw("%c", temp);
            }
        }
        clear_status();
        attroff(COLOR_PAIR(3) | A_BLINK);

    } else {
        // Print message normally
        attron(COLOR_PAIR(2) | A_BLINK);
        
        printw(" ");
        for (int i = 0; i < STATUS_LEN; i += 1) {
            char temp = status[i];
            if (temp == '\0') {
                printw("%c", ' ');
            } else {
                printw("%c", temp);
            }
        }
        clear_status();
        attroff(COLOR_PAIR(2) | A_BLINK);
    }
    
    int y_pos, x_pos = 0;
    getyx(stdscr, y_pos, x_pos);
    attron(COLOR_PAIR(1) | A_BLINK);
    
    for (int j = x_pos; j < max_pos.x; j += 1) {
        printw(" ");
    }
    
    attroff(COLOR_PAIR(1) | A_BLINK);
}

static void draw_file(FileContents * fc, int text_start) {    
    int text_end;
    if (fc->len <= (text_start + max_pos.y - FOOTER_HEIGHT)) {
        text_end = fc->len - 1;
    } else {
        text_end = text_start + max_pos.y - FOOTER_HEIGHT;
    }

    linenum_width = log10(fc->len + 1) + 1;

    clrtobot();
    for (int i = text_start; i < text_end; i += 1) {
        mvprintw(i - text_start, 0, "%*d%s", linenum_width, i + 1, fc->data[i]->data);
    }

}

static bool valid_move(int x, int y, FileContents * fc) {
    return (x >= 0 && y >= 0 &&
            y < fc->len - FOOTER_HEIGHT && x + 1 < fc->data[y]->len);
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
        fileset_status(errno);
        return;
    }

    for (int i = 0; i < fc->len; i += 1) {
        fwrite(fc->data[i]->data, sizeof(char), fc->data[i]->len - 1, fp);
    }

    fclose(fp);
    set_status("Successfully wrote file");
}

static int edit_file(char * filename) {
    FILE * fp = fopen(filename, "r");
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
    init_pair(1, COLOR_BLACK, COLOR_WHITE); // Header color
    init_pair(2, COLOR_BLACK, COLOR_CYAN);  // Status bar color
    init_pair(3, COLOR_RED, COLOR_CYAN);    // Status bar error color
    
    FileContents * fc = read_file(fp);
    fclose(fp);
    update_max();
    draw_file(fc, 0);
    move(1, 1);
    refresh();
    
    int input;
    int start_line = 0;
    bool changed = false;
    int tab_does = 4; // will be read from a config file in later revisions
    
    CursorPos pos = {.x = 0, .y = 0};
    draw_footer(filename, pos.x, pos.y, changed);
    move(pos.y, pos.x + linenum_width);
    
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
            changed = true;
        }

        if (input == KEY_BACKSPACE) {
            if (pos.x >= 1) {
                changed = true;
                pos.x -= 1;
                fc_remove(fc, pos.x, pos.y);
            }
        }
        
        if (input == KEY_DC) {
            changed = true;
            fc_remove(fc, pos.x, pos.y);
        }

        if (input == '\n') {
            changed = true;
            fc_newline(fc, pos.x, pos.y);
        }

        if (input == '\t') {
            if (tab_does == -1) {
                fc_insert(fc, pos.x, pos.y, '\t');
                pos.x += 6;
            } else {
                for (int i = 0; i < tab_does; i += 1) {
                    fc_insert(fc, pos.x, pos.y, ' ');
                    pos.x += 1;
                }
            }
        }
        
        // CTRL^s to save
        if (input == 19) {
            if (changed) {   
                write_file(fc, filename);
                changed = false;
            }
        }

        // CTRL^e to exit
        if (input == 5) {
            break;
        }
        
        draw_file(fc, start_line);
        draw_footer(filename, pos.x, pos.y, changed);
        move(pos.y - start_line, pos.x + linenum_width);       
        update_max();
        refresh();
    }

    fc_cleanup(fc);
    endwin();
    return EXIT_SUCCESS;
    
}

int main(int argc, char * argv[]) {
    if (argc == 1) {
        return EXIT_FAILURE; // Replace with some sort of tutorial/splash page
    }
    
    for (int i = 1; i < argc; i += 1) {
        int file_status;
        if ((file_status = edit_file(argv[i])) != EXIT_SUCCESS) {
            return file_status;
        }
    }
    return EXIT_SUCCESS;    
}
