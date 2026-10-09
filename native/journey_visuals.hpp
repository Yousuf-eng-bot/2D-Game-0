#pragma once
namespace av {
void drawJourneyRanger(int x, int y, float phase, bool moving) {
  // Baked layered sprites when the character atlas shipped with the build;
  // otherwise fall through to the original procedural ranger so the look is
  // preserved on any build without the asset.
  if (spriteHeroReady()) {
    float a = j.active ? j.aim : std::atan2(g.fy, g.fx);
    int fy = y - int(j.z);
    ellipse(x, y + 2, std::max(6, 13 - int(j.z / 8)), 4, 0xff364133);
    int flash = g.hurtTime > 0 ? int(std::min(1.f, g.hurtTime / .22f) * 190) : 0;
    if (g.hurtTime > 0 && int(g.time * 28) % 2 == 0)
      flash = std::min(255, flash + 60);
    int kx = x, ky = fy;
    charKnockback(g.hurtTime, g.hurtx, g.hurty, kx, ky);
    charLunge(a, kx, ky);
    drawCharActor(playerOutfit(), playerAnim, a, kx, ky, PAL_PLAYER, flash, 255,
                  nullptr, true);
    drawBodyMarks(kx, ky);
    drawActorDust(x, y, a);
    return;
  }
  float face = j.active ? j.aim : std::atan2(g.fy, g.fx), fx = std::cos(face),
        fy = std::sin(face), rx = -fy, ry = fx;
  float prone = clamp(j.pose - 1, 0, 1), crouch = clamp(j.pose, 0, 1),
        cycle = g.walkPhase * .65f;
  float bob =
      moving ? std::abs(std::sin(cycle)) * 1.4f : std::sin(g.time * 2) * .35f;
  float action =
      j.active
          ? std::sin(clamp(j.elapsed / (j.windup + j.activeTime), 0, 1) * PI)
          : 0;
  float lean = action * (j.heavy ? 7 : 4);
  int rootY = y - int(j.z) - int(bob * (1 - prone));
  ellipse(x, y + 2, std::max(6, 13 - int(j.z / 8)), 4, 0xff364133);
  C outline = 0xff222e2b, cloth = 0xff526756, leather = 0xff977752,
    skin = 0xffc4a580, steel = 0xffb6c7bb;
  if (g.hurtTime > .18f) {
    cloth = mix(cloth, WHITE, 90);
    leather = mix(leather, WHITE, 100);
  }
  auto pt = [&](float side, float forward, float height) {
    return std::pair<int, int>{
        x + int(rx * side + fx * forward),
        rootY + int(ry * side * .55f + fy * forward * .65f - height)};
  };
  auto limb = [&](std::pair<int, int> a, std::pair<int, int> b, C c,
                  int width) {
    thickLine(a.first, a.second, b.first, b.second, outline, width + 1);
    thickLine(a.first, a.second, b.first, b.second, c, width);
    line(a.first - 1, a.second, b.first - 1, b.second, mix(c, WHITE, 25));
  };
  auto hip = pt(0, lean - 8 * prone, 15 - 6 * crouch - 6 * prone);
  auto shoulder = pt(0, lean + 11 * prone, 33 - 11 * crouch - 15 * prone);
  auto head = pt(0, lean + 22 * prone, 44 - 13 * crouch - 24 * prone);
  for (int order = 0; order < 2; order++) {
    int side = (fy < 0 ? 1 : -1) * (order ? 1 : -1);
    float a = cycle + (side < 0 ? PI : 0),
          u = std::fmod(a / (2 * PI) + 10, 1.f);
    float foot =
        moving ? (u < .6f ? 10 - 20 * u / .6f : -10 + 20 * (u - .6f) / .4f) : 0;
    float lift = moving && u > .6f ? std::sin((u - .6f) / .4f * PI) * 5 : 0;
    auto pelvis = pt(side * 4, lean - 8 * prone, 15 - 6 * crouch - 6 * prone);
    auto knee =
        pt(side * 5, foot * .5f - 12 * prone + 3 * crouch, 8 - 3 * prone);
    auto boot = pt(side * 6, foot - 18 * prone, 2 + lift * (1 - prone));
    limb(pelvis, knee, 0xff515a48, 3);
    limb(knee, boot, 0xff62583e, 2);
    ellipse(boot.first + int(fx * 2), boot.second, 5, 3, outline);
    line(boot.first - 3, boot.second - 1, boot.first + 3, boot.second - 1,
         0xffad946a);
  }
  // Cloth follows the shoulders while the body rotates through a strike.
  int sway = int(std::sin(cycle * .7f + g.time * .5f) * (moving ? 3 : 1));
  poly({{shoulder.first - 10, shoulder.second - 2},
        {shoulder.first + 8, shoulder.second - 2},
        {hip.first + 11 + sway, hip.second + 8},
        {hip.first - 12 + sway, hip.second + 5}},
       outline);
  poly({{shoulder.first - 8, shoulder.second},
        {shoulder.first + 6, shoulder.second},
        {hip.first + 8 + sway, hip.second + 6},
        {hip.first - 9 + sway, hip.second + 3}},
       cloth);
  line(shoulder.first - 7, shoulder.second + 2, hip.first - 7 + sway,
       hip.second + 3, 0xffa3ad87);
  limb(hip, shoulder, leather, 7);
  line(shoulder.first - 5, shoulder.second + 4, hip.first + 4, hip.second - 3,
       0xffc0a57b);
  line(hip.first - 8, hip.second, hip.first + 8, hip.second, outline);
  rect(hip.first - 2, hip.second - 2, 4, 4, GOLD);
  float a = j.active ? strikeAngle(j.elapsed) : face - .65f;
  if (j.toolAnim > 0) {
    float t = 1 - j.toolAnim / .4f;
    a = j.toolAim - 1.4f + 2.5f * easeJourney(t);
  }
  float extension = j.active ? (j.heavy ? 29 : 20) : 9;
  int hx = x + int(std::cos(a) * extension + fx * lean),
      hy = rootY - 18 + int(std::sin(a) * extension) - int(10 * (1 - crouch));
  auto other = pt(-9, lean + (moving ? std::sin(cycle) * 3 : 0),
                  19 - 7 * crouch - 6 * prone);
  auto armRoot = std::make_pair(shoulder.first + int(rx * 7),
                                shoulder.second + int(ry * 4));
  if (j.guard > 0) {
    hx = x + int(fx * 18);
    hy = rootY - 24 + int(fy * 12);
    a = face + PI / 2;
  }
  if (prone > .5f) {
    hx = shoulder.first + int(fx * 15);
    hy = shoulder.second + int(fy * 8);
  }
  auto elbow = std::make_pair((armRoot.first + hx) / 2 - int(fy * 4),
                              (armRoot.second + hy) / 2 + 5);
  limb(std::make_pair(shoulder.first - int(rx * 7),
                      shoulder.second - int(ry * 4)),
       other, leather, 2);
  limb(armRoot, elbow, leather, 3);
  limb(elbow, {hx, hy}, leather, 2);
  rect(hx - 2, hy - 2, 4, 4, skin);
  rect(other.first - 2, other.second - 2, 4, 4, skin);
  ellipse(head.first, head.second - 3, 6, 7, outline);
  ellipse(head.first, head.second - 2, 5, 6, fy < -.45f ? 0xff71634b : skin);
  rect(head.first - 6, head.second - 9, 11, 4, 0xff494936);
  line(head.first - 5, head.second - 10, head.first + 3, head.second - 10,
       0xffb1b393);
  if (fy >= -.45f) {
    int eye = head.first + int(fx * 3);
    point(eye, head.second - 3, outline);
    point(eye + int(rx * 3), head.second - 3 + int(ry * 2), outline);
    rect(head.first - 3, head.second + 1, 7, 2, 0xff78684e);
  } else
    line(head.first - 4, head.second - 3, head.first + 3, head.second - 3,
         0xffa99673);
  if (prone < .6f) {
    if (j.toolAnim > 0) {
      float ux = std::cos(a), uy = std::sin(a), vx = -uy, vy = ux;
      thickLine(hx, hy, hx + int(ux * 28), hy + int(uy * 28), 0xffa08553, 2);
      int bx = hx + int(ux * 27), by = hy + int(uy * 27);
      thickLine(bx - int(vx * 12), by - int(vy * 12), bx + int(vx * 12),
                by + int(vy * 12), steel, 2);
    } else if (g.weapon == BOW) {
      weaponArt(hx, hy, BOW, j.active ? j.aim : face,
                j.active ? clamp(j.elapsed / j.windup, 0, 1) : 0);
    } else {
      // Longer articulated grip; tip is on the same sweep angle used for hit
      // checks.
      float ux = std::cos(a), uy = std::sin(a), vx = -uy, vy = ux;
      int length = g.weapon == AXE ? 39 : 37;
      thickLine(hx - int(ux * 5), hy - int(uy * 5), hx + int(ux * length),
                hy + int(uy * length), outline, 2);
      thickLine(hx, hy, hx + int(ux * length), hy + int(uy * length),
                g.weapon == AXE ? 0xffa08553 : steel, 1);
      if (g.weapon == AXE) {
        int bx = hx + int(ux * 34), by = hy + int(uy * 34);
        poly({{bx + int(vx * 11 - ux * 5), by + int(vy * 11 - uy * 5)},
              {bx + int(vx * 13 + ux * 6), by + int(vy * 13 + uy * 6)},
              {bx - int(vx * 9) + int(ux * 6), by - int(vy * 9) + int(uy * 6)},
              {bx - int(vx * 9) - int(ux * 4), by - int(vy * 9) - int(uy * 4)}},
             steel);
      } else {
        line(hx + int(ux * 10 + vx), hy + int(uy * 10 + vy),
             hx + int(ux * 38 + vx), hy + int(uy * 38 + vy), WHITE);
        thickLine(hx + int(ux * 6 - vx * 6), hy + int(uy * 6 - vy * 6),
                  hx + int(ux * 6 + vx * 6), hy + int(uy * 6 + vy * 6), GOLD,
                  1);
      }
    }
  }
  if (j.active && g.weapon != BOW && j.elapsed >= j.windup &&
      j.elapsed < j.windup + j.activeTime + .035f) {
    float recent = strikeAngle(std::max(j.windup, j.elapsed - .045f));
    float now = strikeAngle(std::min(j.elapsed, j.windup + j.activeTime));
    if (recent > now)
      std::swap(recent, now);
    softArc(x, rootY - 24, g.weapon == AXE ? 61 : 58, recent, now, 0xffede2b3,
            j.heavy ? 160 : 110);
  }
  if (j.counter > 0)
    circle(x, rootY - 53, 3, GOLD, true);
  if (j.guard > 0)
    arc(x, rootY - 20, 24, face - .9f, face + .9f, j.parry > 0 ? WHITE : TEAL,
        2);
  drawBodyMarks(hip.first, rootY);
}
void drawJourneyTerrain() {
  int x0 = std::max(0, int(g.camx / T) - 1),
      y0 = std::max(0, int(g.camy / T) - 1);
  for (int y = y0; y < std::min(MH - 1, y0 + H / T + 3); y++)
    for (int x = x0; x < std::min(MW - 1, x0 + W / T + 3); x++) {
      int i = y * MW + x, h = j.heights[i], xx = sx(x * T), yy = sy(y * T);
      auto dug = j.mined.find({o.originX + x, o.originY + y});
      if (dug != j.mined.end() && dug->second == 2) {
        shade(xx, yy, T, T, 0xff756448, 160);
        for (int r = 0; r < 3; r++)
          line(xx + 4, yy + 5 + r * 6, xx + 19, yy + 5 + r * 6, 0xff5b5641);
      }
      if (g.map[i] == 4)
        continue;
      int down = int(j.heights[i]) - int(j.heights[i + MW]),
          left = int(j.heights[i]) - int(j.heights[i + 1]);
      if (h > 0)
        shade(xx, yy, T, T, 0xffd3d1b8, h * 4);
      if (down > 0) {
        int depth = std::min(18, down * 10 + 4);
        rect(xx, yy + T - depth, T, depth, 0xff57584a);
        for (int r = 0; r < depth; r += 3)
          line(xx, yy + T - depth + r, xx + 23, yy + T - depth + r,
               mix(0xff727565, INK, r * 7));
        line(xx, yy + T - depth, xx + 23, yy + T - depth, 0xffb3b596);
        line(xx, yy + T - 1, xx + 23, yy + T - 1, 0xff323e35);
        if (terrainLink(i, i + MW)) {
          for (int k = 0; k < 4; k++)
            rect(xx + 6, yy + T - depth + k * 3, 12, 2, 0xff939584);
        }
      }
      if (left > 0) {
        rect(xx + 20, yy, 4, T, 0xff53594d);
        line(xx + 19, yy, xx + 19, yy + 22, 0xffa4aa8d);
      }
    }
}
void drawJourneyStructure(const std::pair<int64_t, int64_t> &key,
                          const Structure &b) {
  int x = sx(float(key.first - o.originX) * T + 12),
      y = syAt(float(key.first - o.originX) * T + 12,
               float(key.second - o.originY) * T + 12);
  if (x < -45 || x > W + 45 || y < -15 || y > H + 65)
    return;
  C timber = 0xff8a7350, edge = 0xffb8a481, dark = 0xff3a4036;
  switch (b.kind) {
  case DIRT:
    box(x - 12, y - 14, 24, 25, 0xff77654b, 0xff3d4538);
    rect(x - 11, y - 15, 22, 6, 0xff8d9a6e);
    line(x - 10, y - 16, x + 10, y - 16, 0xffb1b994);
    break;
  case WALL:
  case DOOR: {
    int width = b.kind == DOOR && b.open ? 4 : 24;
    box(x - 12, y - 35, width, 43, timber, dark);
    for (int k = 0; k < width; k += 6)
      line(x - 10 + k, y - 33, x - 10 + k, y + 6, edge);
    rect(x - 12, y - 29, width, 3, dark);
    rect(x - 12, y - 1, width, 3, dark);
    if (b.kind == DOOR)
      rect(x + 5, y - 16, 3, 3, GOLD);
    break;
  }
  case BENCH:
    box(x - 16, y - 20, 32, 13, timber, dark);
    rect(x - 14, y - 7, 4, 14, dark);
    rect(x + 10, y - 7, 4, 14, dark);
    line(x - 11, y - 21, x + 4, y - 24, edge);
    rect(x + 3, y - 26, 8, 6, 0xffa7b2a5);
    break;
  case FURNACE:
    box(x - 13, y - 28, 26, 34, 0xff787e74, dark);
    for (int k = 0; k < 5; k++)
      line(x - 11, y - 25 + k * 6, x + 10, y - 25 + k * 6, 0xffa5aa98);
    box(x - 7, y - 12, 14, 14, 0xff343931, dark);
    if (b.storage[ORE] > 0) {
      rect(x - 5, y - 6, 10, 6, 0xffc88b48);
      rect(x - 1, y - 9, 3, 9, GOLD);
    }
    break;
  case CHEST:
    box(x - 14, y - 17, 28, 24, timber, dark);
    rect(x - 12, y - 10, 24, 3, dark);
    rect(x - 7, y - 16, 3, 22, edge);
    rect(x + 5, y - 16, 3, 22, edge);
    rect(x - 2, y - 8, 5, 6, GOLD);
    break;
  case BED:
    box(x - 12, y - 24, 24, 34, timber, dark);
    rect(x - 10, y - 21, 20, 8, 0xffcec6a6);
    rect(x - 10, y - 12, 20, 19, 0xff647974);
    line(x - 9, y - 8, x + 8, y - 8, 0xff8ea898);
    break;
  case TORCH:
    thickLine(x, y + 3, x, y - 18, timber, 2);
    ellipse(x, y - 22, 4, 6, 0xffcc974a);
    ellipse(x, y - 24 + int(std::sin(g.time * 8)), 2, 4, 0xffedda97);
    break;
  case 18:
    for (int a = -7; a <= 7; a += 7) {
      int size = 5 + int(b.progress / 15);
      line(x + a, y + 4, x + a, y - size, 0xff819358);
      if (b.progress >= 180)
        rect(x + a - 2, y - size, 4, 7, GOLD);
    }
    break;
  default:
    break;
  }
}
void drawJourneyObjects() {
  for (auto &[key, b] : j.built)
    drawJourneyStructure(key, b);
}
C materialColor(int kind) {
  return kind == ORE     ? 0xffb88b68
         : kind == COAL  ? 0xff434743
         : kind == IRON  ? 0xffbdc6bc
         : kind == STONE ? 0xff8c9487
         : kind == GRAIN ? GOLD
         : kind == SEED  ? TEAL
                         : 0xffb18a58;
}
void drawJourneyDrops() {
  for (auto &d : j.drops) {
    int x = sx(float((d.x - o.originX) * T)),
        y = syAt(float((d.x - o.originX) * T), float((d.y - o.originY) * T));
    if (x < -15 || x > W + 15 || y < -15 || y > H + 20)
      continue;
    ellipse(x, y + 2, 6, 2, 0xff384637);
    int bob = y - int(d.z) - 7 - int(std::sin(g.time * 3 + d.x) * 2);
    C c = materialColor(d.kind);
    box(x - 5, bob - 4, 10, 8, c, INK);
    line(x - 3, bob - 3, x + 2, bob - 3, mix(c, WHITE, 65));
    if (d.count > 1)
      text(x + 6, bob - 2, num(d.count), WHITE);
  }
}
void journeyControl(int x, int y, int r, const std::string &label,
                    bool active = false) {
  circle(x, y, r, 0xff173b39, true);
  circle(x, y, r, active ? WHITE : GOLD);
  circle(x, y, r - 3, 0xff688578);
  center(x, y - 3, label, WHITE);
}
void drawJourneyControls() {
  int x = g.joystick >= 0 ? int(g.joyx) : 76,
      y = g.joystick >= 0 ? int(g.joyy) : 282;
  circle(x, y, 42, 0xff5b877b);
  circle(x, y, 28, 0xff6f9a84);
  circle(x + int(g.mx * 24), y + int(g.my * 24), 16, 0xff244d50, true);
  circle(x + int(g.mx * 24), y + int(g.my * 24), 16, 0xffb6d4ab);
  center(76, 337, "MOVE", WHITE);
  button(145, 317, 117, 28,
         std::string(g.weapon == 0   ? "BLADE"
                     : g.weapon == 1 ? "AXE"
                                     : "BOW") +
             " / CHANGE");
  if (g.scene != PLAY)
    return;
  journeyControl(587, 293, 34, "LIGHT", j.active && !j.heavy);
  journeyControl(518, 263, 26, "HEAVY", j.active && j.heavy);
  journeyControl(519, 325, 24, "DODGE");
  journeyControl(585, 218, 23, "GUARD", j.guard > 0);
  journeyControl(462, 324, 22, num(g.flasks));
  journeyControl(448, 259, 25, "JUMP", j.z > 0);
  button(167, 240, 98, 26, "MINE / TOOL");
  button(167, 275, 98, 28,
         j.stance == 0   ? "CROUCH"
         : j.stance == 1 ? "LIE DOWN"
                         : "STAND UP");
  button(278, 276, 115, 27, "INTERACT");
  for (int k = 0; k < 2; k++)
    rect(511 + k * 11, 295, 7, 4, k < g.dodgeCharges ? TEAL : EDGE);
}
void drawJourneyHud() {
  if (g.scene != PLAY)
    return;
  button(435, 72, 92, 23, "CRAFT / BUILD");
  if (j.counter > 0) {
    panel(224, 106, 185, 17);
    center(316, 111, "COUNTER WINDOW", GOLD);
  }
  if (j.active) {
    int width = int(72 * j.elapsed / g.attackLength);
    rect(sx(g.px) - 36, syAt(g.px, g.py) + 12, 72, 3, INK);
    rect(sx(g.px) - 36, syAt(g.px, g.py) + 12, width, 3,
         j.elapsed < j.windup                  ? GOLD
         : j.elapsed < j.windup + j.activeTime ? WHITE
                                               : TEAL);
  }
}
void drawJourneyPanel() {
  dim();
  box(28, 18, 584, 324, PANEL, GOLD);
  text(45, 33, "CRAFT / BUILD / SURVIVE", GOLD, 2);
  button(565, 27, 29, 25, "X");
  text(45, 61,
       "PACK " + num(occupiedSlots()) + "/24 STACKS  /  PICK TIER " +
           num(j.toolTier[0]) + "  USES " + num(j.durability[0]),
       DIM);
  for (int a = 0; a < MATERIALS; a++) {
    int col = a % 6, row = a / 6;
    int x = 44 + col * 94, y = 79 + row * 26;
    rect(x, y, 5, 5, materialColor(a));
    text(x + 8, y, materialName(a), DIM);
    text(x + 8, y + 10, num(materialCount(a)), WHITE);
  }
  int start = j.recipePage * 4;
  for (int a = 0; a < 4; a++) {
    int k = start + a;
    if (k >= int(recipes().size()))
      break;
    auto &r = recipes()[k];
    int y = 165 + a * 29;
    button(45, y, 252, 26, r.name);
    std::string cost;
    for (int c = 0; c < 3; c++)
      if (r.amount[c]) {
        if (!cost.empty())
          cost += " + ";
        cost += num(r.amount[c]) + " " + materialName(r.kind[c]);
      }
    text(309, y + 4, cost, DIM);
    text(309, y + 15,
         r.station ? std::string("NEAR ") + materialName(r.station)
                   : "HAND CRAFT",
         GOLD);
  }
  button(45, 288, 60, 23, "PREV");
  button(110, 288, 60, 23, "NEXT");
  button(177, 288, 160, 23,
         std::string("PLACE ") + materialName(j.selectedBuild));
  button(344, 288, 100, 23, "CYCLE TYPE");
  button(451, 288, 130, 23, "RECLAIM");
  button(45, 316, 106, 20, "SMELT ORE");
  button(158, 316, 106, 20, "STORE ALL");
  button(271, 316, 106, 20, "TAKE ALL");
  button(384, 316, 96, 20, "PLANT");
  button(487, 316, 94, 20, "SKILL");
  if (g.toastTime > 0) {
    panel(80, 139, 480, 20);
    center(320, 145, g.toast.substr(0, 76), GOLD);
  }
}
bool journeyTouch(float x, float y, int id) {
  if (!g.openWorld)
    return false;
  auto in = [&](int xx, int yy, int ww, int hh) {
    return x >= xx && y >= yy && x < xx + ww && y < yy + hh;
  };
  if (g.overlay == 10) {
    if (in(565, 27, 29, 25)) {
      g.overlay = 0;
      return true;
    }
    for (int a = 0; a < 4; a++)
      if (in(45, 165 + a * 29, 252, 26))
        craftRecipe(j.recipePage * 4 + a);
    if (in(45, 288, 60, 23))
      j.recipePage = std::max(0, j.recipePage - 1);
    if (in(110, 288, 60, 23))
      j.recipePage = std::min(int(recipes().size() - 1) / 4, j.recipePage + 1);
    if (in(177, 288, 160, 23)) {
      placeStructure();
      g.overlay = 0;
    }
    if (in(344, 288, 100, 23))
      j.selectedBuild = j.selectedBuild == BRIDGE ? DIRT
                        : j.selectedBuild == DIRT ? TORCH
                                                  : j.selectedBuild + 1;
    if (in(451, 288, 130, 23)) {
      reclaimStructure();
      g.overlay = 0;
    }
    if (in(45, 316, 106, 20))
      smeltOre();
    if (in(158, 316, 106, 20))
      storeMaterials(false);
    if (in(271, 316, 106, 20))
      storeMaterials(true);
    if (in(384, 316, 96, 20)) {
      plantSeed();
      g.overlay = 0;
    }
    if (in(487, 316, 94, 20)) {
      g.overlay = 0;
      skill(0);
    }
    return true;
  }
  if (g.overlay || g.scene != PLAY)
    return false;
  if (in(435, 72, 92, 23)) {
    clearInput();
    g.overlay = 10;
    return true;
  }
  if (len(x - 448, y - 259) < 26) {
    jumpPlayer();
    return true;
  }
  if (len(x - 518, y - 263) < 28) {
    beginStrike(true);
    return true;
  }
  if (len(x - 585, y - 218) < 25) {
    beginGuard();
    j.guardFinger = id;
    return true;
  }
  if (in(167, 240, 98, 26)) {
    harvest();
    return true;
  }
  if (in(167, 275, 98, 28)) {
    cycleStance();
    return true;
  }
  if (in(278, 276, 115, 27)) {
    journeyInteract();
    return true;
  }
  return false;
}
} // namespace av
