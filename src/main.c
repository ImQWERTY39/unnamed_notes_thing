#include <SDL2/SDL.h>
#include <stdint.h>

#include "../include/document.h"
#include "../include/fileio.h"
#include "../include/tiles.h"

#define QUIT 0b00000001
#define MOUSE_BUTTON_DOWN 0b00000010
#define JUST_LEFT_MOUSE 0b00000100
#define TO_PEN_MODE 0b00001000
#define TO_ERASER_MODE 0b00010000
#define TO_PAN_MODE 0b00100000
#define TO_LINE_MODE 0b01000000

const int32_t SCREEN_WIDTH = 1600;
const int32_t SCREEN_HEIGHT = 1000;

const int32_t SIDEBAR_WIDTH = 100;
const int32_t FOOTER_HEIGHT = 100;

const int32_t CANVAS_WIDTH = SCREEN_WIDTH - SIDEBAR_WIDTH;
const int32_t CANVAS_HEIGHT = SCREEN_HEIGHT - FOOTER_HEIGHT;

const int32_t STYLUS_WIDTH = 40;

typedef enum {
    PEN = 0,
    ERASER,
    PAN,
    LINE,
} Mode;

typedef struct {
    uint8_t qualifiers;
    Mode mode;

    Uint32 last_sample_time;

    Point last_sample_pos;
    Point top_left_corner;
} EventState;

void poll_event(EventState* event_state) {
    SDL_Event event;
    event_state->qualifiers &= ~JUST_LEFT_MOUSE;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            event_state->qualifiers |= QUIT;
            break;

        case SDL_MOUSEBUTTONUP:
            event_state->qualifiers &= ~MOUSE_BUTTON_DOWN;
            event_state->qualifiers |= JUST_LEFT_MOUSE;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.x >= CANVAS_WIDTH) {
                switch (event.button.y) {
                case 0 ... SCREEN_HEIGHT / 4:
                    event_state->mode = PEN;
                    break;
                case SCREEN_HEIGHT / 4 + 1 ... SCREEN_HEIGHT / 2:
                    event_state->mode = ERASER;
                    break;
                case SCREEN_HEIGHT / 2 + 1 ... 3 * SCREEN_HEIGHT / 4:
                    event_state->mode = PAN;
                    break;
                default:
                    event_state->mode = LINE;
                    break;
                }

                event_state->last_sample_pos.x = event.button.x;
                event_state->last_sample_pos.y = event.button.y;
            } else if (event.button.button == SDL_BUTTON_LEFT) {
                event_state->qualifiers |= MOUSE_BUTTON_DOWN;
                event_state->qualifiers &= ~JUST_LEFT_MOUSE;
                event_state->last_sample_time = SDL_GetTicks();
                event_state->last_sample_pos.x = event.button.x;
                event_state->last_sample_pos.y = event.button.y;
            }
            break;
        }
    }
}

void draw_line_global(Document* doc, Point start, Point end) {
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = (start.x < end.x) ? 1 : -1;
    int sy = (start.y < end.y) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        Point tile = tile_coords(start);
        Tile* t = get_tile(doc, tile, CREATE_MISSING);

        uint8_t lx = (uint8_t)(start.x & 0xFF);
        uint8_t ly = (uint8_t)(start.y & 0xFF);
        set(t, lx, ly);

        if (start.x == end.x && start.y == end.y)
            break;

        int e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            start.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            start.y += sy;
        }
    }
}

void pen(EventState* event_state, Document* document) {
    Point mouse_coords;
    SDL_GetMouseState(&mouse_coords.x, &mouse_coords.y);

    Point prev_global_coords = event_state->last_sample_pos;
    Point global_coords = mouse_coords;

    prev_global_coords.x += event_state->top_left_corner.x;
    prev_global_coords.y += event_state->top_left_corner.y;
    global_coords.x += event_state->top_left_corner.x;
    global_coords.y += event_state->top_left_corner.y;

    draw_line_global(document, prev_global_coords, global_coords);
    event_state->last_sample_pos = mouse_coords;

    if (document->length >= HASHTABLE_CAPACITY) {
        flush_document(document);
        load_file(document, event_state->top_left_corner);
    }
}

void line(EventState* event_state, Document* doc) {
    Point prev_global_coords = event_state->last_sample_pos;
    Point global_coords;
    SDL_GetMouseState(&global_coords.x, &global_coords.y);

    if (prev_global_coords.x == global_coords.x && prev_global_coords.y == global_coords.y)
        return;

    prev_global_coords.x += event_state->top_left_corner.x;
    prev_global_coords.y += event_state->top_left_corner.y;
    global_coords.x += event_state->top_left_corner.x;
    global_coords.y += event_state->top_left_corner.y;

    draw_line_global(doc, prev_global_coords, global_coords);
}

void erase(EventState* es, Document* doc) {
    Point global_coords;
    SDL_GetMouseState(&global_coords.x, &global_coords.y);
    global_coords.x += es->top_left_corner.x;
    global_coords.y += es->top_left_corner.y;

    Point cur;
    for (int dy = -STYLUS_WIDTH; dy <= STYLUS_WIDTH; dy++) {
        for (int dx = -STYLUS_WIDTH; dx <= STYLUS_WIDTH; dx++) {
            cur.x = global_coords.x + dx;
            cur.y = global_coords.y + dy;

            Tile* t = get_tile(doc, tile_coords(cur), 0);
            if (t == NULL) {
                continue;
            }

            clear(t, (uint8_t)(cur.x & 0xFF), (uint8_t)(cur.y & 0xFF));
        }
    }
}

void pan(Document* document, EventState* event_state) {
    Point screen_coords;
    SDL_GetMouseState(&screen_coords.x, &screen_coords.y);

    int32_t delta_x = screen_coords.x - event_state->last_sample_pos.x;
    int32_t delta_y = screen_coords.y - event_state->last_sample_pos.y;

    event_state->top_left_corner.x -= delta_x;
    event_state->top_left_corner.y -= delta_y;

    event_state->last_sample_pos = screen_coords;
    load_file(document, event_state->top_left_corner);
}

void render(
    SDL_Renderer* renderer,
    EventState* es,
    Document* document,
    uint32_t* framebuffer,
    SDL_Texture* texture
) {
    memset(framebuffer, 0xFFFFFFFF, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

    int32_t tile_start_x = floor_div(es->top_left_corner.x);
    int32_t tile_end_x = floor_div(es->top_left_corner.x + CANVAS_WIDTH);

    int32_t tile_start_y = floor_div(es->top_left_corner.y);
    int32_t tile_end_y = floor_div(es->top_left_corner.y + CANVAS_HEIGHT);

    int32_t offset_x = es->top_left_corner.x % TILE_SIZE;
    if (offset_x < 0)
        offset_x += TILE_SIZE;

    int32_t offset_y = es->top_left_corner.y % TILE_SIZE;
    if (offset_y < 0)
        offset_y += TILE_SIZE;

    // use the metadata checking thing to see if the file exists
    // if any found, load locality
    for (int ty = tile_start_y; ty <= tile_end_y; ty++) {
        for (int tx = tile_start_x; tx <= tile_end_x; tx++) {
            Point tile_coords = {tx, ty};
            Tile* t = get_tile(document, tile_coords, IGNORE);

            if (t == NULL) {
                continue;
            }

            for (int r = 0; r < TILE_SIZE; r++) {
                int sy = (ty - tile_start_y) * TILE_SIZE + r - offset_y;
                if (sy < 0 || sy >= CANVAS_HEIGHT)
                    continue;

                for (int c = 0; c < TILE_SIZE; c++) {
                    if (t->map[r][c / 64] & MSB_SHIFT(c % 64)) {
                        int sx = (tx - tile_start_x) * TILE_SIZE + c - offset_x;
                        if (sx < 0 || sx >= CANVAS_WIDTH)
                            continue;

                        framebuffer[sy * SCREEN_WIDTH + sx] = 0xFF000000;
                    }
                }
            }
        }
    }

    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "NotesApp",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        return -2;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    EventState event_state = {0};
    Document document = {0};
    document.last_key = 0xFFFFFFFFFFFFFFFF;
    load_file(&document, event_state.top_left_corner);

    uint32_t* framebuffer = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint32_t));
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT
    );

    while (1) {
        poll_event(&event_state);
        if (event_state.qualifiers & QUIT) {
            break;
        }

        if (event_state.qualifiers & MOUSE_BUTTON_DOWN) {
            switch (event_state.mode) {
            case PEN:
                pen(&event_state, &document);
                break;

            case PAN:
                pan(&document, &event_state);
                break;

            case ERASER:
                erase(&event_state, &document);
                break;

            default:
                break;
            }
        }

        if (event_state.mode == LINE && event_state.qualifiers & JUST_LEFT_MOUSE) {
            line(&event_state, &document);
        }

        render(renderer, &event_state, &document, framebuffer, texture);
    }

    flush_document(&document);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(framebuffer);
    SDL_DestroyTexture(texture);

    return 0;
}
