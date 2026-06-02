# Terraria

2D sandbox hra inspirovaná principy Terraria, napsaná v C++20 s Raylib.

## Build a spuštění

```bash
cmake -S . -B build
cmake --build build
./build/Terraria
```

Raylib se stáhne automaticky přes FetchContent.

## Co je v projektu

### Hratelné
- Pohyb hráče (A/D/šipky, skok Space)
- Těžení a pokládání bloků
- Těžení zdí (kladivem)
- Inventář (45 slotů, 9 v hotbaru, výběr 1–9)
- Crafting ze surovin (v ruce, u pracovního stolu, pece, kovadliny)
- Denní/noční cyklus s dynamickým osvětlením (12h/12h, 420s celkem)
- Tekutiny: voda a láva s fyzikou šíření, vzájemná interakce (voda+láva→kámen)
- Minimapa (přepnutí M)
- Nastavení hry – volume, rozlišení, fullscreen, zobrazení minimapy (perzistentní, aplikuje se při startu)
- Ukládání a načítání hry (5 slotů)
- 3 velikosti světa (Small 1024×256, Medium 2048×400, Large 4096×600)
- Virtuální rozlišení 1280×720 s letterbox scalingem, F11 fullscreen toggle
- **Obtížnost**: Normal / Hardcore (hardcore = permasmrt)
- **Luky a šípy**: Copper Bow, Iron Bow, Gold Bow – střelba levým tlačítkem, spotřebovávají šípy
- **Boss fight**: Forest Guardian – summonování pomocí Ancient Seed, 3 fáze, projektily (Leaf, Fireball)
- **Biome-specific background music** (forest, desert, ice, jungle, ocean, underground, night, boss)
- **Časová zobrazení v HUD** (HH:MM, Day/Night)
- **On-screen notifikace** (zprávy s fade-out efektem)
- **Boss HP bar** ve vrchní části obrazovky

### Herní světy
- Procedurální generování terénu s biomy (Forest, Desert, Snow, Plains, Jungle, Ocean, Beach)
- Jeskyně, hadovité tunely, podzemní místnosti (cabiny)
- Vodní a lávové kaluže (bowl-shaped pools)
- Různé typy bloků: tráva, hlína, kámen, písek, sníh, led, jíl, štěrk, bláto, dřevo, prkna, listí
- Rudy: měď, železo, zlato
- Speciální biomové bloky: kaktus, mramor, žula, džunglová tráva, hellstone
- Stěny: hliněná, kamenná, prkenná
- Tekutiny: voda, láva (s fyzikou šíření)
- Stromy, kaktusy

### Nepřátelé
- Slime, Blue Slime, Zombie
- **Forest Guardian** – boss s 3000 HP, 3 fázemi:
  - Phase 1 (>66% HP): střílí Leaf projektily
  - Phase 2 (>33% HP): střílí Leaf + Fireball, vyšší rychlost
  - Phase 3 (<33% HP): enraged mód – 3× rychlost
- Jednoduchá AI (idle hop, chase, patrol)
- Noční spawnování

### Technické
- Chunk-based svět (32×32 bloků na chunk, lazy alokace)
- Virtuální rozlišení 1280×720 → škálování na okno s letterboxingem
- AABB kolize s tile mapou
- Částicový systém (těžení, smrt, crafting)
- SoundManager s procedurálně generovanými zvuky (žádné externí audio soubory)
- TextureManager s texturami pro bloky, nástroje a entity (formát `*_0.png`)
- Auto-jump (držení Space pro automatické přeskakování překážek)
- Respawn s návratem na povrch
- Systém nástrojů – krumpáč, sekera, meč, kladivo, **luk** (Wood, Copper, Iron, Gold)
- Nábytek: dveře, židle, stůl, pracovní stůl, pec, kovadlina, truhla
- Score/Distance systém (UI v levém horním rohu)
- Loading screen při generování světa
- **MusicManager** – singleton přehrávající biome-specific hudbu z MP3 souborů (externí assety)
- **Projectile system** – Arrow (hráč), Leaf a Fireball (boss) s gravitací/noGravity flagy
- **TempHitbox** – dočasné hitboxy pro frame-precise damage interakce
- **Boss summoning** – Ancient Seed item summonující Forest Guardiana
- **Zobrazení názvu vybraného itemu v hotbaru** a počtu šípů při výběru luku

## Ovládání

| Klávesa / myš | Akce |
|---|---|
| A / ← | pohyb doleva |
| D / → | pohyb doprava |
| Space | skok (držení = auto-jump) |
| Levé tlačítko myši | těžení / útok / **střelba z luku** |
| Pravé tlačítko myši | položení bloku / otevření dveří / **interakce s Ancient Seed** |
| 1–9 | výběr slotu hotbaru |
| F5 | uložit hru |
| F9 | načíst hru |
| F11 | přepnutí fullscreen |
| Esc | pauza / inventář |
| M | přepnutí minimapy |

## Architektura

```
src/
├── app/          Game, GameState, kamera, ovládání hráče, GameRenderer, GameSession, MobSpawner
├── core/         Konstanty, Math helper, TextureManager, SoundManager, MusicManager, Input
├── crafting/     Recepty a crafting systém
├── entities/     Player, Mob, Entity (základ), Arrow (Projectile), TempHitbox
├── items/        Inventory, ItemStack, Tool, ItemDatabase
├── save/         SaveManager (textový formát)
├── systems/      CollisionSystem, PhysicsSystem, MiningSystem,
│                 RenderSystem, LiquidSystem, ParticleSystem,
│                 CombatSystem, InteractionSystem, DeathSystem
├── ui/           MainMenu, SettingsMenu, InventoryScreen, ChestScreen, HUD, Minimap
└── world/        World, Chunk, Tile, TileRegistry, WorldGenerator
```

### Hlavní komponenty

| Komponenta | Odpovědnost |
|---|---|
| `Game` | Hlavní třída – herní smyčka, stavy, spawn, respawn, save/load orchestrace |
| `World` | Správa chunků, tile-based světa, getter/setter pro bloky, zdi, tekutiny, dveře |
| `Chunk` | 32×32 bloků, lazy alokace, dirty flag pro ukládání, stav dveří |
| `WorldGenerator` | Procedurální generování – noise-based terén, jeskyně, rudy, stromy, biomy |
| `Player` | Pohyb, gravitace, health, inventář, animace, swing/těžení, auto-jump, **bow firing** |
| `Mob` | Nepřátelé (Slime, BlueSlime, Zombie, **ForestGuardian**) s AI |
| `Projectile` | Střely – Arrow (hráč), Leaf/Fireball (boss) |
| `TempHitbox` | Dočasný hitbox pro frame-precise damage (boss melee, swing) |
| `CollisionSystem` | AABB kolize s tile mapou |
| `PhysicsSystem` | Aplikace gravitace a pohybu na entity |
| `MiningSystem` | Těžení bloků podle nástroje a tvrdosti |
| `LiquidSystem` | Fyzika vody a lávy (šíření, detekce ponoření, voda+láva→kámen) |
| `CombatSystem` | Boj – zásahy mečem, kontaktní poškození od mobů, **boss projectile kolize** |
| `InteractionSystem` | Interakce – těžení, pokládání, otevírání dveří, **střelba z luku** |
| `DeathSystem` | Smrt hráče, respawn, hardcore check |
| `RenderSystem` | Vykreslování bloků, zdí, dynamické osvětlení, podsvětí |
| `GameRenderer` | Vrstvení renderingu (pozadí, bloky, entity, UI, **projectiles**), underground threshold |
| `MobSpawner` | Noční spawnování, **boss spawn**, **hardcore ovlivnění spawn rate** |
| `SettingsMenu` | Nastavení hry – volume, rozlišení, fullscreen, minimapa |
| `ParticleSystem` | Částicové efekty |
| `CameraController` | Kamera sledující hráče (zoom 2×) |
| `SaveManager` | Ukládání/načítání do textového formátu |
| `TextureManager` | Správa textur bloků a nástrojů |
| `SoundManager` | Procedurálně generované SFX (žádné externí soubory) + **PlayerHurt, BossSummon** |
| `MusicManager` | **Singleton pro biome-specific hudbu z MP3** (boss, night, forest, desert, ice, jungle, ocean, underground) |
| `Minimap` | Renderování minimapy do texture |
| `TileRegistry` | Definice bloků (název, solid, hardness, barva) |
| `ItemDatabase` | Definice itemů (craftování, spotřební, nástroje, **zbraně, munice**) |
| `HUD` | **Boss HP bar, čas, on-screen zprávy, item name + arrow count** |

## Problémy

### World generation
- Jeskyně se občas prořezávají příliš blízko povrchu a vytvářejí visící travnaté plošiny
- `repairSurfaceLayer` problém zmírňuje, ale ne vždy je dostatečný
- Worm caves (hadovité tunely) někdy vytvoří nečekané průniky na povrch
- Floating islands dočasně zakázané (nestabilní generování)
- Stromy kontrolují překážky v koruně, ale občas chybí vzduchové mezery u kmene

### Optimalizace
- Light overlay počítá osvětlení v buňkách 32×32 px, což vytváří viditelné mřížkové přechody
- Minimapa se rebuilduje při změně – při velkých světech může zdržet
- Save formát je textový – ukládá všechny dirty chunky, při Large světě pomalejší
- Chunková alokace – všechny chunky světa se generují najednou při startu, žádné streamování
- Fyzika tekutin šíří vodu/lávu po celém světě každý frame – může být pomalé při rozsáhlých zaplaveních

### Boss
- Boss AI je very scuffed – forestGuardianAI potřebuje vylepšit, nemá animace
- Fázové přechody jsou ad-hoc, chybí plynulé animace

