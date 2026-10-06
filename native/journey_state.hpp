#pragma once
namespace av {
enum Material {
  LOG,
  STICK,
  STONE,
  COAL,
  ORE,
  IRON,
  PLANK,
  TORCH,
  WALL,
  DOOR,
  BENCH,
  FURNACE,
  CHEST,
  BED,
  BRIDGE,
  SEED,
  GRAIN,
  DIRT,
  MATERIALS
};
struct GroundStack {
  double x = 0, y = 0;
  int kind = 0, count = 0;
  float z = 8, vz = 55, age = 0;
};
struct Structure {
  int kind = 0;
  bool open = false;
  float progress = 0;
  std::array<int, MATERIALS> storage{};
};
struct Journey {
  std::array<int, MATERIALS> stock{};
  std::array<int, 3> toolTier{0, 0, 0}, durability{0, 0, 0};
  std::map<std::pair<int64_t, int64_t>, int> mined, rockHP;
  std::map<std::pair<int64_t, int64_t>, Structure> built;
  std::vector<GroundStack> drops;
  std::array<uint8_t, MW * MH> heights{};
  bool home = false;
  int64_t homeX = 0, homeY = 0;
  float dodgeQueue = 0, toolAnim = 0, toolAim = 0, pose = 0, restTime = 0,
        z = 0, vz = 0, landing = 0, guard = 0, guardCd = 0, parry = 0,
        counter = 0, queue = 0, toolCd = 0, stanceCd = 0;
  bool active = false, heavy = false, queuedHeavy = false, emitted = false;
  int jumpStartHeight = 0, stance = 0, combo = 0, guardFinger = -1,
      recipePage = 0, selectedBuild = BENCH;
  float elapsed = 0, windup = 0, activeTime = 0, recovery = 0, aim = 0,
        previousAngle = 0, noise = 0;
  double safeX = 0, safeY = 0;
  std::set<uint64_t> struck;
  std::vector<std::pair<float, float>> trail;
} j;
void fireArrow(float, float, float, int, int);
void drawJourneyControls();
std::string encodeJourney();
bool decodeJourney(const std::string &, Journey &);
void modifyJourneyTile(int64_t, int64_t, uint8_t &, uint8_t &, uint8_t &);
void refreshJourneyTerrain();
int landHeight(int64_t, int64_t);
bool terrainLink(int, int);
bool journeyMoveAllowed(float, float, float, float, float, bool);
void dropMaterial(float, float, int, int);
void beginStrike(bool);
void journeyPreStep(float);
void journeyStrikeStep(float);
bool defendHit(int &, float, float);
void jumpPlayer();
void cycleStance();
void beginGuard();
void harvest();
void harvestPayload();
void digGround();
void journeyInteract();
void drawJourneyTerrain();
void drawJourneyObjects();
void drawJourneyStructure(const std::pair<int64_t, int64_t> &,
                          const Structure &);
void drawJourneyDrops();
void drawJourneyHud();
void drawJourneyPanel();
void drawJourneyRanger(int, int, float, bool);
bool journeyTouch(float, float, int);
float strikeAngle(float);
float stanceSpeed();
float stealthFactor();
bool enemyAttackSlot();
} // namespace av
