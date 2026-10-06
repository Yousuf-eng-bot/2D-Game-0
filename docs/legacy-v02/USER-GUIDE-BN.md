# Death World 0.2 — Daybreak Update

আপনার আগের কাজ করা Ashen Veil প্রোটোটাইপের বড় আপডেট। নতুন নাম **Death World**। এটি এখনো নিজস্ব 2D pixel-art গেম—GTA/Diablo-এর ছবি বা চরিত্র কপি করা হয়নি।

## সবচেয়ে গুরুত্বপূর্ণ: আগের অ্যাপ মুছবেন না

1. **Death-World-0.2.apk** ডাউনলোড করুন।
2. ফাইল খুলে **Update / Install** চাপুন। **পুরোনো Ashen Veil আগে uninstall করবেন না।**
3. একই app ID ও একই signing certificate রাখা হয়েছে। ইনস্টলের পর অ্যাপের নাম ও icon বদলে Death World হবে।
4. আগের level, XP, gold, forge upgrade, inventory ও পরা gear নতুন save system-এ নেওয়ার ব্যবস্থা আছে।
5. আগের মতোই চলমান expedition-এর অবস্থান সেভ হয় না; আবার খুললে camp থেকে শুরু হবে।

Android 6.0 বা পরের সংস্করণের জন্য তৈরি; ARM64, ARMv7 ও x86_64 build অন্তর্ভুক্ত। সব ফোনে পরীক্ষা করা হয়নি। নতুন করে ইনস্টল করতে source ZIP বা SDK দরকার নেই।

## ১. চরিত্র ও combat-এর পরিবর্তন

- পা ও হাতের আলাদা walking motion, চলার গতির সঙ্গে stride, বুটের পদক্ষেপ, torso/head facing এবং scarf-এর নড়াচড়া।
- Joystick-এর analog movement, acceleration, দ্রুত থামা, diagonal speed ঠিক রাখা এবং smoother camera follow।
- এখন আক্রমণের animation-এর নির্দিষ্ট মুহূর্তে damage হয়। কুঠার ভারী বলে তার আঘাত আসতে একটু সময় লাগে।
- Hit flash, directional knockback, impact particles, ছোট hit-stop, critical damage text এবং hurt recoil।
- Damage পেলে চরিত্রে flash/recoil ও screen edge feedback; ফোনের system settings অনুমতি দিলে হালকা haptic।
- Dodge-এর **দুটি charge**। একবারে একটি করে recharge হয়। Dash-এর পেছনে afterimage দেখা যায়।

## ২. তিনটি অস্ত্র—নিজে বেছে নিন

খেলার নিচে **BLADE / CHANGE**, **AXE / CHANGE** বা **BOW / CHANGE** চাপলে Armory খুলবে। Title screen ও Pause থেকেও Armory খোলা যায়।

| অস্ত্র | খেলার ধরন | Power বোতাম |
|---|---|---|
| **Dawnblade** | দ্রুত তিন-আঘাতের combo; শেষ আঘাত শক্তিশালী | চারপাশে Whirlwind / Sweep |
| **Rift Axe** | ধীর, ভারী damage; guard/stagger-এর বিরুদ্ধে কার্যকর | বড় Earthquake |
| **Wind Bow** | দূর থেকে arrow; কিছু shot শত্রু ভেদ করতে পারে | পাঁচ-arrow Volley |

সবগুলো প্রথম থেকেই খোলা। Inventory-এর **Weapon Core** তিন ধরনের অস্ত্রের base attack বাড়ায়। পুরোনো weapon item হারায় না; এখন সেটি সেই core হিসেবে ব্যবহৃত হয়।

## ৩. দুটি উজ্জ্বল দিনের অঞ্চল

Camp-এ **CHOOSE A WORLD** চাপুন।

### Sunveil Wilds

সবুজ বন, নদী, কাঠের bridge, গাছপালা, ঘাস/ফুল, deer/rabbit, shrine ও ruins। এখানকার guardian **CrownHorn**—charge, cleave, eruption ও shockwave-এর জন্য প্রস্তুত থাকুন।

### Emberfall Reach

সোনালি canyon, পাথর, cactus/acacia ধরনের গাছ, সংকীর্ণ নদী এবং lizard। এখানকার guardian **Solkar, the Sunforged Colossus**—projectile volley, leap impact ও ground hazards ব্যবহার করে।

রাস্তাগুলো সংযুক্ত রাখা হয়েছে। পানি পার হতে bridge ব্যবহার করুন। Mini-map-এ tap করলে বড় map খুলবে। Region layout নির্দিষ্টভাবে তৈরি; encounter placement ও loot seed-ভেদে বদলায়।

## ৪. প্রতিটি অভিযানে কী করবেন

1. তিনটি লাল shrine marker-এর কাছে থাকা শত্রুদের হারান।
2. Shrine clear হলে chest খুলতে তার কাছে যান—gold, gear এবং একটি flask charge পেতে পারেন।
3. তিনটি shrine clear হলে guardian জেগে উঠবে। তার আগে guardian shielded থাকে।
4. গোল্ড marker ধরে boss arena-তে যান।
5. Boss হারিয়ে **CLAIM VICTORY** চাপুন। Camp-এ ফিরে upgrade ও নতুন expedition নিন।

প্রথম অঞ্চলে প্রথম run-এ boss-সহ প্রায় ৩১টি foe, দ্বিতীয়টিতে ৩৭টি থাকে; boss পরে minion ডাকতে পারে। পাঁচ ধরনের সাধারণ enemy archetype এবং empowered variants আছে।

## ৫. বসকে হারানোর কৌশল

- **লাল warning মানেই নড়ার সময়।** শুধু সামনে দাঁড়িয়ে ATTACK ধরে রাখবেন না।
- Charge-এর সামনে থেকে পাশে সরে যান; dodge-এর direction আগে joystick দিয়ে ঠিক করুন।
- Ground circle ভরে ওঠার আগেই বেরিয়ে যান।
- Shockwave-এ **সবুজ safe gap** আছে; gap ধরুন বা ঠিক সময়ে dodge করুন।
- Solkar লাফ দিলে তার landing circle ছাড়ুন।
- Heavy attack-এ stagger bar ভরে। Boss staggered হলে সুযোগ নিয়ে বেশি damage দিন।
- Stagger-এর পর resistance থাকে—একটানা stun-lock ধরে রাখা যায় না।
- Boss-এর তিনটি phase আছে; HP কমলে নতুন pressure/minion আসে।

আরও কঠিন চাইলে World Selection-এর নিচে **DIFFICULTY: VETERAN** বেছে নিন।

## ৬. কন্ট্রোল

- বাঁ joystick: হাঁটা/চলা; joystick-এর কেন্দ্র touch-এর কাছাকাছি বসে।
- **ATTACK ধরে রাখা:** নির্বাচিত অস্ত্রে আক্রমণ।
- **SWEEP / QUAKE / VOLLEY:** অস্ত্রভেদে power skill।
- **DODGE:** দুই charge-এর evade।
- **WARD:** বৃত্তের ভেতরে থাকলে damage কমে; enemy ধীর হয়।
- Flask/বোতল: ৫০% max HP ফেরত দেয়; cooldown আছে।
- **BAG:** item বাছুন → EQUIP অথবা SALVAGE।
- **II / Back:** pause, sound/shake toggle, battery mode, armory, guide।

## ৭. পারফরম্যান্স

ফোন গরম হলে বা ধীর লাগলে Pause → **BATTERY MODE: ON (30)** দিন। Off অবস্থায় 60 FPS লক্ষ্য করে frame scheduling করা হয়; এটি আপনার ফোনে মাপা বা নিশ্চিত FPS নয়।

গেম সম্পূর্ণ অফলাইন। বিজ্ঞাপন, payment, account, tracking বা Internet permission নেই। Title illustration ও sound assets APK-তেই আছে।

## ৮. পরীক্ষা কতটুকু হয়েছে

- APK তিন architecture-এ build, signature ও alignment যাচাই হয়েছে। পুরোনো APK-এর certificate ও app ID একই, versionCode ১ থেকে ২ হয়েছে।
- C++ core-এ ২৯,৪২২টি assertion পাস করেছে; sanitizer test-এ সমস্যা পাওয়া যায়নি।
- দুই অঞ্চল × তিন অস্ত্র × তিন encounter seed—১৮টি input-only bot run-এ অভিযান শেষ হয়েছে। Bot সাধারণ movement, skill, healing ও সংগ্রহ করা gear ব্যবহার করেছে; এটি মানব difficulty test নয়।
- আলাদা stationary boss fixture-এ শুধু দাঁড়িয়ে attack করা bot দুটো guardian-এর কাছেই হেরেছে।
- Android 9 x86_64 emulator-এ পুরোনো 0.1-এর ওপর update installation সফল হয়েছে এবং নতুন title artwork চালু হয়েছে। Emulator-এর নিজের System UI সমস্যায় পূর্ণ touch পরীক্ষা নির্ভরযোগ্যভাবে শেষ করা যায়নি।
- Emulator পরীক্ষার পরে overlapping arrow-এর একটি C++ bug ঠিক করে পুরো native test আবার চালানো হয়েছে। Final APK-র এই সংশোধিত native binary emulator-এ আবার ইনস্টল করা হয়নি; Android bridge ও update identity অপরিবর্তিত আছে।
- **এই নতুন build বাস্তব ফোনে আমার পক্ষ থেকে পরীক্ষা হয়নি।** আগের build আপনার ফোনে চলেছে—নতুনটির feedback বিশেষভাবে মূল্যবান হবে।

## ৯. পরের feedback

ফোনের মডেল/Android versionসহ বলুন:

- হাঁটা ও attack-এর অনুভূতি কেমন?
- কোন অস্ত্র সবচেয়ে ভালো লাগল?
- CrownHorn/Solkar খুব সহজ, নাকি অতিরিক্ত কঠিন?
- FPS কমে কি, ফোন গরম হয় কি?
- পুরোনো gold/gear এসেছে কি?

সমস্যা হলে screenshot/video দিন। Install mismatch হলে save বাঁচাতে আগে uninstall না করে error message পাঠান।
