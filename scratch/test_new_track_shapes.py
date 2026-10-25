"""
Script de diseño y validación de 3 geometrías únicas para Mario Kart CGI:
1. Desert Dunes: The Dakar Wedge / Canyon Triangle (Triangular / Asimétrico)
2. Cosmic Space: The Starfighter / Orbital Station (Alas de Satélite y Apex Norte)
3. Aquatic Reef: The Sea Atoll / Nautilus Lagoon (Laguna orgánica y Serpentina)
"""
import sys

CORNER_PORTS = {
    0.0:   [(-1, 0), (0, -1)], # Oeste y Norte
    90.0:  [(-1, 0), (0, 1)],  # Oeste y Sur
    180.0: [(1, 0), (0, 1)],   # Este y Sur
    270.0: [(1, 0), (0, -1)],  # Este y Norte
}

STRAIGHT_PORTS = {
    0.0:   [(0, -1), (0, 1)],  # Norte y Sur
    90.0:  [(-1, 0), (1, 0)],  # Oeste y Este
}

def validate_track(name, grid, start_pos):
    errors = 0
    if start_pos not in grid or grid[start_pos][0] != 4:
        print(f"[{name}] ERROR: Posicion de salida {start_pos} invalida o no es START_FINISH (tipo 4)")
        return False
        
    for pos, (ttype, rot) in sorted(grid.items()):
        adj = [(pos[0]+dc, pos[1]+dr) for dc, dr in [(-1,0), (1,0), (0,-1), (0,1)] if (pos[0]+dc, pos[1]+dr) in grid]
        rel_adj = [(p[0]-pos[0], p[1]-pos[1]) for p in adj]
        
        if len(rel_adj) != 2:
            print(f"[{name}] ERROR en {pos}: tiene {len(rel_adj)} vecinos conectados (debe tener exactamente 2): {rel_adj}")
            errors += 1
            continue
            
        if ttype in [2, 3]:
            ports = CORNER_PORTS.get(rot)
            if set(rel_adj) != set(ports):
                print(f"[{name}] ERROR CORNER en {pos}: rot={rot}, ports={rel_adj}, deberia ser: {[r for r, p in CORNER_PORTS.items() if set(p) == set(rel_adj)]}")
                errors += 1
        elif ttype in [1, 4]:
            ports = STRAIGHT_PORTS.get(rot)
            if set(rel_adj) != set(ports):
                print(f"[{name}] ERROR STRAIGHT en {pos}: rot={rot}, ports={rel_adj}")
                errors += 1
                
    # Validar conexidad completa en un solo bucle
    visited = set()
    curr = start_pos
    prev = None
    while True:
        visited.add(curr)
        adj = [(curr[0]+dc, curr[1]+dr) for dc, dr in [(-1,0), (1,0), (0,-1), (0,1)] if (curr[0]+dc, curr[1]+dr) in grid]
        next_nodes = [p for p in adj if p != prev]
        if not next_nodes:
            break
        next_node = next_nodes[0]
        prev = curr
        curr = next_node
        if curr == start_pos:
            break
            
    if len(visited) != len(grid):
        print(f"[{name}] ERROR: El circuito tiene {len(grid)} losas pero el bucle solo recorre {len(visited)} (hay islas o bifurcaciones).")
        errors += 1
        
    if errors == 0:
        print(f"[{name}] OK! {len(grid)} losas, bucle 100% verificado y continuo.")
        return True
    else:
        print(f"[{name}] FAILED: {errors} errores detectados.")
        return False

# ==============================================================================
# 1. DISENO DESIERTO: The Dakar Canyon Wedge (Triangular Asimétrico)
# ==============================================================================
def build_desert():
    grid = {}
    # Recta sur principal (Y=21, X=4 a 22)
    for x in range(4, 23):
        if x == 7:
            grid[(x, 21)] = (4, 90.0) # Start / Finish
        else:
            grid[(x, 21)] = (1, 90.0)
            
    # Subida este corta (X=23)
    grid[(23, 21)] = (2, 0.0) # Conecta Oeste (22, 21) y Norte (23, 20)
    for y in range(13, 21):
        grid[(23, y)] = (1, 0.0)
        
    # Escalón 1 hacia el interior (Y=12)
    grid[(23, 12)] = (2, 90.0) # Conecta Sur (23, 13) y Oeste (22, 12)
    for x in range(19, 23):
        grid[(x, 12)] = (1, 90.0)
        
    # Subida escalón 1 (X=18)
    grid[(18, 12)] = (2, 270.0) # Conecta Este (19, 12) y Norte (18, 11)
    for y in range(10, 12):
        grid[(18, y)] = (1, 0.0)
        
    # Escalón 2 hacia el interior (Y=9)
    grid[(18, 9)] = (2, 90.0) # Conecta Sur (18, 10) y Oeste (17, 9)
    for x in range(14, 18):
        grid[(x, 9)] = (1, 90.0)
        
    # Subida escalón 2 (X=13)
    grid[(13, 9)] = (2, 270.0) # Conecta Este (14, 9) y Norte (13, 8)
    for y in range(7, 9):
        grid[(13, y)] = (1, 0.0)
        
    # Escalón 3 hacia la cumbre norte (Y=6)
    grid[(13, 6)] = (2, 90.0) # Conecta Sur (13, 7) y Oeste (12, 6)
    for x in range(9, 13):
        grid[(x, 6)] = (1, 90.0)
        
    # Subida final a la cumbre (X=8)
    grid[(8, 6)] = (2, 270.0) # Conecta Este (9, 6) y Norte (8, 5)
    for y in range(4, 6):
        grid[(8, y)] = (1, 0.0)
        
    # Recta norte de la cumbre (Y=3)
    grid[(8, 3)] = (2, 90.0) # Conecta Sur (8, 4) y Oeste (7, 3)
    for x in range(4, 8):
        grid[(x, 3)] = (1, 90.0)
        
    # Gran bajada del cañón por el Oeste (X=3)
    grid[(3, 3)] = (2, 180.0) # Conecta Este (4, 3) y Sur (3, 4)
    for y in range(4, 21):
        grid[(3, y)] = (1, 0.0)
        
    # Cierre con la recta sur
    grid[(3, 21)] = (2, 270.0) # Conecta Norte (3, 20) y Este (4, 21)
    
    return grid, (7, 21)

# ==============================================================================
# 2. DISENO ESPACIO: The Starfighter / Orbital Station (Alas de Satélite)
# ==============================================================================
def build_cosmic():
    grid = {}
    # Recta sur central de lanzamiento (Y=22, X=8 a 18)
    for x in range(8, 19):
        if x == 10:
            grid[(x, 22)] = (4, 90.0)
        else:
            grid[(x, 22)] = (1, 90.0)
            
    # Giro este hacia la estación de combustible (X=19, Y=22)
    grid[(19, 22)] = (2, 0.0) # Oeste (18, 22) y Norte (19, 21)
    for y in range(19, 22):
        grid[(19, y)] = (1, 0.0)
        
    # Ala Este exterior (Y=18, sale a X=24)
    grid[(19, 18)] = (2, 180.0) # Sur (19, 19) y Este (20, 18)
    for x in range(20, 24):
        grid[(x, 18)] = (1, 90.0)
        
    # Subida por el ala este exterior (X=24, Y=17 a 10)
    grid[(24, 18)] = (2, 0.0) # Oeste (23, 18) y Norte (24, 17)
    for y in range(10, 18):
        grid[(24, y)] = (1, 0.0)
        
    # Retorno del ala este hacia el fuselaje central (Y=9)
    grid[(24, 9)] = (2, 90.0) # Sur (24, 10) y Oeste (23, 9)
    for x in range(18, 24):
        grid[(x, 9)] = (1, 90.0)
        
    # Subida al Apex Norte (X=17, Y=8 a 4)
    grid[(17, 9)] = (2, 270.0) # Este (18, 9) y Norte (17, 8)
    for y in range(4, 9):
        grid[(17, y)] = (1, 0.0)
        
    # Corona Norte del hiperespacio (Y=3, X=16 a 10)
    grid[(17, 3)] = (2, 90.0) # Sur (17, 4) y Oeste (16, 3)
    for x in range(10, 17):
        grid[(x, 3)] = (1, 90.0)
        
    # Bajada desde el Apex Norte por el lado Oeste (X=9, Y=4 a 8)
    grid[(9, 3)] = (2, 180.0) # Este (10, 3) y Sur (9, 4)
    for y in range(4, 9):
        grid[(9, y)] = (1, 0.0)
        
    # Salida hacia el Ala Oeste exterior (Y=9)
    grid[(9, 9)] = (2, 0.0) # Norte (9, 8) y Oeste (8, 9)
    for x in range(3, 9):
        grid[(x, 9)] = (1, 90.0)
        
    # Bajada por el ala oeste exterior (X=2, Y=10 a 17)
    grid[(2, 9)] = (2, 180.0) # Este (3, 9) y Sur (2, 10)
    for y in range(10, 18):
        grid[(2, y)] = (1, 0.0)
        
    # Retorno del ala oeste hacia el fuselaje (Y=18, X=3 a 6)
    grid[(2, 18)] = (2, 270.0) # Norte (2, 17) y Este (3, 18)
    for x in range(3, 7):
        grid[(x, 18)] = (1, 90.0)
        
    # Bajada final hacia la recta de meta (X=7, Y=19 a 21)
    grid[(7, 18)] = (2, 90.0) # Oeste (6, 18) y Sur (7, 19)
    for y in range(19, 22):
        grid[(7, y)] = (1, 0.0)
        
    # Conexión con la recta sur de salida
    grid[(7, 22)] = (2, 270.0) # Norte (7, 21) y Este (8, 22)
    
    return grid, (10, 22)

# ==============================================================================
# 3. DISENO ACUATICO: The Sea Atoll / Nautilus Lagoon (Laguna y Serpentina)
# ==============================================================================
def build_aquatic():
    grid = {}
    # Laguna sur principal (Y=21, X=7 a 21)
    for x in range(7, 22):
        if x == 9:
            grid[(x, 21)] = (4, 90.0)
        else:
            grid[(x, 21)] = (1, 90.0)
            
    # Subida este de la barrera de coral (X=22, Y=17 a 20)
    grid[(22, 21)] = (2, 0.0) # Oeste (21, 21) y Norte (22, 20)
    for y in range(17, 21):
        grid[(22, y)] = (1, 0.0)
        
    # Cuello del atolón hacia el interior (Y=16, X=21 a 17)
    grid[(22, 16)] = (2, 90.0) # Sur (22, 17) y Oeste (21, 16)
    for x in range(17, 22):
        grid[(x, 16)] = (1, 90.0)
        
    # Serpentina del bosque de algas kelp (S-Chicane fluida):
    # Sube Norte a Y=13
    grid[(16, 16)] = (2, 270.0) # Este (17, 16) y Norte (16, 15)
    for y in range(13, 16):
        grid[(16, y)] = (1, 0.0)
        
    # Gira Este a X=20
    grid[(16, 12)] = (2, 180.0) # Sur (16, 13) y Este (17, 12)
    for x in range(17, 21):
        grid[(x, 12)] = (1, 90.0)
        
    # Sube Norte a Y=5
    grid[(21, 12)] = (2, 0.0) # Oeste (20, 12) y Norte (21, 11)
    for y in range(5, 12):
        grid[(21, y)] = (1, 0.0)
        
    # Gran recta de aguas superficiales iluminadas por el sol (Y=4, X=20 a 5)
    grid[(21, 4)] = (2, 90.0) # Sur (21, 5) y Oeste (20, 4)
    for x in range(5, 21):
        grid[(x, 4)] = (1, 90.0)
        
    # Fosa abisal oeste: bajada profunda (X=4, Y=5 a 12)
    grid[(4, 4)] = (2, 180.0) # Este (5, 4) y Sur (4, 5)
    for y in range(5, 13):
        grid[(4, y)] = (1, 0.0)
        
    # Ensenada de conchas hacia el Oeste (Y=13, X=3 a 2)
    grid[(4, 13)] = (2, 0.0) # Norte (4, 12) y Oeste (3, 13)
    grid[(3, 13)] = (1, 90.0)
    
    # Bajada por la ensenada oeste (X=2, Y=14 a 17)
    grid[(2, 13)] = (2, 180.0) # Este (3, 13) y Sur (2, 14)
    for y in range(14, 18):
        grid[(2, y)] = (1, 0.0)
        
    # Retorno de la ensenada hacia la laguna sur (Y=18, X=3 a 5)
    grid[(2, 18)] = (2, 270.0) # Norte (2, 17) y Este (3, 18)
    for x in range(3, 6):
        grid[(x, 18)] = (1, 90.0)
        
    # Bajada final hacia la recta de meta (X=6, Y=19 a 20)
    grid[(6, 18)] = (2, 90.0) # Oeste (5, 18) y Sur (6, 19)
    for y in range(19, 21):
        grid[(6, y)] = (1, 0.0)
        
    # Cierre con la laguna sur
    grid[(6, 21)] = (2, 270.0) # Norte (6, 20) y Este (7, 21)
    
    return grid, (9, 21)

if __name__ == "__main__":
    d_grid, d_start = build_desert()
    validate_track("DESERT DUNES (Wedge)", d_grid, d_start)
    
    c_grid, c_start = build_cosmic()
    validate_track("COSMIC SPACE (Starfighter)", c_grid, c_start)
    
    a_grid, a_start = build_aquatic()
    validate_track("AQUATIC REEF (Atoll)", a_grid, a_start)
