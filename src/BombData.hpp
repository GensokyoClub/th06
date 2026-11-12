#pragma once
#include "Player.hpp"

namespace th06
{
struct BombData
{
    BombCallback calc;
    BombCallback draw;
};

void BombReimuACalc(Player *);
void BombReimuBCalc(Player *);
void BombMarisaACalc(Player *);
void BombMarisaBCalc(Player *);
void BombReimuADraw(Player *);
void BombReimuBDraw(Player *);
void BombMarisaADraw(Player *);
void BombMarisaBDraw(Player *);

} // namespace th06
