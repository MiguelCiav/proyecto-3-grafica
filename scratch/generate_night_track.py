import sys

# Track grid dimensions
COLS = 26
ROWS = 22

# Grid representation
# Track tiles: (col, row, type, rot)
tiles = {}

def add_straight_x(row, col_start, col_end):
    step = 1 if col_end >= col_start else -1
    for c in range(col_start, col_end + step, step):
        tiles[(c, row)] = (1, 90.0)

def add_straight_z(col, row_start, row_end):
    step = 1 if row_end >= row_start else -1
    for r in range(row_start, row_end + step, step):
        tiles[(col, r)] = (1, 0.0)

def add_corner(col, row, rot):
    tiles[(col, row)] = (2, float(rot))

# Let's design the Neon Night Circuit:
# 1. Start/Finish Main Straight at row 18, going East from col 4 to col 21:
# Let col 5 be START_FINISH (type 4)
# (4, 18) is SW corner (rot 270)
# (21, 18) is SE corner (rot 0)
add_corner(4, 18, 270.0)
for c in range(5, 21):
    tiles[(c, 18)] = (1, 90.0)
tiles[(6, 18)] = (4, 90.0) # Start / Finish gantry tile
add_corner(21, 18, 0.0)

# 2. East straight heading North from row 17 to row 14:
add_straight_z(21, 17, 14)
# Turn West at (21, 13): corner rot 90
add_corner(21, 13, 90.0)

# 3. Chicane / Infield transition going West from col 20 to col 17:
add_straight_x(13, 20, 17)
# Turn North at (16, 13): corner rot 270
add_corner(16, 13, 270.0)

# 4. Heading North from row 12 to row 9:
add_straight_z(16, 12, 9)
# Turn East at (16, 8): corner rot 180
add_corner(16, 8, 180.0)

# 5. Heading East from col 17 to col 22 (High speed right hook):
add_straight_x(8, 17, 22)
# Turn North at (23, 8): corner rot 0
add_corner(23, 8, 0.0)

# 6. Heading North on col 23 from row 7 to row 4:
add_straight_z(23, 7, 4)
# Northeast Corner at (23, 3): corner rot 90
add_corner(23, 3, 90.0)

# 7. Long North Back Straight from col 22 to col 7 going West:
add_straight_x(3, 22, 7)
# Northwest Corner at (6, 3): corner rot 180
add_corner(6, 3, 180.0)

# 8. Heading South on col 6 from row 4 to row 8:
add_straight_z(6, 4, 8)
# Turn East into Infield Loop at (6, 9): corner rot 270
add_corner(6, 9, 270.0)

# 9. Infield sweep East from col 7 to col 11:
add_straight_x(9, 7, 11)
# Turn South at (12, 9): corner rot 90 (connects West and South)
add_corner(12, 9, 90.0)

# 10. Heading South from row 10 to row 12:
add_straight_z(12, 10, 12)
# Turn West at (12, 13): corner rot 0 (connects North and West)
add_corner(12, 13, 0.0)

# 11. Heading West from col 11 to col 5:
add_straight_x(13, 11, 5)
# Turn South at (4, 13): corner rot 180
add_corner(4, 13, 180.0)

# 12. Heading South on col 4 from row 14 to row 17:
add_straight_z(4, 14, 17)
# Back to (4, 18) which is corner 270!

# VALIDATION:
print(f"Total tiles in circuit: {len(tiles)}")

# Verify topology: each tile must have exactly 2 valid neighbors that match direction
# Directions:
# 0: North (-row), 1: East (+col), 2: South (+row), 3: West (-col)
def get_openings(tile_type, rot):
    if tile_type in (1, 4): # Straight
        if rot in (0.0, 180.0):
            return {0, 2} # North, South
        else:
            return {1, 3} # East, West
    elif tile_type == 2: # Corner
        if rot == 0.0:
            return {0, 3} # North, West
        elif rot == 90.0:
            return {2, 3} # South, West
        elif rot == 180.0:
            return {1, 2} # East, South
        elif rot == 270.0:
            return {0, 1} # North, East
    return set()

opposites = {0: 2, 1: 3, 2: 0, 3: 1}
dpos = {0: (0, -1), 1: (1, 0), 2: (0, 1), 3: (-1, 0)}

valid = True
for (c, r), (t, rot) in tiles.items():
    ops = get_openings(t, rot)
    for d in ops:
        dc, dr = dpos[d]
        nc, nr = c + dc, r + dr
        if (nc, nr) not in tiles:
            print(f"ERROR: Tile at ({c}, {r}) opens to {d} but ({nc}, {nr}) does not exist!")
            valid = False
        else:
            nt, nrot = tiles[(nc, nr)]
            nops = get_openings(nt, nrot)
            if opposites[d] not in nops:
                print(f"ERROR: Tile at ({c}, {r}) connects to ({nc}, {nr}) direction {d}, but neighbor has openings {nops}")
                valid = False

if valid:
    print("SUCCESS: Night Circuit forms a perfectly closed continuous loop!")
    # Output level file
    with open("assets/levels/track_night_circuit.txt", "w", encoding="utf-8") as f:
        f.write("# Mario Kart CGI - Autodromo Nocturno UCV (Neon GP)\n")
        f.write(f"GRID {COLS} {ROWS}\n")
        for (c, r), (t, rot) in sorted(tiles.items(), key=lambda x: (x[0][1], x[0][0])):
            f.write(f"TILE {c} {r} {t} {rot} 0\n")
    print("Saved to assets/levels/track_night_circuit.txt")
else:
    print("Validation failed!")
    sys.exit(1)
