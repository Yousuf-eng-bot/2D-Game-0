# Death World ০.৮ — Vista graphics update

**OBSIDIAN GAMES · owned by OBSIDIAN SYNDICATE**  
৫ অক্টোবর ২০২৬ · Android 6.0 বা নতুন · সংস্করণ 0.8.0

## ইনস্টল

`Death-World-0.8.apk` আগের গেমের ওপর **Update** হিসেবে ইনস্টল করুন। **Uninstall বা Clear storage/data করবেন না**—স্থানীয় জগৎ হারাতে পারেন। একই package ও আগের signing certificate ব্যবহার করা হয়েছে।

APK খোলার browser/file manager-এর install permission Android চাইতে পারে। প্রয়োজন হলে সেটির জন্য অনুমতি দিন, পরে বন্ধ করতে পারেন। Play Protect/antivirus বন্ধ করার পরামর্শ দেওয়া হচ্ছে না। Install conflict হলে পুরোনো অ্যাপ মুছে না দিয়ে error-এর screenshot দিন।

## পরিবর্তনগুলো দেখুন

- **গাছপালা:** পাতার নতুন গুচ্ছ, নতুন pine/conifer গঠন, নয়টি বিদ্যমান tree species-এর আটটি করে cosmetic variant ও biome palette।
- **মাটি:** material texture clusters, রাস্তার অসমান প্রান্ত, বালি/তুষার/জলাভূমি/আগ্নেয়ভূমির আলাদা detail।
- **গভীরতা:** আগে থেকে থাকা terrain height-এর নতুন projection, পাথরের স্তর, cliff edge ও step। চরিত্র, বস্তু ও drop সেই elevation অনুযায়ী দেখা যায়।
- **পরিবেশ:** fern, ঝোপ, grass/reed, ছোট ফুল, গুঁড়ি, ভাঙা ডাল, ছোট পাথর, mushroom ও litter-এর decorative pattern। এগুলো সব নতুন harvestable resource নয়।
- **ছায়া:** সূর্য/চাঁদের দিক অনুযায়ী projection, নরম প্রান্ত, গাছ/পাথর/পায়ের গোড়ায় contact shadow।
- **পানি ও আলো:** পানির edge/ripple ও সীমিত tree reflection; fire/placed torch-এর আলো, flicker, দেয়াল/পাথর/উচ্চতার ভিত্তিতে light blocking।
- **Focus:** দূরের/প্রান্তের scenery-তে অল্প softening। এটি artistic DOF-style effect, বাস্তব camera lens simulation নয়। Focus pass hero, enemy ও UI blur করে না; আগের optional motion blur আলাদা।

## আপনার মাঝারি ফোনের জন্য

ডিফল্ট **ভারসাম্য / Balanced** রাখুন।

**Pause → সেটিংস → দেখার সুবিধা → পরবর্তী → পরবর্তী**  
তৃতীয় পাতায় **গ্রাফিক্সের মান** বোতামে চাপলে মান বদলাবে।

| মান | কখন ব্যবহার করবেন |
|---|---|
| **কম / Low** | ফোন গরম হলে বা frame drop হলে। কম decoration, সহজ shadow; reflection/focus/cloud effect বন্ধ। |
| **ভারসাম্য / Balanced** | মাঝারি ফোনের জন্য default; নরম shadow, সীমিত reflection, সংযত focus ও সর্বোচ্চ তিনটি local emitter। |
| **উন্নত / High** | বাড়তি effect পরীক্ষা করতে; সূক্ষ্ম reflection sampling ও সর্বোচ্চ ছয়টি local emitter। |

আরেকবার **পরবর্তী** দিলে চতুর্থ পাতায় **ফোকাস প্রভাব** চালু/বন্ধ করতে পারবেন। খুব crisp pixel look পছন্দ হলে বন্ধ করুন। **Battery saver** চালু থাকলে নির্বাচিত quality যাই হোক, কার্যকর quality Low হয়।

Graphics preferences world save থেকে আলাদা থাকে। মান বদলালে জগতের terrain, enemy difficulty বা inventory বদলায় না।

## UI ও gameplay আগের মতো

বাংলা default; English বেছে নেওয়া যায়। সহজ, লড়াই, বাঁ হাতে ও বড় বোতাম—চার layout এবং custom button placement আছে।

- বাঁয়ের joystick-এ চলুন; ডানের আঘাত ধরে attack করুন; এড়ান দিয়ে dodge। বাঁ-হাতি layout-এ দিক উল্টো।
- **আরও** থেকে weapon, guard/heavy, posture, mining, crafting, body ও bag খুলুন।
- জীবন bar-এ চাপলে খাবার/পানি/আঘাতের panel। Menu খোলা থাকলে simulation থামে।
- কাঠ → planks → কাজের টেবিল → pick → stone-এর crafting progression এবং tool durability আগের মতো।
- বের হওয়ার আগে **Pause → সেভ করে বের হন**।

## পুরোনো জগতের কী হবে?

Geography, collision ও save format বদলানো হয়নি। পুরোনো জগৎ নতুন materials/trees/lighting দিয়ে render হবে, কিন্তু সেখানে নতুন পাহাড়, নদী বা resource ঢুকিয়ে progress বদলানো হবে না। **খুব পুরোনো flat generator-এর জগতে এই update নতুন পাহাড় যোগ করবে না।** ০.৬ বা পরের generator-এ যে উচ্চতা আগে থেকেই আছে, নতুন projection সেটি বেশি স্পষ্ট দেখায়।

প্রতিনিধিত্বমূলক পুরোনো save-এর ৩,৮৭৫টি terrain/height record ও progress পরীক্ষা পাস করেছে। আপনার ব্যক্তিগত ফোনের প্রতিটি save পরীক্ষা করা হয়নি; স্থানীয় migration archive cloud backup নয়।

## সঙ্গে দেওয়া দৃশ্যগুলো

- **Before–After HTML:** একই seed, স্থান ও সময়ের ০.৭ ও ০.৮ native render; slider দিয়ে তুলনা করুন। Download করে browser-এ offline-ও খুলতে পারবেন।
- **৬০ সেকেন্ডের preview:** পাঁচ biome ও night camp। দৃশ্য দেখানোর জন্য teleport, clock setup ও invulnerability ব্যবহার করা হয়েছে। এটি ফোনের recording বা normal progression test নয়।

আলাদা Bengali ordinary-input progression test কোনো HP/items/teleport grant ছাড়াই পাস করেছে। মোট ১১/১১ automated test ও নতুন rendering/UI sanitizer tests পাস করেছে।

## সৎ সীমা ও আপনার পরীক্ষা

এই renderer 2D C++ raster-based: ray tracing, full 3D lighting, true lens DOF, full fluid simulation বা হাজারো হাতে আঁকা নতুন animation frame নয়। পুরোনো combat/character animations বজায় রাখা হয়েছে। কিছু পুরোনো notification ইংরেজি থাকে; world name এখনও ২৪ ASCII অক্ষর।

**বাস্তব ফোনে FPS, তাপ, battery, audio latency বা touch comfort আমি পরীক্ষা করিনি।** শুধু RAM দিয়ে ফোনের GPU/CPU ক্ষমতা বোঝা যায় না; তাই Balanced দিয়ে শুরু করুন।

প্রথমে পুরোনো জগতে ঢুকে হাঁটুন, গাছের আড়ালে যান, চলতে চলতে attack/dodge করুন এবং পাহাড়/পানির কাছে দেখুন। Frame drop হলে Low বেছে তুলনা করুন। প্রতিক্রিয়ায় ফোনের model, Android version, graphics quality এবং সমস্যা-দেখানো screenshot/recording দিলে পরের পরিবর্তনটি নির্দিষ্টভাবে করা যাবে।
