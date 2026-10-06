#pragma once
namespace av {
bool ui7Active();
void ui7Render();
void ui7Hud();
bool ui7Touch(int action, int id, float x, float y);
void ui7Boot();
void ui7InputClear();
void ui7Step(float dt);
bool ui7Back();
int ui7TextWidth(const std::string &, int);
void ui7WorldText(int, int, const std::string &, C, int);
} // namespace av
