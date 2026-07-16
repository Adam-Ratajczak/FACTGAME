#include "item.h"

ItemRecipe* item_recipe_create(int firstItemId, ...)
{
    ItemRecipe* recipe = malloc(sizeof(ItemRecipe));
    if (!recipe)
        return NULL;

    recipe->requires = NULL;
    recipe->requiresCount = 0;

    va_list args;
    va_start(args, firstItemId);

    int itemId = firstItemId;

    while (itemId != -1)
    {
        int amount = va_arg(args, int);

        ItemRecipeRequirement* grown = realloc(recipe->requires, (recipe->requiresCount + 1) * sizeof(ItemRecipeRequirement));
        if (!grown)
        {
            free(recipe->requires);
            free(recipe);
            va_end(args);
            return NULL;
        }

        recipe->requires = grown;
        recipe->requires[recipe->requiresCount].itemId = itemId;
        recipe->requires[recipe->requiresCount].amount = amount;
        recipe->requiresCount++;

        itemId = va_arg(args, int);
    }

    va_end(args);
    return recipe;
}

ItemRegistry* item_registry_create(void)
{
    ItemRegistry* reg = calloc(1, sizeof(*reg));

    strcpy(reg->info[ITEM_STONE].name, "Stone");
    reg->info[ITEM_STONE].maxDurability = 0;
    reg->info[ITEM_STONE].function = ITEM_FUNCTION_MATERIAL;

    strcpy(reg->info[ITEM_COAL].name, "Coal");
    reg->info[ITEM_COAL].maxDurability = 0;
    reg->info[ITEM_COAL].function = ITEM_FUNCTION_MATERIAL;

    strcpy(reg->info[ITEM_RAW_IRON].name, "Iron Ore");
    reg->info[ITEM_RAW_IRON].maxDurability = 0;
    reg->info[ITEM_RAW_IRON].function = ITEM_FUNCTION_MATERIAL;

    strcpy(reg->info[ITEM_RAW_COPPER].name, "Copper Ore");
    reg->info[ITEM_RAW_COPPER].maxDurability = 0;
    reg->info[ITEM_STONE].function = ITEM_FUNCTION_MATERIAL;

    strcpy(reg->info[ITEM_IRON].name, "Iron Plate");
    reg->info[ITEM_IRON].maxDurability = 0;
    reg->info[ITEM_IRON].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_IRON].recipe =
        item_recipe_create(
            ITEM_RAW_IRON, 1,
            ITEM_COAL, 1,
            -1);

    strcpy(reg->info[ITEM_COPPER].name, "Copper Plate");
    reg->info[ITEM_COPPER].maxDurability = 0;
    reg->info[ITEM_COPPER].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_COPPER].recipe =
        item_recipe_create(
            ITEM_RAW_COPPER, 1,
            ITEM_COAL, 1,
            -1);

    strcpy(reg->info[ITEM_STONE_BRICK].name, "Stone Brick");
    reg->info[ITEM_STONE_BRICK].maxDurability = 0;
    reg->info[ITEM_STONE_BRICK].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_STONE_BRICK].recipe =
        item_recipe_create(
            ITEM_STONE, 2,
            ITEM_COAL, 1,
            -1);

    strcpy(reg->info[ITEM_GEAR].name, "Iron Gear");
    reg->info[ITEM_GEAR].maxDurability = 0;
    reg->info[ITEM_GEAR].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_GEAR].recipe =
        item_recipe_create(
            ITEM_IRON, 2,
            -1);

    strcpy(reg->info[ITEM_COPPER_WIRE].name, "Copper Wire");
    reg->info[ITEM_COPPER_WIRE].maxDurability = 0;
    reg->info[ITEM_COPPER_WIRE].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_COPPER_WIRE].recipe =
        item_recipe_create(
            ITEM_COPPER, 1,
            -1);

    strcpy(reg->info[ITEM_CIRCUIT].name, "Circuit");
    reg->info[ITEM_CIRCUIT].maxDurability = 0;
    reg->info[ITEM_CIRCUIT].function = ITEM_FUNCTION_MATERIAL;
    reg->info[ITEM_CIRCUIT].recipe =
        item_recipe_create(
            ITEM_COPPER_WIRE, 3,
            ITEM_IRON, 1,
            -1);

    strcpy(reg->info[ITEM_CONVEYOR_BELT].name, "Conveyor Belt");
    reg->info[ITEM_CONVEYOR_BELT].maxDurability = 0;
    reg->info[ITEM_CONVEYOR_BELT].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_CONVEYOR_BELT].recipe =
        item_recipe_create(
            ITEM_GEAR, 1,
            ITEM_IRON, 1,
            -1);

    strcpy(reg->info[ITEM_CHEST].name, "Chest");
    reg->info[ITEM_CHEST].maxDurability = 0;
    reg->info[ITEM_CHEST].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_CHEST].recipe =
        item_recipe_create(
            ITEM_STONE_BRICK, 8,
            -1);

    strcpy(reg->info[ITEM_FURNACE].name, "Furnace");
    reg->info[ITEM_FURNACE].maxDurability = 0;
    reg->info[ITEM_FURNACE].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_FURNACE].recipe =
        item_recipe_create(
            ITEM_STONE_BRICK, 12,
            ITEM_GEAR, 2,
            -1);

    strcpy(reg->info[ITEM_MINE].name, "Mining Drill");
    reg->info[ITEM_MINE].maxDurability = 0;
    reg->info[ITEM_MINE].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_MINE].recipe =
        item_recipe_create(
            ITEM_GEAR, 8,
            ITEM_IRON, 12,
            ITEM_CIRCUIT, 3,
            -1);

    strcpy(reg->info[ITEM_CRAFTER_HEAD].name, "Crafter Head");
    reg->info[ITEM_CRAFTER_HEAD].maxDurability = 0;
    reg->info[ITEM_CRAFTER_HEAD].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_CRAFTER_HEAD].recipe =
        item_recipe_create(
            ITEM_GEAR, 6,
            ITEM_CIRCUIT, 5,
            ITEM_IRON, 10,
            -1);

    strcpy(reg->info[ITEM_CRAFTER_MODULE].name, "Crafter Module");
    reg->info[ITEM_CRAFTER_MODULE].maxDurability = 0;
    reg->info[ITEM_CRAFTER_MODULE].function = ITEM_FUNCTION_BLOCK;
    reg->info[ITEM_CRAFTER_MODULE].recipe =
        item_recipe_create(
            ITEM_GEAR, 2,
            ITEM_CIRCUIT, 2,
            ITEM_IRON, 6,
            -1);

    strcpy(reg->info[ITEM_ATTACK_ORB].name, "Attack Orb");
    reg->info[ITEM_ATTACK_ORB].maxDurability = 256;
    reg->info[ITEM_ATTACK_ORB].function = ITEM_FUNCTION_WEAPON;
    reg->info[ITEM_ATTACK_ORB].recipe =
        item_recipe_create(
            ITEM_CIRCUIT, 6,
            ITEM_COPPER, 8,
            ITEM_GEAR, 2,
            -1);

    strcpy(reg->info[ITEM_MINING_ORB].name, "Mining Orb");
    reg->info[ITEM_MINING_ORB].maxDurability = 256;
    reg->info[ITEM_MINING_ORB].function = ITEM_FUNCTION_TOOL;
    reg->info[ITEM_MINING_ORB].recipe =
        item_recipe_create(
            ITEM_CIRCUIT, 4,
            ITEM_GEAR, 4,
            ITEM_IRON, 6,
            -1);

    return reg;
}

Item* item_create(ItemRegistry* itemReg, TextureManager* texmgr, int itemId, int amount){
    Item* item = (Item*)malloc(sizeof(Item));
    int texLeft, texTop;
    if(!item_get_texcoords(itemId, &texLeft, &texTop)){
        free(item);
        return NULL;
    }

    item->itemId = itemId;
    item->sprite = create_entity(0, 0, 8, 8);
    add_sprite_to_entity(texmgr, item->sprite, "ASSETS/TEXTURES/items.pcx", texLeft, texTop);
    item->amount = amount;
    item->maxDurability = itemReg->info[itemId].maxDurability;
    item->durability = itemReg->info[itemId].maxDurability;
    item->function = itemReg->info[itemId].function;
    item->inInventory = 0;

    return item;
}
void item_destroy(Item* item){
    if(!item){
        return;
    }
    destroy_entity(item->sprite);
    free(item);
}
void item_render(BITMAP* scr, Item* item, const Box* vp)
{
    if (!scr || !item)
        return;

    render_entity(scr, item->sprite, vp);

    if (!item->inInventory)
        return;

    int x = item->sprite->x;
    int y = item->sprite->y;

    if (item->amount > 1)
    {
        textprintf_right_ex(
            scr,
            font,
            x + 8,
            y,
            makecol(255, 255, 255),
            -1,
            "%d",
            item->amount);
    }

    if (item->maxDurability)
    {
        int width = (item->durability * 8) / item->maxDurability;
        if (width < 0) width = 0;
        if (width > 8) width = 8;

        hline(scr, x, y + 7, x + 7, makecol(40, 40, 40));

        if (width > 0)
            hline(scr, x, y + 7, x + width - 1, makecol(255, 0, 0));
    }
}

int item_get_texcoords(int itemId, int* left, int* top){
    if (itemId < 0 || itemId >= ITEM_COUNT)
        return 0;

    *left = (itemId % 8) * 8;
    *top  = (itemId / 8) * 8;

    return 1;
}

int item_has_function(Item* item, int function){
    if(!item){
        return 0;
    }

    return item->function == function;
}
