"""
Exportador de los nuevos circuitos variados:
- assets/levels/track_desert_dunes.txt
- assets/levels/track_cosmic_space.txt
- assets/levels/track_aquatic_reef.txt
"""
import sys, os
sys.path.insert(0, os.path.abspath("."))
from scratch.test_new_track_shapes import build_desert, build_cosmic, build_aquatic, validate_track

def export_track(filepath, comment, cols, rows, grid):
    with open(filepath, "w") as f:
        f.write(f"# {comment}\n")
        f.write(f"GRID {cols} {rows}\n")
        for (x, y), (tileType, rot) in sorted(grid.items(), key=lambda k: (k[0][1], k[0][0])):
            f.write(f"TILE {x} {y} {tileType} {rot:.1f} 0\n")
    print(f"[EXPORT] {filepath} ({len(grid)} losas)")

def main():
    # 1. Desierto (Wedge Asimétrico)
    d_grid, d_start = build_desert()
    assert validate_track("DESERT", d_grid, d_start)
    export_track("assets/levels/track_desert_dunes.txt",
                 "Mario Kart CGI - Valle del Desierto UCV (Dakar Canyon Wedge)",
                 26, 24, d_grid)
                 
    # 2. Espacio (Starfighter Orbital Wings)
    c_grid, c_start = build_cosmic()
    assert validate_track("COSMIC", c_grid, c_start)
    export_track("assets/levels/track_cosmic_space.txt",
                 "Mario Kart CGI - Circuito Espacial UCV (Starfighter Orbital Highway)",
                 26, 26, c_grid)
                 
    # 3. Acuático (Atolón y Laguna Marina)
    a_grid, a_start = build_aquatic()
    assert validate_track("AQUATIC", a_grid, a_start)
    export_track("assets/levels/track_aquatic_reef.txt",
                 "Mario Kart CGI - Arrecife Abisal UCV (Nautilus Reef & Kelp Atoll)",
                 26, 26, a_grid)

if __name__ == "__main__":
    main()
