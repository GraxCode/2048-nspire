#include <os.h>
#include <SDL/SDL.h>

int pad = 10; //padding of the table

SDL_Surface* screen;
nSDL_Font* font;
nSDL_Font* tinyfont;

SDL_bool done = SDL_FALSE;

int tiles[16];
int score = 0;

int last[16]; //for undo
int lastscore = -1;

bool changed = false;

//colors
Uint32 _2 = 0;
Uint32 _4 = 0;
Uint32 _8 = 0;
Uint32 _16 = 0;
Uint32 _32 = 0;
Uint32 _64 = 0;
Uint32 _128 = 0;
Uint32 _256 = 0;
Uint32 _512 = 0;
Uint32 _1024 = 0;
Uint32 _2048 = 0;
Uint32 _other = 0;

void init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    screen = SDL_SetVideoMode(320, 240, has_colors ? 16 : 8, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("Couldn't initialize display: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    font = nSDL_LoadFont(NSDL_FONT_VGA,
        250, 250, 250);
    tinyfont = nSDL_LoadFont(NSDL_FONT_TINYTYPE,
        128, 128, 128);
    SDL_ShowCursor(SDL_DISABLE);
    srand(time(NULL)); //seed random

    _2 = SDL_MapRGB(screen->format, 238, 228, 218);
    _4 = SDL_MapRGB(screen->format, 237, 224, 200);
    _8 = SDL_MapRGB(screen->format, 242, 177, 121);
    _16 = SDL_MapRGB(screen->format, 245, 149, 99);
    _32 = SDL_MapRGB(screen->format, 246, 124, 95);
    _64 = SDL_MapRGB(screen->format, 246, 93, 59);
    _128 = SDL_MapRGB(screen->format, 237, 206, 113);
    _256 = SDL_MapRGB(screen->format, 237, 204, 97);
    _512 = SDL_MapRGB(screen->format, 236, 200, 80);
    _1024 = SDL_MapRGB(screen->format, 237, 197, 63);
    _2048 = SDL_MapRGB(screen->format, 236, 193, 45);
    _other = SDL_MapRGB(screen->format, 255, 30, 32);
}

void quit(void)
{
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void draw_rect_bordered(SDL_Rect rect, Uint32 map)
{
    SDL_Rect hrect = { rect.x + 1, rect.y, rect.w - 2, rect.h };
    SDL_Rect vrect = { rect.x, rect.y + 1, rect.w, rect.h - 2 };
    SDL_FillRect(screen, &hrect, map);
    SDL_FillRect(screen, &vrect, map);
}

void draw_table()
{
    int x = 40 + pad;
    int y = pad;
    int w = 240 - (pad * 2);
    int h = 240 - (pad * 2);
    SDL_Rect outlineRect = { x - 2, y - 2, w + 4, h + 4 };
    SDL_FillRect(screen, &outlineRect, SDL_MapRGB(screen->format, 187, 173, 160));
    SDL_Rect rect = { x, y, w, h };
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 204, 192, 179));
    for (int i = 0; i < 4; i++) {
        SDL_Rect hline = { x, y + ((h / 4) * i) - 1, w, 2 };
        SDL_FillRect(screen, &hline, SDL_MapRGB(screen->format, 187, 173, 160));
        SDL_Rect vline = { x + ((w / 4) * i) - 1, y, 2, h };
        SDL_FillRect(screen, &vline, SDL_MapRGB(screen->format, 187, 173, 160));
    }
}

void draw_tile(int col, int row, int num)
{
    int x = 40 + pad;
    int y = pad;
    int w = 240 - (pad * 2);
    int h = 240 - (pad * 2);
    x += w / 4 * col;
    y += h / 4 * row;

    int tile_pad = 2;
    int color;
    switch (num) {
    case 2:
        color = _2;
        break;
    case 4:
        color = _4;
        break;
    case 8:
        color = _8;
        break;
    case 16:
        color = _16;
        break;
    case 32:
        color = _32;
        break;
    case 64:
        color = _64;
        break;
    case 128:
        color = _128;
        break;
    case 256:
        color = _256;
        break;
    case 512:
        color = _512;
        break;
    case 1024:
        color = _1024;
        break;
    case 2048:
        color = _2048;
        break;
    default:
        color = _other;
    }
    SDL_Rect rect = { x + tile_pad, y + tile_pad, w / 4 - (tile_pad * 2), h / 4 - (tile_pad * 2) };
    draw_rect_bordered(rect, color);
    char s[10];
	sprintf(s, "%d", num);
    nSDL_DrawString(screen, font, x + w / 8 - nSDL_GetStringWidth(font, s) / 2, y + h / 8 - nSDL_GetStringHeight(font, s) / 2, s);
}

bool lost()
{
    bool lost = true;
    for (int i = 0; i < 16; i++) {
        if (tiles[i] == 0) {
            lost = false;
        }
    }
    return lost;
}

void add_tile()
{
    int value = 2;
    if ((rand() % 8) == 0) {
        value = 4;
    }
    int tile = 0;
    do {
        tile = rand() % 16;
    } while (tiles[tile] != 0);
    tiles[tile] = value;
}

void reset()
{
    for (int i = 0; i < 16; i++) {
        tiles[i] = 0;
    }
    score = 0;
    add_tile();
    add_tile();
}

void draw_tiles()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int tile = tiles[i * 4 + j];
            if (tile != 0) {
                draw_tile(j, i, tile);
            }
        }
    }
}

int* fill_zeros(int line[4])
{
Label : {
    for (int i = 0; i < 4; i++) {
        int tile = line[i];
        int next_tile = line[i + 1];
        if (i < 3 && tile == 0 && next_tile != 0) {
            line[i] = next_tile;
            line[i + 1] = 0;
            changed = true;
            goto Label;
        }
    }
}
    return line;
}
int* merge_line(int line[4])
{
    //fill zeros
    line = fill_zeros(line);
    int oldtile = 0;
    for (int i = 0; i < 4; i++) {
        int tile = line[i];
        if (oldtile != 0 && oldtile == tile) {
            line[i - 1] = oldtile * 2;
            line[i] = 0;
            tile = 0;
            changed = true;
            score += oldtile * 2;
        }
        oldtile = tile;
    }
    //fill zeros again
    line = fill_zeros(line);
    return line;
}
void left()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int i = 0; i < 4; i++) {
        int line[4];
        for (int j = 0; j < 4; j++) {
            line[j] = tiles[i * 4 + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[i * 4 + k] = mtile;
        }
    }

    if (!lost() && changed) {
		memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void right()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int i = 0; i < 4; i++) {
        int line[4];
        for (int j = 0; j < 4; j++) {
            line[j] = tiles[i * 4 + (3 - j)];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[i * 4 + (3 - k)] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void up()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int j = 0; j < 4; j++) {
        int line[4];
        for (int i = 0; i < 4; i++) {
            line[i] = tiles[i * 4 + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[k * 4 + j] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void down()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int j = 0; j < 4; j++) {
        int line[4];
        for (int i = 0; i < 4; i++) {
            line[i] = tiles[(12 - i * 4) + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[(12 - k * 4) + j] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void draw_bg()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)); //black background
}

void draw_score(int score)
{
    nSDL_DrawString(screen, font, 10, 10, "%d", score);
    const char* ver = "v1.2";
    nSDL_DrawString(screen, font, 310 - nSDL_GetStringWidth(font, ver), 10, ver);
    if(lastscore == -1) { //only display at start
        const char* cpr = "Made by\nnoverify";
        nSDL_DrawString(screen, tinyfont, 320 - nSDL_GetStringWidth(font, cpr), 230 - nSDL_GetStringHeight(font, cpr), cpr);
    }
}

void handle_keydown(SDLKey key)
{
    switch (key) {
    case SDLK_8:
    case SDLK_UP:
        up();
        break;
    case SDLK_2:
    case SDLK_DOWN:
        down();
        break;
    case SDLK_4:
    case SDLK_LEFT:
        left();
        break;
    case SDLK_6:
    case SDLK_RIGHT:
        right();
        break;
    case SDLK_ESCAPE:
        done = SDL_TRUE;
        break;
    case SDLK_BACKSPACE:
        reset();
        break;
    case SDLK_LSHIFT:
        //undo
        memcpy(tiles, last, sizeof(tiles));
        score = lastscore;
        break;
    default:
        break;
    }
}
int main(void)
{
    init();

    add_tile();
    add_tile();

    while (!done) {
        SDL_Event event;
        draw_bg();
        draw_table();
        draw_tiles();
        draw_score(score);
        SDL_Flip(screen);
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_KEYDOWN:
            handle_keydown(event.key.keysym.sym);
            break;
        default:
            break;
        }
    }
    nSDL_FreeFont(font);
    nSDL_FreeFont(tinyfont);
    quit();
    return EXIT_SUCCESS;
}
