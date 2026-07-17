#ifndef HELP_H
#define HELP_H
#include "alwrap.h"

typedef struct {
    char** lines;
    int linesCount;
} HelpPage;

typedef struct {
    HelpPage** pages;
    int pagesCount;
    int activePage;
    int shown;
} Help;

Help* help_create();
void help_destroy(Help* help);
HelpPage* help_page_create();
void help_add_page(Help* help, HelpPage* page);
void help_page_add_line(HelpPage* page, const char* line);

void help_show(Help* help);
void help_hide(Help* help);
void help_next_page(Help* help);
void help_prev_page(Help* help);

void help_render(BITMAP* scr, Help* help);

#endif
