#include <Geode/Geode.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GJItemIcon.hpp>
#include <algorithm>

using namespace geode::prelude;

// icons that can only be unlocked in 2.21
std::unordered_map<UnlockType, const std::vector<int>> BANNED_ICONS = {
  {UnlockType::Cube,   { 156, 157, 169, 170, 174, 182, 183, 184, 187, 193, 194, 195, 197, 198, 201,
                         208, 209, 210, 211, 214, 215, 216, 219, 220, 223, 224, 231, 234, 241, 243,
                         248, 251, 252, 254, 257, 258, 260, 262, 266, 277, 279, 283, 286, 288, 292,
                         297, 304, 305, 306, 307, 309, 310, 311, 320, 321, 324, 327, 332, 337, 339,
                         343, 346, 347, 351, 354, 357, 362, 363, 364, 369, 370, 372, 375, 378, 379,
                         381, 387, 390, 399, 400, 401, 409, 411, 412, 413, 416, 417, 422, 423, 424,
                         427, 430, 433, 434, 435, 439, 440, 442, 444, 445, 447, 448, 449, 451, 452,
                         454, 455, 458, 460, 462, 468, 469, 470, 472, 473, 475, 477, 479, 481, 484 }},
  {UnlockType::Ship,   { 54, 56, 57, 61, 67, 68, 74, 76, 84, 86, 92, 93, 95, 100, 101, 102, 104,
                         106, 107, 109, 113, 115, 117, 118, 119, 120, 121, 124, 128, 133, 139, 145,
                         146, 147, 149, 151, 152, 153, 154, 155, 157, 158, 159, 165, 167 }},
  {UnlockType::Ball,   { 47, 48, 50, 51, 59, 63, 73, 74, 86, 90, 91, 95, 97, 99, 100, 108, 114 }},
  {UnlockType::Bird,   { 41, 43, 44, 46, 48, 49, 50, 51, 54, 55, 57, 59, 60, 62, 63, 64, 65, 66,
                         68, 69, 70, 71, 72, 76, 82, 88, 89, 90, 92, 96, 98, 101, 105, 109, 110,
                         111, 112, 113, 114, 115, 116, 120, 122, 124, 125, 128, 129, 130, 131, 133,
                         134, 138, 139, 145, 147 }},
  {UnlockType::Dart,   { 39, 40, 44, 48, 55, 61, 67, 69, 70, 83, 86, 88, 92, 95 }},
  {UnlockType::Robot,  { 37 }},
  {UnlockType::Spider, { 27, 30, 31, 33, 37, 40, 68 }},
  {UnlockType::Swing,  { 5, 17, 24, 33 }},
};

// the vanilla max icon counts
// i *think* this mod should also work if you add custom icons (they should appear after the 2.21 icons)
std::unordered_map<UnlockType, const int> MAX_ICONS = {
  {UnlockType::Cube,   484},
  {UnlockType::Ship,   169},
  {UnlockType::Ball,   118},
  {UnlockType::Bird,   149},
  {UnlockType::Dart,    96},
  {UnlockType::Robot,   68},
  {UnlockType::Spider,  69},
  {UnlockType::Swing,   43},
};

std::unordered_map<IconType, UnlockType> ICON_TO_UNLOCK = {
  {IconType::Cube,   UnlockType::Cube},
  {IconType::Ship,   UnlockType::Ship},
  {IconType::Ball,   UnlockType::Ball},
  {IconType::Ufo,    UnlockType::Bird},
  {IconType::Wave,   UnlockType::Dart},
  {IconType::Robot,  UnlockType::Robot},
  {IconType::Spider, UnlockType::Spider},
  {IconType::Swing,  UnlockType::Swing},
};

#define ACTUAL_ICONS(unlockType) (MAX_ICONS[unlockType] - BANNED_ICONS[unlockType].size())
#define FINDFULL(container, item) (std::find(container.begin(), container.end(), item))

const std::vector<IconType> ICON_TYPES_TO_CHANGE = {
    IconType::Cube, IconType::Ship, IconType::Ball,
    IconType::Ufo, IconType::Wave, IconType::Robot,
    IconType::Spider, IconType::Swing
};

const std::vector<UnlockType> UNLOCK_TYPES_TO_CHANGE = {
    UnlockType::Cube, UnlockType::Ship, UnlockType::Ball,
    UnlockType::Bird, UnlockType::Dart, UnlockType::Robot,
    UnlockType::Spider, UnlockType::Swing
};

std::unordered_map<UnlockType, const float> LOCK_SCALES = {
  {UnlockType::Cube,   1.000/0.8},
  {UnlockType::Ship,   1.333/0.8},
  {UnlockType::Ball,   1.067/0.8},
  {UnlockType::Bird,   1.176/0.8},
  {UnlockType::Dart,   1.000/0.8},
  {UnlockType::Robot,  1.231/0.8},
  {UnlockType::Spider, 1.231/0.8},
  {UnlockType::Swing,  1.143/0.8},
};

// input: position of the item
// output: which item should be displayed
int positionToDisplay(UnlockType unlockType, int item) {
  int new_item = item;
  if (item <= ACTUAL_ICONS(unlockType))
    for (int i : BANNED_ICONS[unlockType]) if (i <= new_item) { new_item++; } else break;
  else if (item > ACTUAL_ICONS(unlockType) && item <= MAX_ICONS[unlockType])
    new_item = BANNED_ICONS[unlockType][item - ACTUAL_ICONS(unlockType) - 1];
  return new_item;
}

// input: the item that needs to be displayed
// output: the position that, when transformed with positionToDisplay, displays that item
// (inverse of positionToDisplay)
// used only to determine the location of the active icon to go to the page where it's at
int displayToPosition(UnlockType unlockType, int item) {
  if (item > MAX_ICONS[unlockType]) return item;
  auto p = FINDFULL(BANNED_ICONS[unlockType], item);
  if (p != BANNED_ICONS[unlockType].end())
    return (p - BANNED_ICONS[unlockType].begin() + 1) + ACTUAL_ICONS(unlockType);
  int new_item = item;
  for (int i : BANNED_ICONS[unlockType]) if (i < item) new_item--; else break;
  return new_item;
}

bool should_change_icon = false;

class $modify(GJItemIcon) {
  static GJItemIcon* create(UnlockType unlockType, int item, ccColor3B p2, ccColor3B p3, bool p4, bool p5, bool p6, ccColor3B p7) {
    if (FINDFULL(UNLOCK_TYPES_TO_CHANGE, unlockType) == UNLOCK_TYPES_TO_CHANGE.end() || !should_change_icon)
      return GJItemIcon::create(unlockType, item, p2, p3, p4, p5, p6, p7);
    return GJItemIcon::create(unlockType, positionToDisplay(unlockType, item), p2, p3, p4, p5, p6, p7);
  }
  // i'll handle the locks manually later because the GJItemIcon gets created before i get the chance to set the item's tag
  void changeToLockedState(float p0) {
    if (!should_change_icon) GJItemIcon::changeToLockedState(p0);
  }
};

class $modify(GJGarageLayer) {
	
  void setupPage(int page, IconType iconType) {
    if (FINDFULL(ICON_TYPES_TO_CHANGE, iconType) == ICON_TYPES_TO_CHANGE.end())	
      return GJGarageLayer::setupPage(page, iconType);

    should_change_icon = true;
    
    UnlockType unlockType = ICON_TO_UNLOCK[iconType];

    int active_icon_display = GameManager::get()->activeIconForType(iconType);
    
    if (page == -1) {
      int active_icon_position = displayToPosition(unlockType, active_icon_display);
      page = (active_icon_position - 1) / 36;
    }
    GJGarageLayer::setupPage(page, iconType);

    CCMenu *menu = getChild<CCMenu>(getChild<ListButtonPage>(getChild<ExtendedLayer>(getChild<BoomScrollLayer>(getChildOfType<ListButtonBar>(this, 0), 0), 0), 0), 0);
    
    CCSprite *cursor = static_cast<CCSprite *>(getChildByID("cursor-1"));
    // no node ids if we're entering the garage for the first time, hopefully this is always the same
    if (!cursor) cursor = getChild<CCSprite>(this, 11);

    int first_icon_on_page = page * 36 + 1;
    bool encountered_active = false;
    for (int i = 0; i < menu->getChildrenCount(); i++) {
      int selected_icon_position = first_icon_on_page + i;
      int selected_icon_display = positionToDisplay(unlockType, selected_icon_position);
      bool display_locked = (!GameManager::get()->isIconUnlocked(selected_icon_display, iconType));
      if (selected_icon_display == active_icon_display) encountered_active = true;

      CCMenuItemSpriteExtra *icon = getChild<CCMenuItemSpriteExtra>(menu, i);

      if (selected_icon_display == active_icon_display && cursor) {
        cursor->setPosition(menu->convertToWorldSpace({0, 0}) + icon->getPosition());
        cursor->setVisible(true);
      }

      GJItemIcon * gj_icon = getChild<GJItemIcon>(icon, 0);
      
      // GJItemIcon::create creates a texture of the display now, not the position
      // change the tag so that the behavior on selecting the icon matches the icon that's actually visible
      icon->setTag(selected_icon_display);
      
      // handle the lock manually
      should_change_icon = false;
      if (display_locked) gj_icon->changeToLockedState(LOCK_SCALES[unlockType]);
      should_change_icon = true;
    }
    // in case the game thinks that my cursor should be on this page, but i don't
    if (!encountered_active && cursor) cursor->setVisible(false);

    should_change_icon = false;
  }
};
