# Vista ০.৮ — গবেষণা ও graphics design

৫ অক্টোবর ২০২৬। ব্যবহারকারীর সিদ্ধান্ত: শুধু রাত নয়, **পুরো art-এর মান** উন্নয়ন; **মাঝারি ক্ষমতার ফোন** লক্ষ্য। ০.৭-এর বাংলা UI, চার control layout ও পুরোনো world progress বজায় রাখতে হবে। Natural/grounded palette-ও রাখা হয়েছে।

## কী পড়েছি ও কী শিখেছি

### ১. Eastward — detail-এর সঙ্গে আলো ও palette

[80.lv: Eastward: Charming Chinese Pixel Art Adventure](https://80.lv/articles/eastward-charming-chinese-pixel-art-adventure), fetched article date 21 September 2016। লেখাটি 2D art-এর সঙ্গে complex lighting, palette, ছোট environmental details, moving elements এবং SSAO-like/LUT techniques আলোচনা করে। এটি studio pipeline-এর পূর্ণ technical specification নয়; article-এর unrelated cultural generalizations আমাদের design rationale নয়।

**শিক্ষা:** শুধু বেশি pixels নয়—গাছ, মাটি, ছোট বস্তু ও আলোকে একই visual language-এ রাখতে হবে। গেমে নিজস্ব clustered foliage, coherent material clusters ও biome-aware scatter pattern করেছি। Eastward-এর proprietary lighting system বা asset ব্যবহার করিনি।

### ২. Sea of Stars — সীমিত pixel language, আধুনিক lighting

[LRM Online: Thierry Boulanger interview](https://lrmonline.com/news/how-the-retro-inspired-sea-of-star-captures-the-feeling-of-90s-rpgs-exclusive-interview/), 15 April 2020। পড়া প্রথম অংশে director retro audio/visual feeling, dynamic lighting ও simplicity—‘distill instead of dilute’—এর কথা বলেছেন। পাঁচ-chunk interview-এর প্রথম chunk পড়া হয়েছে; পূর্ণ interview/video বিশ্লেষণের দাবি নয়।

**শিক্ষা:** effect দিয়ে মূল sprite হারানো নয়। নতুন shadow/reflection/focus pass-এর পর hero, enemies ও vegetation আঁকা হয়; scenery focus চরিত্র/UI blur করে না। পুরোনো optional motion blur আলাদা।

### ৩. Octopath Traveler II — depth, organic maps, day/night consistency

[Unreal Engine / Acquire developer interview](https://www.unrealengine.com/en-US/developer-interviews/octopath-traveler-ii-builds-a-bigger-bolder-world-in-its-stunning-hd-2d-style), 22 August 2023। প্রথম chunk-এ organic pixel-art maps, dynamic day/night lighting, 2D character বনাম প্রায় সম্পূর্ণ 3D background-এর সামঞ্জস্য ও camera challenge পড়েছি। সেই interview-এর ship/harbour screenshot-ও দেখেছি: lit upper planes, dark recesses, material direction এবং foreground/background separation লক্ষ করেছি।

**শিক্ষা:** depth-এর জন্য পুরো screen blur নয়; উচ্চতার projection, top/side plane-এর আলাদা tone, object-foot contact ও আলোর দিক গুরুত্বপূর্ণ। আমাদের renderer এখনও CPU-based 2D raster; Octopath-এর 3D/Unreal pipeline বা camera system বাস্তবায়ন করা হয়নি।

### উৎসের সীমা

Image search-এর কয়েকটি ফল requested game-এর বদলে stock/generated waterfall ও অন্য indie game দেখিয়েছে। সেগুলোকে Sea of Stars/Eastward-এর screenshot বলে ব্যবহার বা বিশ্লেষণ করা হয়নি। Unity Pixel Perfect documentation fetch ব্যর্থ হয়েছে; তাকে পড়া উৎস হিসেবে দাবি করছি না। কোনো reference screenshot, third-party game sprite বা commercial game code APK-তে ঢোকানো হয়নি। ‘সবচেয়ে সুন্দর’ subjective—এই গবেষণা objective ranking বা studio-scale visual parity প্রতিষ্ঠা করে না।

## বাস্তব implementation

মূল file `native/vista.hpp`; tree/foliage update `native/look.hpp`; projection `native/art.hpp` ও render callsites; pass ordering `native/presentation.hpp`।

| সমস্যা | পরিবর্তন | সীমা |
|---|---|---|
| সমান সমান মাটি | বিদ্যমান height-এর ৬-pixel-per-level projection; cliff face/strata, edge highlight ও ramp step | নতুন পাহাড়/নতুন geography তৈরি করা হয়নি |
| পুনরাবৃত্তি ও noisy floor | 4×3 clustered material patches, continuous macro variation, biome-specific marks, irregular path edges | procedural pixel materials; হাতে আঁকা প্রতিটি tile নয় |
| গাছের একঘেয়ে volume | পাঁচ-tone clustered foliage, নতুন conifer tier, asymmetric crown; নয় existing species × আট cosmetic variants | নতুন species/gameplay resource নয়; biome palette-সহ 360 cache combinations পরীক্ষা |
| পরিবেশ খালি | fern, grass/reed, flowering tuft, clustered bush, log/branch, pebble, mushroom, litter-এর 12 scatter patterns | decorative; নতুন harvest/collision object নয় |
| শক্ত/সমতল ছায়া | tree-silhouette projection, actor/object casts, half-resolution separable softening, tight contact shadow | 2D approximation; ray tracing/3D shadow map/SSAO নয় |
| পানির uniform look | shallow edge/foam-like highlights, moving ripples, water-mask-limited tree reflection | সীমিত screen-space reflection; full-world reflection/fluids নয় |
| local light দেয়াল ভেদ করে | camp/fire/placed torch light, flicker, existing solid/height grid blocker test, local cast approximation | Low-তে simplified glow; full per-pixel 3D occlusion নয় |
| দূরত্বের অনুভূতি কম | background/edge terrain-এর অল্প focus softening, broad cloud shade ও airborne motes | artistic DOF-style focus, optical lens/bokeh simulation নয় |

## Pass order ও interaction

Terrain + water stencil → limited reflection → projected/soft/contact shadows → scenery-only focus → telegraphs ও Y-sorted props/decor/actors → drops → day/local lighting/clouds → existing optional motion blur → unchanged UI। Hero, animals, enemies, props, structures, drops ও relevant telegraphs-এ consistent elevation projection ব্যবহার করা হয়েছে। Physics, collision grid, generator, enemy difficulty, save serializer ও 24-minute active day অপরিবর্তিত।

Decor deterministic coordinate hashes ব্যবহার করে; gameplay RNG consume করে না। গাছ কাটলে standing tree shadow/reflection বন্ধ হয়; falling tree-এর projection বদলায়। Local lighting grid blockers respects walls/rocks/large elevation changes, কিন্তু real 3D penumbra বা physically based normals-এর দাবি নেই।

## মাঝারি ফোনের budget

**Balanced default**: soft tree shadows, reflection, restrained focus, maximum three visible local emitters, moderate decorative density। **High**: finer reflection sampling, up to six local emitters, stronger optional focus/more motes। **Low**: fewer scattered decorations, simpler casts; reflections/focus/cloud layer off। Battery saver effective quality Low করে।

`visual8.cfg` আলাদা checked/atomic preference file; world save ও `ui7.cfg` format বদলায়নি। Existing tree sprites cached; deterministic macro noise per tile reused; shadow buffers 320×180।

Host warm-cache renderer cost, 640×360, 120 frames/scene: Balanced scene means **4.97–7.45 ms**, scene p95 **5.29–9.10 ms**। এটি desktop/server CPU rendering-only timing; gameplay simulation, Android presentation, audio, thermal ও touch বাদ। এই সংখ্যাকে ফোনের FPS বলা যাবে না। High-এর কয়েকটি scene sample Balanced-এর চেয়ে দ্রুত হয়েছে—host variance/scene workload; সব ক্ষেত্রের monotonic performance guarantee নয়।

## মূল্যায়ন

০.৭-এর unmodified native source checkpoint দিয়ে একই capture utility compile করে একই seed/coordinate/time-এর before images বানিয়েছি। ০.৮-এর corresponding renderer images দিয়ে standalone before/after page হয়েছে। এগুলো staged renders, user phone screenshot নয়। New graphics invariants, old UI, ordinary-input progression এবং authentic prior-save fixture পরীক্ষা হয়েছে; বিস্তারিত `TEST-REPORT.md`।

এখনও দরকার বাস্তব ফোনে visual preference, character readability under foliage, frame pacing, thermal/battery ও gesture comfort যাচাই। Character/enemy animation system রাখা হয়েছে; এই release-এ হাজারো hand-drawn animation frame বা পুরো studio-quality character-art library তৈরি হয়নি।
