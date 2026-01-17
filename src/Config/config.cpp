#include <SDL2/SDL.h>
#include <SDL2/SDL_messagebox.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <utility>
#include <vector>

#include "../Supervisor.hpp"
#include "../inttypes.hpp"
#include "Widgets.hpp"

constexpr SDL_Color kBackgroundColor{43, 43, 43, 255};

namespace FileSystem
{
unsigned char *OpenPath(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    const size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto *data = (unsigned char *)malloc(size);
    if (!data)
    {
        fclose(file);
        return nullptr;
    }

    if (fread(data, 1, size, file) != size)
    {
        free(data);
        fclose(file);
        return nullptr;
    }

    fclose(file);
    return data;
}

int WriteDataToFile(const char *path, void *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (!f)
    {
        return -1;
    }

    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return -2;
    }

    fclose(f);
    return 0;
}
} // namespace FileSystem

class ConfigUI
{
  public:
    ConfigUI(GameConfiguration &cfg, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, const char *cfgPath)
        : cfg(cfg), window(window), renderer(renderer), font(font), configPath(cfgPath),
          ctx(renderer, font, kBackgroundColor)
    {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        buildLayout();
    }

    void run()
    {
        while (!shouldExit)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    requestQuit();
                    continue;
                }

                for (auto &binding : checkboxBindings)
                {
                    binding.widget.handleEvent(event);
                }
                for (auto &binding : radioBindings)
                {
                    binding.widget.handleEvent(event);
                }
                for (auto &button : bottomButtons)
                {
                    button.handleEvent(event);
                }
            }

            SET_DRAW_COLOR(renderer, ctx.Background());

            SDL_RenderClear(renderer);

            for (auto &binding : checkboxBindings)
            {
                binding.widget.render(ctx);
            }
            for (auto &binding : radioBindings)
            {
                binding.widget.render(ctx);
            }
            for (const auto &box : groupBoxes)
            {
                box.render(ctx);
            }
            for (auto &button : bottomButtons)
            {
                button.render(ctx);
            }

            SDL_RenderPresent(renderer);
        }
    }

  private:
    struct CheckboxBinding
    {
        Checkbox widget;
        int bit;

        CheckboxBinding(Checkbox &&widget, int bit) : widget(std::move(widget)), bit(bit)
        {
        }
    };

    struct RadioBinding
    {
        RadioButton widget;
        int groupId;
        int optionId;

        RadioBinding(RadioButton &&widget, int groupId, int optionId)
            : widget(std::move(widget)), groupId(groupId), optionId(optionId)
        {
        }
    };

    GameConfiguration &cfg;
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    const char *configPath;
    UIContext ctx;

    bool hasUnsavedChanges = false;
    bool shouldExit = false;
    int windowWidth = 0;
    int windowHeight = 0;

    std::vector<CheckboxBinding> checkboxBindings;
    std::vector<RadioBinding> radioBindings;
    std::vector<GroupBox> groupBoxes;
    std::vector<Button> bottomButtons;

    void buildLayout()
    {
        buildCheckboxes();
        buildRadioGroups();
        buildBottomButtons();
    }

    void buildCheckboxes()
    {
        struct CheckboxSpec
        {
            const char *label;
            int bit;
        };

        const CheckboxSpec specs[11] = {
            {"No Vertex Buffer", GCOS_DONT_USE_VERTEX_BUF},
            {"No Fog", GCOS_DONT_USE_FOG},
            {"16 Bit Textures", GCOS_FORCE_16BIT_COLOR_MODE},
            {"Suppress use of gouraud shading", GCOS_SUPPRESS_USE_OF_GOROUD_SHADING},
            {"Don't apply color composition", GCOS_NO_COLOR_COMP},
            {"Start in reference rasterizer mode", GCOS_REFERENCE_RASTERIZER_MODE},
            {"Clear back buffer on refresh", GCOS_CLEAR_BACKBUFFER_ON_REFRESH},
            {"Display minimum graphics needed", GCOS_DISPLAY_MINIMUM_GRAPHICS},
            {"No Depth test", GCOS_TURN_OFF_DEPTH_TEST},
            {"Force 60 FPS", GCOS_FORCE_60FPS},
            {"No DirectInput pad", GCOS_NO_DIRECTINPUT_PAD},
        };

        const int checkboxCount = sizeof(specs) / sizeof(specs[0]);
        checkboxBindings.reserve(checkboxCount);

        int top = 16;
        for (const auto &spec : specs)
        {
            SDL_Rect rect{15, top, 16, 16};
            bool initial = (cfg.opts >> spec.bit) & 1;
            checkboxBindings.emplace_back(Checkbox(rect, spec.label, initial, font, &hasUnsavedChanges), spec.bit);
            top += 24;
        }
    }

    void buildRadioGroups()
    {
        struct RadioGroupSpec
        {
            const char *label;
            std::vector<const char *> options;
            int spacing;
            int groupId;
            int initialIndex;
        };

        const RadioGroupSpec radioGroup[]{{"Screen Mode", {"Fullscreen", "Windowed"}, 130, 0, cfg.windowed},
                                          {"Refresh Rate", {"1/1", "1/2", "1/3"}, 70, 1, cfg.frameskipConfig},
                                          {"Colors", {"32-bit", "16-bit"}, 120, 2, cfg.colorMode16bit}};

        int totalRadioButtons = 0;
        for (const auto &group : radioGroup)
        {
            totalRadioButtons += static_cast<int>(group.options.size());
        }
        radioBindings.reserve(totalRadioButtons);

        constexpr int boxWidth = 360;
        constexpr int boxHeight = 56;
        constexpr int left = 15;
        int top = 16 + checkboxBindings.size() * 24 + 20;

        for (const auto &group : radioGroup)
        {
            SDL_Rect boxRect{left, top, boxWidth, boxHeight};
            groupBoxes.push_back({boxRect, group.label});

            const int buttonY = boxRect.y + (boxRect.h / 2) - 8;
            int buttonX = boxRect.x + 18;

            for (int i = 0; i < group.options.size(); ++i)
            {
                SDL_Rect rbRect{buttonX, buttonY, 16, 16};
                bool selected = (i == group.initialIndex);
                radioBindings.emplace_back(
                    RadioButton{rbRect, group.options[i], group.groupId, i, selected, font, &hasUnsavedChanges},
                    group.groupId, i);
                RadioButton &radio = radioBindings.back().widget;
                radio.setOnSelected(
                    [&](int groupId, RadioButton *selectedButton) { onRadioSelected(groupId, selectedButton); });

                buttonX += group.spacing;
            }

            top += boxHeight + 12;
        }
    }

    void buildBottomButtons()
    {
        const int buttonWidth = 100;
        const int buttonHeight = 32;
        const int buttonSpacing = 12;
        const int margin = 20;

        const int top = windowHeight - margin - buttonHeight;
        const int saveX = windowWidth - margin - buttonWidth;
        const int cancelX = saveX - buttonSpacing - buttonWidth;

        bottomButtons.reserve(2);

        bottomButtons.emplace_back(SDL_Rect{cancelX, top, buttonWidth, buttonHeight}, "Cancel", [&] { requestQuit(); });

        bottomButtons.emplace_back(SDL_Rect{saveX, top, buttonWidth, buttonHeight}, "Save", [&] {
            if (saveConfig())
            {
                shouldExit = true;
            }
        });
    }

    void onRadioSelected(int groupId, RadioButton *selectedButton)
    {
        for (auto &binding : radioBindings)
        {
            if (binding.groupId != groupId)
            {
                continue;
            }

            bool shouldSelect = (&binding.widget == selectedButton);
            binding.widget.setSelectedState(shouldSelect, shouldSelect);
            if (!shouldSelect)
            {
                binding.widget.setSelectedState(false, false);
            }
        }
    }

    void requestQuit()
    {
        shouldExit = true;
    }

    void applyUiToConfig()
    {
        cfg.opts = (1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING);

        for (const auto &binding : checkboxBindings)
        {
            if (binding.widget.isChecked())
            {
                cfg.opts |= (1 << binding.bit);
            }
        }

        auto getSelectedOption = [&](int groupId) -> int {
            for (const auto &binding : radioBindings)
            {
                if (binding.groupId == groupId && binding.widget.isSelected())
                {
                    return binding.optionId;
                }
            }
            return 0;
        };

        cfg.windowed = getSelectedOption(0);
        cfg.frameskipConfig = getSelectedOption(1);
        cfg.colorMode16bit = getSelectedOption(2);
    }

    bool saveConfig()
    {
        applyUiToConfig();
        if (FileSystem::WriteDataToFile(configPath, &cfg, sizeof(GameConfiguration)) != 0)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Save Failed", "Could not write configuration file.",
                                     window);
            return false;
        }
        hasUnsavedChanges = false;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Save Successful", "Configuration saved successfully.",
                                 window);
        printf("Saved config to %s\n", configPath);
        return true;
    }
};

int main()
{
    const char *configPath = "東方紅魔郷.cfg";

    GameConfiguration cfg;
    if (const auto data = FileSystem::OpenPath(configPath))
    {
        memcpy(&cfg, data, sizeof(GameConfiguration));
        free(data);
        cfg.windowed = std::min<u8>(cfg.windowed, 1);
        cfg.frameskipConfig = std::min<u8>(cfg.frameskipConfig, 2);
        cfg.colorMode16bit = std::min<u8>(cfg.colorMode16bit, 1);
    }
    else
    {
        cfg.version = GAME_VERSION;
        cfg.lifeCount = 2;
        cfg.bombCount = 3;
        cfg.colorMode16bit = 0;
        cfg.musicMode = 1;
        cfg.playSounds = 1;
        cfg.defaultDifficulty = 1;
        cfg.windowed = 0;
        cfg.frameskipConfig = 0;
        cfg.padXAxis = 600;
        cfg.padYAxis = 600;
        cfg.opts = (1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0)
    {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("Touhou 6 Config", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 450, 600, SDL_WINDOW_SHOWN);

    if (!window)
    {
        printf("Failed to create window: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("NotoSans-Regular.ttf", 18);
    if (!font)
    {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    ConfigUI ui = {cfg, window, renderer, font, configPath};
    ui.run();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
