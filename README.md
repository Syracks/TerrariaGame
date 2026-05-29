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
- Inventář (20 slotů, 8 v hotbaru, výběr 1–8)
- Crafting ze surovin
- Denní/noční cyklus s dynamickým osvětlením
- Minimapa
- Ukládání a načítání hry (5 slotů)
- 3 velikosti světa (Small 1024×256, Medium 2048×400, Large 4096×600)

### Herní světy
- Procedurální generování terénu s biomy (Forest, Desert, Snow, Plains, Mushroom, Jungle)
- Jeskyně, hadovité tunely, podzemní místnosti
- Různé typy bloků: tráva, hlína, kámen, písek, sníh, led, jíl, štěrk, bláto, dřevo, listí
- Rudy: měď, železo, zlato
- Speciální biomové bloky: kaktus, houbová tráva, mramor, žula, džunglová tráva, hellstone
- Tekutiny: voda, láva (s fyzikou šíření)
- Stromy, kaktusy, houby

### Nepřátelé
- Slime, Blue Slime, Zombie
- Jednoduchá AI (idle hop, chase, patrol)

### Technické
- Chunk-based svět (32×32 bloků na chunk, lazy alokace)
- AABB kolize s tile mapou
- Částicový systém (těžení, smrt, crafting)
- SoundManager s procedurálně generovanými zvuky (žádné externí audio soubory)
- TextureManager s texturami pro bloky, nástroje a entity

## Ovládání

| Klávesa / myš | Akce |
|---|---|
| A / ← | pohyb doleva |
| D / → | pohyb doprava |
| Space | skok |
| Levé tlačítko myši | těžení / útok |
| Pravé tlačítko myši | položení bloku |
| 1–8 | výběr slotu hotbaru |
| F5 | uložit hru |
| Esc | pauza / inventář |
| M | přepnutí minimapy |

## Architektura

```
src/
├── app/          Game, GameState, kamera, ovládání hráče
├── core/         Konstanty, Input handler, Math, TextureManager, SoundManager
├── crafting/     Recepty a crafting systém
├── entities/     Player, Mob, Entity (základ)
├── items/        Inventory, ItemStack, Tool
├── save/         SaveManager (textový formát)
├── systems/      CollisionSystem, PhysicsSystem, MiningSystem,
│                 RenderSystem, LiquidSystem, ParticleSystem
├── ui/           MainMenu, SettingsMenu, InventoryScreen, HUD, Minimap
└── world/        World, Chunk, Tile, TileRegistry, WorldGenerator
```

### Hlavní komponenty

| Komponenta | Odpovědnost |
|---|---|
| `Game` | Hlavní třída – herní smyčka, stavy, spawn, respawn, save/load orchestrace |
| `World` | Správa chunků, tile-based světa, getter/setter pro bloky, zdi, tekutiny |
| `Chunk` | 32×32 bloků, lazy alokace, dirty flag pro ukládání |
| `WorldGenerator` | Procedurální generování – noise-based terén, jeskyně, rudy, stromy, biomy |
| `Player` | Pohyb, gravitace, health, inventář, animace, swing/těžení |
| `Mob` | Nepřátelé (Slime, BlueSlime, Zombie) s AI |
| `CollisionSystem` | AABB kolize s tile mapou |
| `PhysicsSystem` | Aplikace gravitace, pohybu na entity |
| `MiningSystem` | Těžení bloků podle nástroje, pokládání |
| `LiquidSystem` | Fyzika vody a lávy (šíření, detekce ponoření) |
| `RenderSystem` | Vykreslování bloků, dynamické osvětlení |
| `ParticleSystem` | Částicové efekty |
| `CameraController` | Kamera sledující hráče s vyhlazováním |
| `SaveManager` | Ukládání/načítání do textového formátu |
| `TextureManager` | Správa textur bloků a nástrojů |
| `SoundManager` | Procedurálně generované SFX (žádné externí soubory) |
| `Minimap` | Renderování minimapy do texture |
| `TileRegistry` | Definice bloků (název, solid, hardness, barva) |

## Problémy

### World generation
- Jeskyně se občas prořezávají příliš blízko povrchu a vytvářejí visící travnaté plošiny
- `repairSurfaceLayer` problém zmírňuje, ale ne vždy je dostatečný
- Spawn detection při poškozeném terénu nad spawnem – používá se spiral search, ale občas selže
- Floating islands dočasně zakázané (nestabilní generování)
- Worm caves (hadovité tunely) někdy vytvoří nečekané průniky na povrch

### Optimalizace
- Light overlay počítá osvětlení v buňkách 32×32 px, což vytváří viditelné mřížkové přechody
- Minimapa se rebuilduje každých 30 frameů při změně – při velkých světech může zdržet
- Save formát je textový – ukládá všechny dirty chunky, při Large světě pomalejší
- Chunková alokace – všechny chunky světa se generují najednou při startu, žádné streamování
- Fyzika tekutin šíří vodu/lávu po celém světě každý frame – může být pomalé při rozsáhlých zaplaveních

### Bug fixes
- ~~Segfault při zavírání hry ve světě~~ (opraveno – `Minimap` texture se uvolňovala až po `CloseWindow()`)

## Plánované změny

- Texturované bloky (momentálně barevné obdélníky pro chybějící textury)
- Zvukové efekty 
- Crafting s více recepty a kategoriemi
- Více typů nepřátel 
- Floating islands
- Dynamické světelné zdroje (mobilní pochodně)
- Tooltipy a vylepšené UI

