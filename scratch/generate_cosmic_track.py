"""
Script para disenar y validar el trazado del Circuito Espacial UCV:
assets/levels/track_cosmic_space.txt
"""
import sys

def build_cosmic_track():
    cols = 26
    rows = 26
    grid = {}
    
    # Convencion de curvas (verificada):
    # rot 0.0: conecta Oeste (-1, 0) y Norte (0, -1)
    # rot 90.0: conecta Oeste (-1, 0) y Sur (0, 1)
    # rot 180.0: conecta Este (1, 0) y Sur (0, 1)
    # rot 270.0: conecta Este (1, 0) y Norte (0, -1)
    
    # Convencion de rectas:
    # rot 0.0: Norte-Sur (0, -1) a (0, 1)
    # rot 90.0: Este-Oeste (-1, 0) a (1, 0)
    
    # 1. Recta principal de lanzamiento espacial (Y=23, X=4 a 21)
    for x in range(4, 22):
        if x == 6:
            grid[(x, 23)] = (4, 90.0) # Start / Finish
        else:
            grid[(x, 23)] = (1, 90.0) # Recta Este
            
    # 2. Curva 1 hacia el Norte (X=22, Y=23): viene del Oeste, sale al Norte -> rot 0.0
    grid[(22, 23)] = (2, 0.0)
    
    # 3. Subida Este recta hacia el cuadrante superior (X=22, Y=22 a 18)
    for y in range(18, 23):
        grid[(22, y)] = (1, 0.0)
        
    # 4. Curva hacia el Oeste (X=22, Y=17): viene del Sur, sale al Oeste -> rot 90.0
    grid[(22, 17)] = (2, 90.0)
    
    # 5. Tramo Oeste (X=21 a 18, Y=17)
    for x in range(18, 22):
        grid[(x, 17)] = (1, 90.0)
        
    # 6. Chicana Orbital: Curva hacia el Norte (X=17, Y=17): viene del Este, sale al Norte -> rot 270.0
    grid[(17, 17)] = (2, 270.0)
    
    # 7. Subida Norte corta (X=17, Y=16 a 15)
    for y in range(15, 17):
        grid[(17, y)] = (1, 0.0)
        
    # 8. Curva hacia el Este (X=17, Y=14): viene del Sur, sale al Este -> rot 180.0
    grid[(17, 14)] = (2, 180.0)
    
    # 9. Recta Este (X=18 a 22, Y=14)
    for x in range(18, 23):
        grid[(x, 14)] = (1, 90.0)
        
    # 10. Curva hacia el Norte (X=23, Y=14): viene del Oeste, sale al Norte -> rot 0.0
    grid[(23, 14)] = (2, 0.0)
    
    # 11. Subida Norte larga hacia la Estacion Superior (X=23, Y=13 a 5)
    for y in range(5, 14):
        grid[(23, y)] = (1, 0.0)
        
    # 12. Curva Norte-Oeste de alta velocidad (X=23, Y=4): viene del Sur, sale al Oeste -> rot 90.0
    grid[(23, 4)] = (2, 90.0)
    
    # 13. Gran Recta Warp Espacial (X=22 a 5, Y=4)
    for x in range(5, 23):
        grid[(x, 4)] = (1, 90.0)
        
    # 14. Curva Oeste-Sur hacia el cuadrante izquierdo (X=4, Y=4): viene del Este, sale al Sur -> rot 180.0
    grid[(4, 4)] = (2, 180.0)
    
    # 15. Bajada Sur Oeste (X=4, Y=5 a 9)
    for y in range(5, 10):
        grid[(4, y)] = (1, 0.0)
        
    # 16. Curva hacia el Este (X=4, Y=10): viene del Norte, sale al Este -> rot 270.0
    grid[(4, 10)] = (2, 270.0)
    
    # 17. Tramo Este (X=5 a 12, Y=10)
    for x in range(5, 13):
        grid[(x, 10)] = (1, 90.0)
        
    # 18. Horquilla Estelar: Curva hacia el Sur (X=13, Y=10): viene del Oeste, sale al Sur -> rot 90.0
    grid[(13, 10)] = (2, 90.0)
    
    # 19. Tramo Sur central (X=13, Y=11 a 16)
    for y in range(11, 17):
        grid[(13, y)] = (1, 0.0)
        
    # 20. Horquilla Estelar baja: Curva hacia el Oeste (X=13, Y=17): viene del Norte, sale al Oeste -> rot 0.0
    grid[(13, 17)] = (2, 0.0)
    
    # 21. Recta Oeste hacia el borde (X=12 a 5, Y=17)
    for x in range(5, 13):
        grid[(x, 17)] = (1, 90.0)
        
    # 22. Curva hacia el Sur (X=4, Y=17): viene del Este, sale al Sur -> rot 180.0
    grid[(4, 17)] = (2, 180.0)
    
    # 23. Bajada Sur final hacia la recta de meta (X=4, Y=18 a 22)
    for y in range(18, 23):
        grid[(4, y)] = (1, 0.0)
        
    # 24. Curva final conectando con la recta de meta (X=4, Y=23): viene del Norte, sale al Este -> rot 270.0
    grid[(4, 23)] = (2, 270.0)
    
    print(f"Total losas espaciales construidas: {len(grid)}")
    
    # Validar conexidad matematica
    corner_ports = {
        0.0:   [(-1, 0), (0, -1)],
        90.0:  [(-1, 0), (0, 1)],
        180.0: [(1, 0), (0, 1)],
        270.0: [(1, 0), (0, -1)],
    }
    straight_ports = {
        0.0:   [(0, -1), (0, 1)],
        90.0:  [(-1, 0), (1, 0)],
    }
    
    errors = 0
    for pos, (ttype, rot) in sorted(grid.items()):
        adj = [(pos[0]+dc, pos[1]+dr) for dc, dr in [(-1,0), (1,0), (0,-1), (0,1)] if (pos[0]+dc, pos[1]+dr) in grid]
        rel_adj = [(p[0]-pos[0], p[1]-pos[1]) for p in adj]
        
        if ttype in [2, 3]:
            ports = corner_ports.get(rot)
            if set(rel_adj) != set(ports):
                print(f"  [ERROR CORNER] en {pos}: rot={rot}, ports={rel_adj}, deberia ser: {[r for r, p in corner_ports.items() if set(p) == set(rel_adj)]}")
                errors += 1
        elif ttype in [1, 4]:
            ports = straight_ports.get(rot)
            if set(rel_adj) != set(ports):
                print(f"  [ERROR STRAIGHT] en {pos}: rot={rot}, ports={rel_adj}")
                errors += 1
                
    if errors == 0:
        print("  Loop espacial 100% verificado y matematicamente perfecto!")
    else:
        print(f"  ATENCION: {errors} errores detectados.")
        return False
        
    outputPath = "assets/levels/track_cosmic_space.txt"
    with open(outputPath, "w") as f:
        f.write("# Mario Kart CGI - Circuito Espacial UCV (Rainbow Cosmic Highway)\n")
        f.write(f"GRID {cols} {rows}\n")
        for (x, y), (tileType, rot) in sorted(grid.items(), key=lambda k: (k[0][1], k[0][0])):
            f.write(f"TILE {x} {y} {tileType} {rot:.1f} 0\n")
            
    print(f"Circuito espacial exportado a: {outputPath}")
    return True

if __name__ == "__main__":
    build_cosmic_track()
