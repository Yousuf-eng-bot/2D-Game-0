# Death World 0.2 — গবেষণা থেকে বাস্তব পরিবর্তন

## যে প্রকাশ্য উৎসগুলো পড়া হয়েছে

- Blizzard-এর Diablo IV VFX/combat আলোচনা: animated damage areas, swing-এর সঙ্গে damage timing মেলানো, impact-এর দিক এবং অনেক effects থাকলেও combat পড়তে পারা। [1](https://news.blizzard.com/en-us/article/23746639/diablo-iv-quarterly-updatedecember-2021)
- Rockstar-এর GTA V controls guide: weapon নির্বাচন, বিভিন্ন weapon slot এবং দ্রুত switching-এর প্রকাশ্য control design। [1](https://www.rockstargames.com/newswire/article/51974aa3a724o2/rockstar-game-tips-tailoring-your-settings-and-controls-in)
- Rockstar-এর GTA VI official site এবং Only in Leonida-তে Jason Duval ও Lucia Caminos-এর প্রকাশিত character/world presentation। [2](https://www.rockstargames.com/VI) [3](https://www.rockstargames.com/VI/only-in-leonida)
- Rockstar-এর প্রকাশিত বক্তব্য অনুযায়ী Trailer 2-তে in-game gameplay এবং cutscene দুটোই আছে। তাই cinematic দৃশ্যকে স্বাধীনভাবে যাচাই করা gameplay mechanic হিসেবে ধরে নেওয়া হয়নি। [3](https://x.com/RockstarGames/status/1920181314092765494)

## এই ছোট 2D গেমে কী প্রয়োগ করা হয়েছে

### Character movement

GTA VI-এর চরিত্র বা proprietary animation system কপি করা হয়নি। প্রকাশ্য character presentation থেকে শরীরের ভর, পোশাকের silhouette এবং পরিবেশে চরিত্রের উপস্থিতিকে design goal হিসেবে নেওয়া হয়েছে। ছোট pixel character-এর জন্য নিজস্ব procedural animation তৈরি করা হয়েছে:

- বাস্তবে যত দূর হাঁটা হয়, সেই দূরত্ব অনুযায়ী stride phase।
- আলাদা পা, বুট, হাত, torso, মাথা এবং scarf movement।
- সামনের/পেছনের মুখভঙ্গি, বাম/ডান facing এবং attack lean।
- Analog acceleration, দ্রুত braking এবং normalized diagonal speed।
- Camera follow, guardian fight-এ camera bias, dash afterimages ও dust।

### Combat feedback

- Attack চাপার মুহূর্তেই damage নয়: Sword 0.10 s, Axe 0.24 s, Bow 0.22 s startup-এর পর payload।
- এক swing-এ একই শত্রুর repeated frame damage বন্ধ।
- Sword combo finisher, ধীর কিন্তু ভারী Axe, projectile-based Bow।
- অস্ত্রভেদে Whirlwind, Earthquake বা পাঁচ-arrow Volley।
- Directional knockback, hit flash, impact particles, brief hit-stop, crit text।
- Player damage-এ recoil, colored edge flash ও system-respecting haptic feedback।
- Cooldown এড়াতে weapon-switch exploit বন্ধ।

### World identity

Dark dungeon-এর বদলে bright daytime biomes: সবুজ Sunveil Wilds এবং ochre/cactus/stone-ভিত্তিক Emberfall Reach। নিজস্ব গাছপালা, নদী, কাঠের bridge, পাথর, ঘাস/ফুল, ruins, shrine chest, deer/rabbit/lizard এবং পাখি যোগ করা হয়েছে। রাস্তা ও গুরুত্বপূর্ণ জায়গা ইচ্ছাকৃতভাবে সংযুক্ত; encounters ও loot seed-ভেদে বদলায়। এটি সম্পূর্ণ random open-world generator নয়।

Title artwork নতুন করে AI-assisted generation দিয়ে তৈরি; gameplay sprites, tile/prop drawing ও animation C++-এ নিজস্বভাবে আঁকা। কোনো Rockstar বা Blizzard ছবি, model, soundtrack বা চরিত্র গেমে অন্তর্ভুক্ত করা হয়নি।

### Boss difficulty

শুধু HP বাড়ানো হয়নি। CrownHorn ও Solkar-এর তিনটি phase, locked-direction charge, warning-before-damage strikes, eruption circles, safe-gap shockwave, projectile fan, guardian-specific leap এবং phase-change minions আছে। Heavy hits stagger তৈরি করে; stagger-এর পর resistance থাকায় স্থায়ী stun-lock সম্ভব নয়।

Standing-and-attacking test bot দুটো isolated guardian encounter-এ হেরেছে। Perfect-information tactical bot normal movement, attacks, consumables এবং collected equipment দিয়ে জিতেছে। এগুলো mechanical checks; মানুষের কাছে গেম কতটা কঠিন বা মজার, তার চূড়ান্ত প্রমাণ নয়। ফোনের feedback দিয়েই পরবর্তী tuning করা উচিত।
