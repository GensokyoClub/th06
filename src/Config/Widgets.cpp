#include "Widgets.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

// why does sdl2 not have this?
constexpr double PI = 3.141592653589793238462643383279502884;

inline bool PointInRect(int x, int y, const SDL_Rect &rect)
{
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

LabelCache::LabelCache(TTF_Font *font) : font(font)
{
}

LabelCache::~LabelCache()
{
    clear();
}

TextureLabel LabelCache::render(SDL_Renderer *renderer, const std::string &text, SDL_Color color)
{
    CacheKey key{text, color};
    auto it = cache.find(key);
    if (it != cache.end())
    {
        return it->second;
    }

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface)
    {
        return {};
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    TextureLabel label;
    if (!texture)
    {
        SDL_FreeSurface(surface);
        return {};
    }
    label.texture = texture;
    label.rect = {0, 0, surface->w, surface->h};
    SDL_FreeSurface(surface);

    cache.emplace(key, label);
    return cache.at(key);
}

void LabelCache::clear()
{
    for (auto &entry : cache)
    {
        if (entry.second.texture)
        {
            SDL_DestroyTexture(entry.second.texture);
        }
    }
    cache.clear();
}

bool LabelCache::CacheKey::operator==(const CacheKey &other) const
{
    return text == other.text && color.r == other.color.r && color.g == other.color.g && color.b == other.color.b &&
           color.a == other.color.a;
}

std::size_t LabelCache::CacheHasher::operator()(const CacheKey &key) const
{
    std::size_t hash = std::hash<std::string>{}(key.text);
    hash = hash * 31 + key.color.r;
    hash = hash * 31 + key.color.g;
    hash = hash * 31 + key.color.b;
    hash = hash * 31 + key.color.a;
    return hash;
}

UIContext::UIContext(SDL_Renderer *renderer, TTF_Font *font, SDL_Color backgroundColor)
    : renderer(renderer), labelCache(font), backgroundColor(backgroundColor)
{
}

SDL_Color UIContext::Foreground() const
{
    return {255, 255, 255, 255};
}

SDL_Color UIContext::ButtonBg() const
{
    return {40, 40, 40, 255};
}

SDL_Color UIContext::Background() const
{
    return backgroundColor;
}

ToggleBase::ToggleBase(SDL_Rect rect, std::string label, TTF_Font *font, bool *dirtyFlag)
    : rect(rect), label(std::move(label)), font(font), dirtyFlag(dirtyFlag)
{
    if (font && TTF_SizeUTF8(font, this->label.c_str(), &labelWidth, &labelHeight) != 0)
    {
        labelWidth = 0;
        labelHeight = 0;
    }
}

SDL_Rect ToggleBase::labelRect() const
{
    SDL_Rect out = {rect.x + rect.w + 10, rect.y + (rect.h - labelHeight) / 2, labelWidth, labelHeight};
    return out;
}

void ToggleBase::markDirty()
{
    if (dirtyFlag)
    {
        *dirtyFlag = true;
    }
}

Checkbox::Checkbox(SDL_Rect rect, std::string label, bool initial, TTF_Font *font, bool *dirtyFlag)
    : ToggleBase(rect, std::move(label), font, dirtyFlag), checked(initial)
{
}

void Checkbox::handleEvent(const SDL_Event &e)
{
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
    {
        return;
    }

    const int x = e.button.x;
    const int y = e.button.y;
    SDL_Rect labelBounds = labelRect();
    if (PointInRect(x, y, rect) || PointInRect(x, y, labelBounds))
    {
        setCheckedState(!checked, true);
    }
}

void Checkbox::render(UIContext &ctx)
{
    SET_DRAW_COLOR(ctx.renderer, ctx.Foreground());
    SDL_RenderDrawRect(ctx.renderer, &rect);

    if (checked)
    {
        constexpr int inset = 3;
        const int innerWidth = rect.w - 2 * inset;
        const int innerHeight = rect.h - 2 * inset;
        const SDL_Rect innerRect = {rect.x + inset, rect.y + inset, innerWidth, innerHeight};
        SDL_RenderFillRect(ctx.renderer, &innerRect);
    }

    auto labelTexture = ctx.labelCache.render(ctx.renderer, label, ctx.Foreground());
    labelTexture.rect.x = rect.x + rect.w + 10;
    labelTexture.rect.y = rect.y + (rect.h - labelTexture.rect.h) / 2;
    SDL_RenderCopy(ctx.renderer, labelTexture.texture, nullptr, &labelTexture.rect);
}

bool Checkbox::isChecked() const
{
    return checked;
}

void Checkbox::setCheckedState(bool value, bool notifyDirty)
{
    if (checked == value)
        return;
    checked = value;
    if (notifyDirty)
    {
        markDirty();
    }
}

RadioButton::RadioButton(SDL_Rect rect, std::string label, int groupId, int optionId, bool initial, TTF_Font *font,
                         bool *dirtyFlag)
    : ToggleBase(rect, std::move(label), font, dirtyFlag), selected(initial), groupId(groupId), optionId(optionId)
{
}

void RadioButton::handleEvent(const SDL_Event &e)
{
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
    {
        return;
    }

    const int mx = e.button.x;
    const int my = e.button.y;
    SDL_Rect labelBounds = labelRect();
    if ((PointInRect(mx, my, rect) || PointInRect(mx, my, labelBounds)) && !selected)
    {
        if (onSelected)
        {
            onSelected(groupId, this);
        }
        else
        {
            setSelectedState(true, true);
        }
    }
}

void RadioButton::render(UIContext &ctx)
{
    SET_DRAW_COLOR(ctx.renderer, ctx.Foreground());

    const int cx = rect.x + rect.w / 2;
    const int cy = rect.y + rect.h / 2;
    const int radius = std::max(2, rect.w / 2 - 1);
    for (double angle = 0.0; angle < 2.0 * PI; angle += PI / 90.0)
    {
        int x = std::lround(cx + radius * std::cos(angle));
        int y = std::lround(cy + radius * std::sin(angle));
        SDL_RenderDrawPoint(ctx.renderer, x, y);
    }

    if (selected)
    {
        const int innerRadius = std::max(2, radius - 3);
        for (int dy = -innerRadius; dy <= innerRadius; ++dy)
        {
            for (int dx = -innerRadius; dx <= innerRadius; ++dx)
            {
                if (dx * dx + dy * dy <= innerRadius * innerRadius)
                {
                    SDL_RenderDrawPoint(ctx.renderer, cx + dx, cy + dy);
                }
            }
        }
    }

    auto labelTexture = ctx.labelCache.render(ctx.renderer, label, ctx.Foreground());
    SDL_Rect dest = labelTexture.rect;
    dest.x = rect.x + rect.w + 10;
    dest.y = rect.y + (rect.h - dest.h) / 2;
    SDL_RenderCopy(ctx.renderer, labelTexture.texture, nullptr, &dest);
}

void RadioButton::setSelectedState(bool value, bool notifyDirty)
{
    if (selected == value)
        return;
    selected = value;
    if (notifyDirty)
    {
        markDirty();
    }
}

void RadioButton::setOnSelected(std::function<void(int, RadioButton *)> handler)
{
    onSelected = std::move(handler);
}

Button::Button(SDL_Rect rect, std::string label, std::function<void()> onClick)
    : rect(rect), label(std::move(label)), onClick(std::move(onClick))
{
}

void Button::handleEvent(const SDL_Event &e)
{
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
    {
        return;
    }
    const int mx = e.button.x;
    const int my = e.button.y;
    if (PointInRect(mx, my, rect) && onClick)
    {
        onClick();
    }
}

void Button::render(UIContext &ctx)
{
    SET_DRAW_COLOR(ctx.renderer, ctx.ButtonBg());
    SDL_RenderFillRect(ctx.renderer, &rect);
    SET_DRAW_COLOR(ctx.renderer, ctx.Foreground());
    SDL_RenderDrawRect(ctx.renderer, &rect);

    auto labelTexture = ctx.labelCache.render(ctx.renderer, label, ctx.Foreground());
    SDL_Rect dest = labelTexture.rect;
    dest.x = rect.x + (rect.w - dest.w) / 2;
    dest.y = rect.y + (rect.h - dest.h) / 2;
    SDL_RenderCopy(ctx.renderer, labelTexture.texture, nullptr, &dest);
}

void GroupBox::render(UIContext &ctx) const
{
    SDL_Renderer *renderer = ctx.renderer;

    SET_DRAW_COLOR(renderer, ctx.Foreground());

    SDL_RenderDrawRect(renderer, &rect);

    auto labelTexture = ctx.labelCache.render(renderer, label, ctx.Foreground());
    SDL_Rect dest = labelTexture.rect;
    dest.x = rect.x + 18;
    dest.y = rect.y - dest.h / 2;

    const int gapPadding = 4;
    SDL_Rect gapRect = {dest.x - gapPadding, rect.y - dest.h / 2, dest.w + gapPadding * 2, dest.h};
    SET_DRAW_COLOR(renderer, ctx.Background());
    SDL_RenderFillRect(renderer, &gapRect);

    SET_DRAW_COLOR(renderer, ctx.Foreground());

    SDL_RenderDrawLine(renderer, rect.x, rect.y, gapRect.x, rect.y);
    SDL_RenderDrawLine(renderer, gapRect.x + gapRect.w, rect.y, rect.x + rect.w, rect.y);

    SDL_RenderCopy(renderer, labelTexture.texture, nullptr, &dest);
}
