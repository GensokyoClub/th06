#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>

class Checkbox {
public:
    Checkbox(int x, int y, int size, const std::string& text, TTF_Font* font) {
        this->rect = { x, y, size, size };
        this->label = text;
        this->font = font;
        this->checked = false;
    }

    void handleEvent(const SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mx = e.button.x;
            int my = e.button.y;
            if (mx >= rect.x && mx <= rect.x + rect.w &&
                my >= rect.y && my <= rect.y + rect.h) {
                checked = !checked;
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);

        if (checked) {
            SDL_Rect inner {
                rect.x + 4,
                rect.y + 4,
                rect.w - 8,
                rect.h - 8
            };
            SDL_RenderFillRect(renderer, &inner);
        }

        SDL_Color white{255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, label.c_str(), white);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect{
            rect.x + rect.w + 10,
            rect.y + (rect.h - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    bool isChecked() const { return checked; }

private:
    SDL_Rect rect;
    std::string label;
    TTF_Font* font;
    bool checked;
};

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow(
        "Touhou 6 Config",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        // zun why
        400, 510,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("msgothic.ttc", 16);
    if (!font) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    std::vector<Checkbox> checkboxes;

    int height = 16;
    checkboxes.push_back({ 15, height, 16, "No Vertex Buffer", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "No Fog", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "16 Bit Textures", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Controlled use of gouraud shading", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Color composition is not applied to textures", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Start in reference rasterizer mode", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Clear back buffer on refresh", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Display minimum graphics needed", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "No Depth test", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "Force 60 FPS", font });
    height += 24;
    checkboxes.push_back({ 15, height, 16, "No DirectInput pad", font });
    height += 24;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;

            for (auto& checkbox : checkboxes) {
                checkbox.handleEvent(e);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (auto& checkbox : checkboxes) {
            checkbox.render(renderer);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
