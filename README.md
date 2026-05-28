# Terraria

## Popis projektu

2D sandbox hra inspirovaná principy Terraria, napsaná v C++20 s Raylib. Hra pracuje s blokovým světem, který je procedurálně generovaný a rozdělený na chunky. Hráč se může pohybovat, skákat, těžit a pokládat bloky.

## Použité technologie

- C++20
- Raylib 5.5 (grafika, vstup, okno)
- CMake 3.20+ (build system)

## Funkce hry

- Procedurálně generovaný 2D blokový svět (tráva, hlína, kámen, rudy, jeskyně)
- Pohyb hráče (doleva, doprava, skok) s gravitací a kolizemi
- Kamera sledující hráče
- Těžení bloků levým tlačítkem myši
- Pokládání bloků pravým tlačítkem myši
- Hotbar s 8 sloty, výběr klávesami 1–8
- Ukládání a načítání světa
- Menu pro novou hru / načtení

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

## Build a spuštění

```bash
cmake -S . -B build
cmake --build build
./build/Terraria
```

Raylib se stáhne automaticky přes FetchContent.

## Architektura projektu

- `Game` — hlavní třída, řídí životní cyklus aplikace a stavy
- `World` — správa chunků a tile-based světa
- `Chunk` — 32×32 bloků, lazy alokace, dirty flag
- `WorldGenerator` — procedurální generování terénu
- `Player` — hráč s pohybem, gravitací, inventářem
- `CollisionSystem` — kolize hráče s tile mapou v ose X a Y
- `MiningSystem` — těžení a pokládání bloků
- `RenderSystem` — vykreslování pouze viditelných bloků
- `Inventory` — hotbar s 8 sloty
- `SaveManager` — ukládání/načítání světa do textového formátu

## Procedurální generování světa

1. Základní výška terénu je určena jednoduchým noise algoritmem
2. Povrch je pokryt trávou, pod ním je hlína, hlouběji kámen
3. Rudy jsou náhodně rozmístěny v kamenné vrstvě
4. Jeskyně jsou vytvořeny jako kruhové dutiny v hlubších vrstvách

## Chunk systém a optimalizace

Svět je rozdělen na chunky o velikosti 32×32 bloků. Vykreslují se pouze chunky, které jsou viditelné kamerou. Každý chunk má dirty flag, který se nastaví při změně bloku a využívá se při ukládání.

## Ukládání a načítání

Save formát je textový. Ukládá se pozice hráče, inventář a všechny neprázdné změněné bloky. Pokud save soubor neexistuje, vytvoří se nový svět.

## Co bych dále vylepšil

- Textury místo barevných obdélníků
- Nepřátelé s jednoduchou AI
- Crafting systém
- Zvuky a částicové efekty
- Více biomů
- Osvětlení
