# নতুন চ্যাটের জন্য হ্যান্ডঅফ — Death World

**এই নথি আপডেট: ২০২৬-১০-০৬। বর্তমান source: 0.9.2 / Android versionCode 11।**

## ১. GitHub থেকে নতুন Arena চ্যাটে চালিয়ে যাওয়া

হ্যাঁ, নতুন চ্যাটে repository নির্বাচন করে কাজ চালানো সম্ভব **যদি ওই চ্যাটে repository-র ফাইল পড়া/checkout করার access পাওয়া যায়**। পরিবর্তন GitHub-এ commit/push করার জন্য write permission-ও লাগবে। এই ZIP তৈরি করার সেশনে সরাসরি GitHub write tool পাওয়া যায়নি; repository তৈরি বা push করা হয়নি।

**আগের চ্যাটের ইতিহাস, ব্যক্তিগত signing key, workspace-এর backup এবং phone screenshots স্বয়ংক্রিয়ভাবে repository-র সঙ্গে যায় না।** তাই এই হ্যান্ডঅফ ও `AGENTS.md` দেওয়া হয়েছে। নতুন চ্যাটকে এগুলো আগে পড়তে বলুন।

### GitHub-এ ফাইল দেওয়া

1. ZIP ডাউনলোড করে **Extract/Unzip** করুন। ভেতরে `death-world` নামে একটি ফোল্ডার পাবেন।
2. GitHub-এ একটি repository তৈরি করুন; আপাতত **Private** রাখার পরামর্শ।
3. `death-world` ফোল্ডারের **ভেতরের ফাইল/ফোল্ডারগুলো** repository-র root-এ দিন। root-এ `README.md`, `AGENTS.md`, `CMakeLists.txt`, `native`, `android`, `assets`, `tools`, `tests` দেখা উচিত।
4. **শুধু ZIP ফাইলটি repository-তে রেখে দেবেন না।** সেটি archive হিসেবে থাকবে, সরাসরি browse/build করার source tree হবে না।
5. Web uploader-এ ফাইলসংখ্যার সীমা হলে কয়েকটি folder করে upload করুন। কম্পিউটারে GitHub Desktop দিয়ে extracted folder commit/push করা সুবিধাজনক। ফোনে folder upload অসুবিধা হলে কম্পিউটার অথবা অনুমোদিত cloud development terminal ব্যবহার করুন; ডিরেক্টরি কাঠামো সমতল করে ফেলবেন না।
6. Hidden `.gitignore` ও `.gitattributes` বাদ দেবেন না। **৩২টি WAV**, fonts, `cover.bin`, generated font header এবং synthetic save fixtures-ও source-এর অংশ।
7. Commit শেষ হলে Arena-তে নতুন চ্যাট খুলে repository/সঠিক branch নির্বাচন করুন। Agent-কে আগে ফাইল access হয়েছে কি না যাচাই করতে বলুন।

নতুন চ্যাটে পাঠানোর উদাহরণ:

> এই repository-র Death World প্রকল্পে কাজ চালিয়ে যাও। আগে AGENTS.md, HANDOFF_BN.md, README.md, SIGNING.md এবং docs/medium-thermal-fix/STATUS.md পড়ো। বাংলায় কথা বলো। বর্তমান source 0.9.2/code11। Galaxy F23-এ Medium thermal fallback-এর সংশোধন ফোনে এখনও নিশ্চিত হয়নি—আগে আমার ফলাফল নাও ও পরীক্ষা করো। পুরোনো world/Low look/signing identity নষ্ট করবে না। তারপর অনুমোদিত sound ও Earth-like world পরিকল্পনা এগোবে। GitHub-এর ফাইল পড়া ও write-access সত্যিই আছে কি না আগে যাচাই করো।

## ২. বর্তমান সমস্যা ও সর্বশেষ কাজ

ফোন: **Samsung Galaxy F23, 6 GB RAM, Android 14**।

- Owner Medium বাছলেও পুরোনো player/HUD দেখতে পেয়েছেন।
- 0.9.1 screenshot-এ `Active graphics: Low`, `Device is hot`, `Auto fallback: Off`, `Suggested: Medium` ছিল।
- পরে owner raw `Thermal status` হিসেবে **2** বলেছেন। Screenshot ও copied number একই মুহূর্তের কি না জানা নেই।
- আগের policy-তে severe thermal signal পেলেই Low হতো। এছাড়া raw platform value ও policy পরের frame-এ আলাদাভাবে প্রয়োগ হওয়ায় মাঝে raw 2/পুরোনো hot decision একসঙ্গে দেখা সম্ভব ছিল। ওই অল্প সময়ের অমিল একাই persistent phone সমস্যার কারণ—এমন দাবি করা হয়নি।

### 0.9.2-তে যা হয়েছে

- Raw platform status ও applied decision একই owner-thread call-এ commit হয়: `graphicsPlatformProfile`। Android JNI ও regression tests একই helper ব্যবহার করে।
- Java প্রতি সেকেন্ডে profile নেয়; resume/focus ফিরে এলে ও Feedback খোলার আগে fresh sample নেয়।
- Thermal 0–1: স্বাভাবিক Medium, নির্বাচিত 30/60 cap এবং অন্যান্য সুরক্ষা সাপেক্ষে।
- **Thermal 2: Medium, 30 FPS cap।**
- **Thermal 3: reduced-effects Medium, 20 FPS cooling cap।** বিস্তারিত actor, নতুন HUD ও genuine normal lighting থাকে; এক local light, bloom/height occlusion/দামী post-effects ও scenery effects কমে।
- **Thermal 4–6: mandatory Low, 20 cap।** Auto fallback Off হলেও critical/GPU/memory সুরক্ষা থাকে। OS protection বন্ধ করা হয়নি।
- Settings-এ raw thermal number/name, কার্যকর mode ও cap দেখা যায়। Feedback-এ applied thermal enum/profile revision আছে। Cap প্রকৃত মাপা FPS নয়।

**Owner এখনও 0.9.2 ফোনে ঠিক হয়েছে বলে নিশ্চিত করেননি।** তাই পরের কাজের প্রথম ধাপ তাঁর নতুন ফলাফল। পুরো রিপোর্ট পেলে selected quality নয়, effective quality/reason/GPU readiness/thermal number দেখুন।

সর্বশেষ APK: `Death-World-0.9.2-Thermal-Fix.apk`, 16,964,842 bytes। এটি এই source ZIP-এর মধ্যে নেই।

SHA256: `f21feccb7a27902212792b5adaac5c3db318b69763675517fd2c3b193d8a1f85`।

## ৩. বাস্তবে যে পরীক্ষা হয়েছে

- 0.9.2 full native CTest: **১৪/১৪ pass**।
- Policy/config: **১২,৪৫২ assertions pass**।
- Actual host GLES + capture run: **১২,৯৩৭ assertions pass**। Thermal 2/3-তে সত্যিকারের Medium shader, cooling অবস্থাতেও normal response, critical 4–6 Low এবং recovery পরীক্ষা হয়েছে।
- ৭২টি controlled Low frame ও ১১,০২৪টি legacy geography record পূর্বে compiled মূল 0.8 reference hash-এর সঙ্গে হুবহু মিলে গেছে।
- APK signature/৩ ABI/১৮ JNI/১৬ KB alignment/৩৮ asset-license file/৩২ WAV যাচাই হয়েছে।
- Host GPU ছিল Mesa llvmpipe। **Galaxy F23-এ সরাসরি FPS/তাপ/ল্যাটেন্সি পরীক্ষা নয়।** Sandbox RAM/KVM সীমায় আগের emulator startup সফল হয়নি।

এই source export-এ game C++/Java logic পাল্টানো হয়নি। Documentation, repository hygiene, archive integrity tools ও missing-signing-key preflight যুক্ত হয়েছে। Historical reports-কে নতুন phone test হিসেবে চালিয়ে দেবেন না।

## ৪. স্থাপত্য ও গুরুত্বপূর্ণ ফাইল

- `native/engine.cpp`: frame/simulation integration, Android JNI, diagnostics।
- `graphics_quality.hpp`: requested/effective Low/Medium, caps, fallback, explicit thermal mapping, checked settings।
- `graphics_runtime.hpp`: platform profile প্রয়োগ, G-buffer/UI capture boundary।
- `gpu_medium.hpp`: GLES 3 lighting; 640×360 framebuffer-এ shade, পরে nearest display scaling।
- `medium_visuals.hpp`: procedural actor albedo/normals, bounded caches, directional animation/canopy treatment।
- `generation5.hpp`, `frontier.hpp`: versioned organic world layout; পুরোনো generator branches অপরিবর্তিত।
- `ui7.hpp`: UI/controls/settings/HUD; generated shaped Bengali masks `ui7_fonts.hpp`।
- `MainActivity.java`: **একটি UI-thread owner**-এ native game, TextureView/EGL, Canvas fallback, audio/input/lifecycle। আলাদা concurrent simulation writer নেই।

Medium হলো **CPU G-buffer + আসল GPU lighting hybrid**। এটিকে সম্পূর্ণ GPU sprite-atlas batching বা সব frame হাতে আঁকা finished studio art বলা যাবে না।

## ৫. Owner-এর স্থায়ী সিদ্ধান্ত

- মোবাইল top-down 2D/pixel-art; সম্ভব হলে বেশিরভাগ C++। চলমান game-এর নাম **Death World**। In-game `New Horizon Z` app/package বদলানোর অনুমতি নয়।
- Studio branding `obsidian games`, owner `obsidian syndicate`। অন্য প্রতিষ্ঠানের সঙ্গে affiliation/trademark clearance দাবি নয়।
- বাংলায় আলোচনা; Bengali default + English UI; চার control layout/editing এবং সহজ move/attack/dodge/context ব্যবস্থা রাখুন।
- **Low = আগের 0.8 appearance/preferences**, জোর করে পুরোনো lowest preset নয়। এই পর্যায়ে Low + Medium; High স্থগিত।
- Quality যেন combat, AI difficulty, simulation, RNG, world seed/geometry/progress বদলায় না। অনুমোদিত AI/combat উন্নতি দুই tier-এ সমান।
- পুরোনো named/seeded world-এর geography, position, level, inventory ও progress অক্ষত। নতুন geography শুধু নতুন world-এ। Generator 5 নতুন world-এর জন্য; 1–4 compatibility রাখুন।
- ২৪ মিনিটের active day, পাঁচ day phase, explored-only map; unknown অংশ কালো।
- Walking/attacking/hurt/weapon selection অক্ষত; enemy-to-player targeting line ফিরিয়ে আনবেন না।
- Survival hunger/thirst/stamina, body injuries, scarce resources/rest, ১৬ wildlife species/food chain, purposeful former-human enemies, guardian territory/boss difficulty—বিদ্যমান ব্যবস্থাগুলো preserve করুন।
- Crafting/building/farming/materials/tools, cuttable trees, bobbing pickups, বিভিন্ন biome, rivers/streams/ponds/lakes; top-down + elevation, side-view নয়।
- প্রাকৃতিক restrained palette/composition; artificial plantation/grid নয়। Player readability, equipment, direction/action animation ও canopy visibility অগ্রাধিকার।
- Target performance থাকলেও পরীক্ষা ছাড়া 60 FPS বা minimum-30 guarantee নয়। Safety slowdown আলাদা বিষয়।
- সৎ audit, phased implementation এবং বাস্তব test report দরকার; অসম্পূর্ণ কাজ complete বলবেন না।

## ৬. পরের কাজ / অসম্পূর্ণ অংশ

**প্রথমে thermal/Medium fix ফোনে নিশ্চিত করুন।** তারপর owner চান:

1. নতুন ও উন্নত sound design।
2. পৃথিবীর মতো আরও ভালো world—শুরু থেকেই পাহাড়, ভূপ্রকৃতি ও আরও প্রাকৃতিক উপাদান। Existing saved geography না ভেঙে পরিকল্পনা করুন।

আগের বড় Medium scope-এর মধ্যে এখনও অসম্পূর্ণ: full bespoke wildlife/prop/inventory icons, fully GPU-atlas-batched/compressed world pipeline, advanced parallax/fog/cinematic polish, actual phone sustained FPS/thermal/lifecycle/audio validation। অনেক prop/wildlife/tree silhouette/animation এখনও shared বা legacy art।

## ৭. সাইনিং, ব্যাকআপ ও source ZIP-এর সীমা

**`SIGNING.md` পড়ুন।** নতুন চ্যাটে মূল private key না থাকলে একই package-এর update APK বানিয়ে owner-এর পুরোনো installation আপডেট করা যাবে না। Public certificate fingerprint private key নয়। Build script এখন key না থাকলে থামে; নতুন key তৈরি explicit disposable-test opt-in।

- মূল 0.5/0.8 ব্যাকআপ, সর্বশেষ APK এবং original signing key মূল workspace-এ রক্ষিত; এই ZIP-এ নেই।
- আগের uploads owner-এর নির্দেশে মুছে শুধু সর্বশেষ দুটি screenshot মূল workspace-এ রাখা হয়েছে। ব্যক্তিগত screenshots repository ZIP-এ নেই।
- ব্যর্থ/superseded 0.9.1 APK owner-এর অনুমতিতে সরানো হয়েছে। Source ZIP রাখার জায়গার জন্য পুরোনো 0.9 Preview APK সরানোরও অনুমতি দেওয়া হয়েছে।
- SDK/NDK/build cache ও পুরোনো source-backup ZIPs source package-এ নেই। দরকারি current source, test fixtures, runtime assets/audio ও licenses আছে।
- Package-এর `SOURCE-MANIFEST.json` ও `tools/check_source.py` initial import যাচাই করে। Archive-টি extract করে ফাইল commit করুন; ZIP-কে source tree ভেবে কাজ করবেন না।
