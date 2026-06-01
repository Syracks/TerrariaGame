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
- Systém nástrojů – krumpáč, sekera, meč, kladivo (Wood, Copper, Iron, Gold)
- Nábytek: dveře, židle, stůl, pracovní stůl, pec, kovadlina, truhla
- Score/Distance systém (UI v levém horním rohu)
- Loading screen při generování světa

## Ovládání

| Klávesa / myš | Akce |
|---|---|
| A / ← | pohyb doleva |
| D / → | pohyb doprava |
| Space | skok (držení = auto-jump) |
| Levé tlačítko myši | těžení / útok |
| Pravé tlačítko myši | položení bloku / otevření dveří |
| 1–9 | výběr slotu hotbaru |
| F5 | uložit hru |
| F9 | načíst hru |
| F11 | přepnutí fullscreen |
| Esc | pauza / inventář |
| M | přepnutí minimapy |

## Architektura

```
src/
├── app/          Game, GameState, kamera, ovládání hráče, GameRenderer, GameSession
├── core/         Konstanty, Math helper, TextureManager, SoundManager
├── crafting/     Recepty a crafting systém
├── entities/     Player, Mob, Entity (základ)
├── items/        Inventory, ItemStack, Tool, ItemDatabase
├── save/         SaveManager (textový formát)
├── systems/      CollisionSystem, PhysicsSystem, MiningSystem,
│                 RenderSystem, LiquidSystem, ParticleSystem,
│                 CombatSystem, InteractionSystem, DeathSystem
├── ui/           MainMenu, SettingsMenu, InventoryScreen, HUD, Minimap
└── world/        World, Chunk, Tile, TileRegistry, WorldGenerator
```

### Hlavní komponenty

| Komponenta | Odpovědnost |
|---|---|
| `Game` | Hlavní třída – herní smyčka, stavy, spawn, respawn, save/load orchestrace |
| `World` | Správa chunků, tile-based světa, getter/setter pro bloky, zdi, tekutiny, dveře |
| `Chunk` | 32×32 bloků, lazy alokace, dirty flag pro ukládání, stav dveří |
| `WorldGenerator` | Procedurální generování – noise-based terén, jeskyně, rudy, stromy, biomy |
| `Player` | Pohyb, gravitace, health, inventář, animace, swing/těžení, auto-jump |
| `Mob` | Nepřátelé (Slime, BlueSlime, Zombie) s AI |
| `CollisionSystem` | AABB kolize s tile mapou |
| `PhysicsSystem` | Aplikace gravitace a pohybu na entity |
| `MiningSystem` | Těžení bloků podle nástroje a tvrdosti |
| `LiquidSystem` | Fyzika vody a lávy (šíření, detekce ponoření, voda+láva→kámen) |
| `CombatSystem` | Boj – zásahy mečem, kontaktní poškození od mobů |
| `InteractionSystem` | Interakce – těžení, pokládání, otevírání dveří |
| `DeathSystem` | Smrt hráče, respawn, nalezení bezpečné pozice na povrchu |
| `RenderSystem` | Vykreslování bloků, zdí, dynamické osvětlení, podsvětí |
| `GameRenderer` | Vrstvení renderingu (pozadí, bloky, entity, UI), underground threshold |
| `MobSpawner` | Noční spawnování (timer, počet, distance od hráče) |
| `SettingsMenu` | Nastavení hry – volume, rozlišení, fullscreen, minimapa |
| `ParticleSystem` | Částicové efekty |
| `CameraController` | Kamera sledující hráče (zoom 2×) |
| `SaveManager` | Ukládání/načítání do textového formátu |
| `TextureManager` | Správa textur bloků a nástrojů |
| `SoundManager` | Procedurálně generované SFX (žádné externí soubory) |
| `Minimap` | Renderování minimapy do texture |
| `TileRegistry` | Definice bloků (název, solid, hardness, barva) |
| `ItemDatabase` | Definice itemů (craftování, spotřební, nástroje) |

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
