#ifndef ITEM_H
#define ITEM_H
#include "entity.h"
#include "tiles.h"

#define ITEM_STONE          0x0000
#define ITEM_COAL           0x0001
#define ITEM_IRON           0x0002
#define ITEM_COPPER         0x0003
#define ITEM_CONVEYOR_BELT  0x0004
#define ITEM_FURNACE        0x0005
#define ITEM_MINE           0x0006
#define ITEM_CRAFTER_HEAD   0x0007
#define ITEM_CRAFTER_MODULE 0x0008
#define ITEM_CHEST          0x0009
#define ITEM_ATTACK_ORB     0x000A
#define ITEM_MINING_ORB     0x000B
#define ITEM_RAW_IRON       0x000C
#define ITEM_RAW_COPPER     0x000D
#define ITEM_GEAR           0x000E
#define ITEM_PLATE          0x000F
#define ITEM_COPPER_WIRE    0x0010
#define ITEM_CIRCUIT        0x0011
#define ITEM_STONE_BRICK    0x0012
#define ITEM_SPLITTER       0x0013
#define ITEM_TUNNEL         0x0014
#define ITEM_COUNT          0x0015

#define ITEM_FUNCTION_WEAPON 0x0
#define ITEM_FUNCTION_TOOL 0x1
#define ITEM_FUNCTION_MATERIAL 0x2
#define ITEM_FUNCTION_BLOCK 0x3

#define ITEM_STACK_SIZE 64

typedef struct {
    int itemId;
    int amount;
} ItemRecipeRequirement;

typedef struct {
    ItemRecipeRequirement* requires;
    int requiresCount;
} ItemRecipe;

ItemRecipe* item_recipe_create(int firstItemId, ...);

typedef struct{
    char name[64];
    int maxDurability;
    int function;
    int aquiredFrom;
    int placedAs;
    int smelting;
    ItemRecipe* recipe;
} ItemInfo;

typedef struct{
    ItemInfo info[ITEM_COUNT];
} ItemRegistry;

ItemRegistry* item_registry_create();
void item_registry_destroy(ItemRegistry* registry);

typedef struct{
    int itemId;
    int amount;
    int maxDurability;
    int durability;
    int inInventory;
    int function;
    Entity* sprite;
} Item;

Item* item_create(ItemRegistry* itemReg, TextureManager* texmgr, int itemId, int amount);
void item_destroy(Item* item);
void item_render(BITMAP* scr, Item* item, const Box* vp);

int item_get_texcoords(int itemId, int* left, int* top);

int item_has_function(Item* item, int function);
int item_is_stackable(Item* item);

#endif