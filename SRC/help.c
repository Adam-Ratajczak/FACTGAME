#include "help.h"
#include <stdio.h>

Help* help_create()
{
    Help* help = (Help*)malloc(sizeof(Help));
    if (!help)
        return NULL;

    help->pages = NULL;
    help->pagesCount = 0;
    help->activePage = 0;
    help->shown = 0;

    HelpPage* page;

    page = help_page_create();

    help_page_add_line(page, "FACTGAME");
    help_page_add_line(page, "");
    help_page_add_line(page, "Welcome to FACTGAME!");
    help_page_add_line(page, "");
    help_page_add_line(page, "Controls:");
    help_page_add_line(page, "WSAD  - Move");
    help_page_add_line(page, "E     - Inventory");
    help_page_add_line(page, "H     - Help");
    help_page_add_line(page, "Q     - Pick up / Drop items");
    help_page_add_line(page, "R     - Rotate blocks");
    help_page_add_line(page, "F     - Open machine's inventory");
    help_page_add_line(page, "1-6   - Select hotbar slot");
    help_page_add_line(page, "Esc   - Cancel");
    help_page_add_line(page, "X     - Exit game");
    help_page_add_line(page, "");
    help_page_add_line(page, "Use <- and -> to change pages.");

    help_add_page(help, page);

    page = help_page_create();

    help_page_add_line(page, "Inventory");
    help_page_add_line(page, "");
    help_page_add_line(page, "Click the source slot first,");
    help_page_add_line(page, "then click the destination.");
    help_page_add_line(page, "");
    help_page_add_line(page, "Hotbar slots:");
    help_page_add_line(page, "Red   - Weapon");
    help_page_add_line(page, "Green - Mining tool");
    help_page_add_line(page, "Blue  - Crafting / Smelting");
    help_page_add_line(page, "Others- Building blocks");
    help_page_add_line(page, "");
    help_page_add_line(page, "Only matching items can be");
    help_page_add_line(page, "placed into colored slots.");
    help_page_add_line(page, "");
    help_page_add_line(page, "");

    help_add_page(help, page);

    page = help_page_create();

    help_page_add_line(page, "Resources");
    help_page_add_line(page, "");
    help_page_add_line(page, "Mine the world to obtain:");
    help_page_add_line(page, "");
    help_page_add_line(page, "- Stone");
    help_page_add_line(page, "- Coal");
    help_page_add_line(page, "- Iron Ore");
    help_page_add_line(page, "- Copper Ore");
    help_page_add_line(page, "");
    help_page_add_line(page, "These materials are used");
    help_page_add_line(page, "for crafting and smelting.");
    help_page_add_line(page, "");
    help_page_add_line(page, "");
    help_page_add_line(page, "");

    help_add_page(help, page);

    page = help_page_create();

    help_page_add_line(page, "Items & Resources");
    help_page_add_line(page, "");
    help_page_add_line(page, "Stone - Basic building material.");
    help_page_add_line(page, "Coal - Furnace fuel.");
    help_page_add_line(page, "Iron Ore -> Iron Plate");
    help_page_add_line(page, "Copper Ore -> Copper Plate");
    help_page_add_line(page, "Iron Plate - Basic component.");
    help_page_add_line(page, "Copper Plate - Basic component.");
    help_page_add_line(page, "Stone Brick - Smelt 2 Stone.");
    help_page_add_line(page, "Iron Gear = 2 Iron Plate");
    help_page_add_line(page, "Copper Wire = 1 Copper");
    help_page_add_line(page, "Circuit = 3 Wire + 1 Iron");
    help_page_add_line(page, "");
    help_page_add_line(page, "Machines on next page...");
    help_page_add_line(page, "");
    help_page_add_line(page, "");

    help_add_page(help, page);

    page = help_page_create();

    help_page_add_line(page, "Machines");
    help_page_add_line(page, "");
    help_page_add_line(page, "Conveyor");
    help_page_add_line(page, "  1 Gear + 1 Iron");
    help_page_add_line(page, "Splitter");
    help_page_add_line(page, "  2 Conveyor + 2 Gear");
    help_page_add_line(page, "  + 2 Iron");
    help_page_add_line(page, "Chest");
    help_page_add_line(page, "  8 Stone Brick");
    help_page_add_line(page, "Furnace");
    help_page_add_line(page, "  12 Brick + 2 Gear");
    help_page_add_line(page, "Mining Drill");
    help_page_add_line(page, "  8 Gear + 12 Iron");
    help_page_add_line(page, "  + 3 Circuit");
    help_page_add_line(page, "");
    help_page_add_line(page, "Continue...");

    help_add_page(help, page);

    page = help_page_create();

    help_page_add_line(page, "Advanced Crafting");
    help_page_add_line(page, "");
    help_page_add_line(page, "Crafter Head");
    help_page_add_line(page, "  6 Gear + 5 Circuit");
    help_page_add_line(page, "  + 10 Iron");
    help_page_add_line(page, "Crafter Module");
    help_page_add_line(page, "  2 Gear + 2 Circuit");
    help_page_add_line(page, "  + 6 Iron");
    help_page_add_line(page, "Attack Orb");
    help_page_add_line(page, "  6 Circuit + 8 Copper");
    help_page_add_line(page, "  + 2 Gear");
    help_page_add_line(page, "Mining Orb");
    help_page_add_line(page, "  4 Circuit + 4 Gear");
    help_page_add_line(page, "  + 6 Iron");
    help_page_add_line(page, "");
    help_page_add_line(page, "Good luck!");

    help_add_page(help, page);

    return help;
}

void help_destroy(Help* help)
{
    if (!help)
        return;

    for (int i = 0; i < help->pagesCount; ++i)
    {
        HelpPage* page = help->pages[i];
        if (!page)
            continue;

        for (int j = 0; j < page->linesCount; ++j)
            free(page->lines[j]);

        free(page->lines);
        free(page);
    }

    free(help->pages);
    free(help);
}

HelpPage* help_page_create()
{
    HelpPage* page = malloc(sizeof(HelpPage));
    if (!page)
        return NULL;

    page->lines = NULL;
    page->linesCount = 0;

    return page;
}

void help_add_page(Help* help, HelpPage* page)
{
    if (!help || !page)
        return;

    HelpPage** pages = realloc(help->pages, sizeof(HelpPage*) * (help->pagesCount + 1));

    if (!pages)
        return;

    help->pages = pages;
    help->pages[help->pagesCount++] = page;
}

void help_page_add_line(HelpPage* page, const char* line)
{
    if (!page || !line)
        return;

    char** lines = realloc(page->lines, sizeof(char*) * (page->linesCount + 1));

    if (!lines)
        return;

    page->lines = lines;

    page->lines[page->linesCount] = malloc(strlen(line) + 1);
    strcpy(page->lines[page->linesCount], line);

    page->linesCount++;
}

void help_show(Help* help)
{
    if (help)
        help->shown = 1;
}

void help_hide(Help* help)
{
    if (help)
        help->shown = 0;
}

void help_next_page(Help* help){
    if(!help || !help->shown || help->activePage >= help->pagesCount - 1){
        return;
    }

    help->activePage++;
}

void help_prev_page(Help* help){
    if(!help || !help->shown || help->activePage <= 0){
        return;
    }

    help->activePage--;
}

void help_render(BITMAP* scr, Help* help)
{
    if (!scr || !help || !help->shown)
        return;

    if (help->pagesCount == 0)
        return;

    if (help->activePage < 0 || help->activePage >= help->pagesCount)
        return;

    HelpPage* page = help->pages[help->activePage];

    rectfill(
        scr,
        0,
        0,
        SCREEN_W - 1,
        SCREEN_H - 1,
        makecol(0, 0, 0));

    int y = 8;

    for (int i = 0; i < page->linesCount; ++i)
    {
        textout_ex(
            scr,
            font,
            page->lines[i],
            8,
            y,
            makecol(255, 255, 255),
            -1);

        y += text_height(font) + 2;
    }

    char footer[64];
    sprintf(
        footer,
        "Page %d/%d",
        help->activePage + 1,
        help->pagesCount);

    textout_right_ex(
        scr,
        font,
        footer,
        SCREEN_W - 8,
        SCREEN_H - text_height(font) - 8,
        makecol(255, 255, 255),
        -1);
}
