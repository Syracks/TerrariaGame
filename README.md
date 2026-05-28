# Terraria

2D sandbox hra inspirovaná principy Terraria, napsaná v C++20 s Raylib.

## Build a spuštění

```bash
cmake -S . -B build
cmake --build build
./build/Terraria
```

Raylib se stáhne automaticky přes FetchContent.

## Ovládání

| Klávesa / myš | Akce |
|---|---|
| A / ← | pohyb doleva |
| D / → | pohyb doprava |
| Space | skok |
| Levé tlačítko myši | těžení bloku |
| Pravé tlačítko myši | položení bloku |
| 1–8 | výběr slotu hotbaru |
| F5 | uložit hru |
| F9 | načíst hru |
| Esc | pauza / menu |
| I | inventář |
| M | minimapa |

## Architektura

- `Game` — hlavní třída, řídí stavy, herní smyčku, spawn a respawn
- `World` — správa chunků a tile-based světa
- `Chunk` — 32×32 bloků, lazy alokace, dirty flag pro ukládání
- `WorldGenerator` — procedurální generování terénu, jeskyní, rud, stromů
- `Player` — pohyb, gravitace, health, inventář, animace
- `Mob` — nepřátelé (slime, zombie) s jednoduchou AI
- `CollisionSystem` — AABB kolize s tile mapou
- `MiningSystem` — těžení a pokládání bloků
- `RenderSystem` — rendering bloků + dynamické osvětlení
- `ParticleSystem` — částicové efekty
- `CameraController` — kamera sledující hráče
- `SaveManager` — ukládání/načítání do textového formátu

## Svět a generování

Svět je generován v tomto pořadí:

1. buildWorldMaps — výšková mapa a biomy
2. protectSpawnArea — zarovnání spawn zóny
3. generateTerrain — bloky podle výškové mapy a biomu
4. generateCaves — noise-based jeskyně (min. 26 bloků pod povrchem)
5. generateWormCaves — hadovité tunele (hluboko, daleko od spawnu)
6. repairSurfaceLayer — oprava povrchu po jeskyních
7. generateUndergroundCabins — podzemní místnůstky
8. generateOreVeins — rudy v kamenné vrstvě
9. generateUndergroundPockets — podzemní kapsy
10. repairSurfaceLayer — finální oprava před stromy
11. generateTrees — stromy, kaktusy, houby

Floating islands jsou dočasně zakázané.

## Osvětlení

Svět má den/noční cyklus a dynamické osvětlení:
- Povrch je osvětlený podle denní doby (v noci tlumené)
- Pod zemí klesá světlo s hloubkou
- Hráč a pochodně vytvářejí kruhové světelné zdroje
- Světlo je počítáno v buňkách o polovině tile size

## Aktuální problémy

- **Stabilita generování**: jeskyně se někdy prořezávají příliš blízko povrchu, vznikají visící travnaté plošiny. Opraveno pomocí SURFACE_SAFE_DEPTH a repairSurfaceLayer, ale pořád se testuje.
- **Osvětlení**: light overlay používá buňky 32×32 px, což občas vytváří viditelné mřížkové přechody.
- **Spawn detection**: hledání bezpečné spawn pozice je složité, pokud je terén nad spawnem poškozený. Používá se spiral search s kontrolou pevné země a volného prostoru.

## Plánované změny

- Textury místo barevných obdélníků
- Crafting systém
- Více typů nepřátel (letící, lezoucí)
- Floating islands (až bude terén stabilní)
- Dynamické světelné zdroje (mobilní pochodně)
- Vylepšený save formát (binární nebo komprimovaný)
- Tooltipy a vylepšené UI
- Slot pro batoh / rozšířený inventář
