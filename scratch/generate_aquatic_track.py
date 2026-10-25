"""
Script para disenar y validar el trazado del Circuito Arrecife Abisal UCV:
assets/levels/track_aquatic_reef.txt
"""
import sys

def build_aquatic_track():
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
    
    # 1. Gran Recta Abisal de Salida (Y=22, X=5 a 20)
    for x in range(5, 21):
        if x == 7:
            grid[(x, 22)] = (4, 90.0) # Start / Finish
        else:
            grid[(x, 22)] = (1, 90.0)
            
    # 2. Curva 1 hacia la fosa este (X=21, Y=22): viene del Oeste, sale al Norte -> rot 0.0
    grid[(21, 22)] = (2, 0.0)
    
    # 3. Subida por la pared de coral este (X=21, Y=21 a 16)
    for y in range(16, 22):
        grid[(21, y)] = (1, 0.0)
        
    # 4. Curva hacia el cañon interior (X=21, Y=15): viene del Sur, sale al Oeste -> rot 90.0
    grid[(21, 15)] = (2, 90.0)
    
    # 5. Tramo Oeste (X=20 a 17, Y=15)
    for x in range(17, 21):
        grid[(x, 15)] = (1, 90.0)
        
    # 6. Chicana de Corales: Curva al Norte (X=16, Y=15): viene del Este, sale al Norte -> rot 270.0
    grid[(16, 15)] = (2, 270.0)
    
    # 7. Subida corta (X=16, Y=14 a 12)
    for y in range(12, 15):
        grid[(16, y)] = (1, 0.0)
        
    # 8. Curva hacia el Este (X=16, Y=11): viene del Sur, sale al Este -> rot 180.0
    grid[(16, 11)] = (2, 180.0)
    
    # 9. Recta Este (X=17 a 21, Y=11)
    for x in range(17, 22):
        grid[(x, 11)] = (1, 90.0)
        
    # 10. Curva al Norte hacia la laguna superior (X=22, Y=11): viene del Oeste, sale al Norte -> rot 0.0
    grid[(22, 11)] = (2, 0.0)
    
    # 11. Subida Norte hacia el Santuario de Perlas (X=22, Y=10 a 5)
    for y in range(5, 11):
        grid[(22, y)] = (1, 0.0)
        
    # 12. Curva Norte-Oeste (X=22, Y=4): viene del Sur, sale al Oeste -> rot 90.0
    grid[(22, 4)] = (2, 90.0)
    
    # 13. Gran Recta Submarina de la Superficie (X=21 a 5, Y=4)
    for x in range(5, 22):
        grid[(x, 4)] = (1, 90.0)
        
    # 14. Curva Oeste-Sur hacia el bosque de algas (X=4, Y=4): viene del Este, sale al Sur -> rot 180.0
    grid[(4, 4)] = (2, 180.0)
    
    # 15. Bajada Sur por el bosque de algas (X=4, Y=5 a 9)
    for y in range(5, 10):
        grid[(4, y)] = (1, 0.0)
        
    # 16. Curva hacia la meseta central (X=4, Y=10): viene del Norte, sale al Este -> rot 270.0
    grid[(4, 10)] = (2, 270.0)
    
    # 17. Tramo Este (X=5 a 10, Y=10)
    for x in range(5, 11):
        grid[(x, 10)] = (1, 90.0)
        
    # 18. Curva al Sur (X=11, Y=10): viene del Oeste, sale al Sur -> rot 90.0
    grid[(11, 10)] = (2, 90.0)
    
    # 19. Tramo Sur (X=11, Y=11 a 16)
    for y in range(11, 17):
        grid[(11, y)] = (1, 0.0)
        
    # 20. Curva hacia el Oeste (X=11, Y=17): viene del Norte, sale al Oeste -> rot 0.0
    grid[(11, 17)] = (2, 0.0)
    
    # 21. Recta Oeste hacia el arrecife exterior (X=10 a 5, Y=17)
    for x in range(5, 11):
        grid[(x, 17)] = (1, 90.0)
        
    # 22. Curva hacia el Sur (X=4, Y=17): viene del Este, sale al Sur -> rot 180.0
    grid[(4, 17)] = (2, 180.0)
    
    # 23. Bajada Sur final hacia la recta de meta (X=4, Y=18 a 21)
    for y in range(18, 22):
        grid[(4, y)] = (1, 0.0)
        
    # 24. Curva final conectando con la recta de salida (X=4, Y=22): viene del Norte, sale al Este -> rot 270.0
    grid[(4, 22)] = (2, 270.0)
    
    print(f"Total losas acuáticas construidas: {len(grid)}")
    
    # Validar conexidad matemática
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
        print("  Loop acuático 100% verificado y matemáticamente perfecto!")
    else:
        print(f"  ATENCION: {errors} errores detectados.")
        return False
        
    outputPath = "assets/levels/track_aquatic_reef.txt"
    with open(outputPath, "w") as f:
        f.write("# Mario Kart CGI - Circuito Arrecife Abisal UCV (Aquatic Deep Reef)\n")
        f.write(f"GRID {cols} {rows}\n")
        for (x, y), (tileType, rot) in sorted(grid.items(), key=lambda k: (k[0][1], k[0][0])):
            f.write(f"TILE {x} {y} {tileType} {rot:.1f} 0\n")
            
    print(f"Circuito acuático exportado a: {outputPath}")
    return True

if __name__ == "__main__":
    build_aquatic_track()
