#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>


#define SET_DRAW_COLOR(renderer, color) \
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

struct TextureLabel {
    SDL_Rect rect{};
    SDL_Texture *texture = nullptr;
};

class LabelCache {
public:
    explicit LabelCache(TTF_Font *font);
    ~LabelCache();

    TextureLabel render(SDL_Renderer *renderer, const std::string &text, SDL_Color color);
    void clear();

private:
    struct CacheKey {
        std::string text;
        SDL_Color color;

        bool operator==(const CacheKey& other) const;
    };

    struct CacheHasher {
        std::size_t operator()(const CacheKey& key) const;
    };

    TTF_Font *font;
    std::unordered_map<CacheKey, TextureLabel, CacheHasher> cache;
};

class UIContext {
public:
    UIContext(SDL_Renderer *renderer, TTF_Font* font, SDL_Color backgroundColor);

    SDL_Color Foreground() const;
    SDL_Color ButtonBg() const;
    SDL_Color Background() const;

    SDL_Renderer *renderer;
    LabelCache labelCache;
    SDL_Color backgroundColor;
};

class Widget {
public:
    virtual ~Widget() = default;
    virtual void handleEvent(const SDL_Event &e) = 0;
    virtual void render(UIContext &ctx) = 0;
};

class ToggleBase : public Widget {
public:
    ToggleBase(SDL_Rect rect, std::string label, TTF_Font *font, bool *dirtyFlag);
    ~ToggleBase() override = default;

protected:
    SDL_Rect labelRect() const;
    void markDirty();

    SDL_Rect rect;
    std::string label;
    TTF_Font *font;
    bool *dirtyFlag;
    int labelWidth = 0;
    int labelHeight = 0;
};

class Checkbox : public ToggleBase {
public:
    Checkbox(SDL_Rect rect, std::string label, bool initial, TTF_Font *font, bool *dirtyFlag);

    void handleEvent(const SDL_Event &e) override;
    void render(UIContext &ctx) override;

    bool isChecked() const;
    void setCheckedState(bool value, bool notifyDirty);

private:
    bool checked;
};

class RadioButton : public ToggleBase {
public:
    RadioButton(SDL_Rect rect, std::string label, int groupId, int optionId, bool initial, TTF_Font *font, bool *dirtyFlag);

    void handleEvent(const SDL_Event &e) override;
    void render(UIContext &ctx) override;

    void setSelectedState(bool value, bool notifyDirty);
    bool isSelected() const {
        return selected;
    };
    int getGroupId() const {
        return groupId;
    };
    int getOptionId() const {
        return optionId;
    };

    void setOnSelected(std::function<void(int, RadioButton*)> handler);

private:
    bool selected;
    int groupId;
    int optionId;
    std::function<void(int, RadioButton*)> onSelected;
};

class Button : public Widget {
public:
    Button(SDL_Rect rect, std::string label, std::function<void()> onClick);

    void handleEvent(const SDL_Event &e) override;
    void render(UIContext &ctx) override;

private:
    SDL_Rect rect;
    std::string label;
    std::function<void()> onClick;
};

struct GroupBox {
    SDL_Rect rect;
    std::string label;

    void render(UIContext& ctx) const;
};
