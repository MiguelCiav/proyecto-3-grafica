"""
Script de diseño y validación para el Circuito Caldera del Volcán UCV:
assets/levels/track_volcano_caldera.txt
Geometría: The Volcano Caldera / Magma Ring (Crater Asimétrico y Curvas Cerradas)
"""
import sys, os

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
            print(f"[{name}] ERROR en {pos}: tiene {len(rel_adj)} vecinos conectados: {rel_adj}")
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
        print(f"[{name}] ERROR: El circuito tiene {len(grid)} losas pero el bucle solo recorre {len(visited)}.")
        errors += 1
        
    if errors == 0:
        print(f"[{name}] OK! {len(grid)} losas, bucle 100% verificado y continuo.")
        return True
    else:
        print(f"[{name}] FAILED: {errors} errores detectados.")
        return False

def build_volcano():
    grid = {}
    # 1. Recta principal sur de la caldera (Y=21, X=11 a 22)
    # Start / Finish en (14, 21)
    for x in range(11, 23):
        if x == 14:
            grid[(x, 21)] = (4, 90.0)
        else:
            grid[(x, 21)] = (1, 90.0)
            
    # 2. Subida por la pared este del volcán (X=23, Y=20 a 16)
    grid[(23, 21)] = (2, 0.0) # Oeste (22, 21) y Norte (23, 20)
    for y in range(16, 21):
        grid[(23, y)] = (1, 0.0)
        
    # 3. Horquilla de Lava: Giro hacia el interior del cráter (Y=15, X=22 a 16)
    grid[(23, 15)] = (2, 90.0) # Sur (23, 16) y Oeste (22, 15)
    for x in range(16, 23):
        grid[(x, 15)] = (1, 90.0)
        
    # 4. Chicana volcánica: Sube Norte (X=15, Y=14 a 11)
    grid[(15, 15)] = (2, 270.0) # Este (16, 15) y Norte (15, 14)
    for y in range(11, 15):
        grid[(15, y)] = (1, 0.0)
        
    # 5. Salida de la chicana hacia la cornisa este (Y=10, X=16 a 21)
    grid[(15, 10)] = (2, 180.0) # Sur (15, 11) y Este (16, 10)
    for x in range(16, 22):
        grid[(x, 10)] = (1, 90.0)
        
    # 6. Ascenso final al borde superior del cráter (X=22, Y=9 a 5)
    grid[(22, 10)] = (2, 0.0) # Oeste (21, 10) y Norte (22, 9)
    for y in range(5, 10):
        grid[(22, y)] = (1, 0.0)
        
    # 7. Gran Recta Norte sobre la cresta del volcán (Y=4, X=21 a 4)
    grid[(22, 4)] = (2, 90.0) # Sur (22, 5) y Oeste (21, 4)
    for x in range(4, 22):
        grid[(x, 4)] = (1, 90.0)
        
    # 8. Bajada vertiginosa oeste por las coladas de lava (X=3, Y=5 a 17)
    grid[(3, 4)] = (2, 180.0) # Este (4, 4) y Sur (3, 5)
    for y in range(5, 18):
        grid[(3, y)] = (1, 0.0)
        
    # 9. Retorno suroeste hacia la caldera (Y=18, X=4 a 9)
    grid[(3, 18)] = (2, 270.0) # Norte (3, 17) y Este (4, 18)
    for x in range(4, 10):
        grid[(x, 18)] = (1, 90.0)
        
    # 10. Conexión final sur (X=10, Y=19 a 20)
    grid[(10, 18)] = (2, 90.0) # Oeste (9, 18) y Sur (10, 19)
    for y in range(19, 21):
        grid[(10, y)] = (1, 0.0)
        
    # 11. Cierre con la recta de salida
    grid[(10, 21)] = (2, 270.0) # Norte (10, 20) y Este (11, 21)
    
    return grid, (14, 21)

if __name__ == "__main__":
    v_grid, v_start = build_volcano()
    valid = validate_track("VOLCANO CALDERA", v_grid, v_start)
    if valid:
        print(f"Total losas: {len(v_grid)}")
        out_path = "assets/levels/track_volcano_caldera.txt"
        with open(out_path, "w") as f:
            f.write("# Mario Kart CGI - Caldera del Volcan UCV (Inferno Magma Crater)\n")
            f.write("GRID 26 26\n")
            for (x, y), (tileType, rot) in sorted(v_grid.items(), key=lambda k: (k[0][1], k[0][0])):
                f.write(f"TILE {x} {y} {tileType} {rot:.1f} 0\n")
        print(f"[EXPORT] Circuito volcanico exportado a: {out_path}")
