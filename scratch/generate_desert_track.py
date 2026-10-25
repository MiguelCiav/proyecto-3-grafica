"""
Script para generar y validar topologicamente el circuito del desierto:
assets/levels/track_desert_dunes.txt
"""
import sys

def build_desert_track():
    cols = 26
    rows = 24
    
    # Matriz del circuito
    # Tipos: 1 = Straight, 2 = Corner Small, 3 = Corner Large, 4 = Start/Finish
    grid = {}
    
    # Diseno del trazado:
    # Inicio en recta sur (Y=21):
    # Meta en (6, 21) direccion E (90 deg)
    
    # 1. Recta principal de meta (Y=21, X=4 a 21)
    for x in range(4, 22):
        if x == 6:
            grid[(x, 21)] = (4, 90.0)
        else:
            grid[(x, 21)] = (1, 90.0)
            
    # 2. Curva 1 hacia el norte (X=22, Y=21) -> Corner Small 0 deg
    grid[(22, 21)] = (2, 0.0)
    
    # 3. Subida norte este (X=22, Y=20 a 17)
    for y in range(17, 21):
        grid[(22, y)] = (1, 0.0)
        
    # 4. Curva hacia el oeste (X=22, Y=16) -> Corner Small 90 deg
    grid[(22, 16)] = (2, 90.0)
    
    # 5. Tramo hacia el oeste (X=21 a 19, Y=16)
    for x in range(19, 22):
        grid[(x, 16)] = (1, 90.0)
        
    # 6. Chicane de las dunas: giro norte (X=18, Y=16) -> Corner Small 270 deg
    grid[(18, 16)] = (2, 270.0)
    
    # 7. Subida corta norte (X=18, Y=15 a 14)
    for y in range(14, 16):
        grid[(18, y)] = (1, 0.0)
        
    # 8. Giro este (X=18, Y=13) -> Corner Small 180 deg
    grid[(18, 13)] = (2, 180.0)
    
    # 9. Recta este (X=19 a 22, Y=13)
    for x in range(19, 23):
        grid[(x, 13)] = (1, 90.0)
        
    # 10. Curva hacia el norte (X=23, Y=13) -> Corner Small 0 deg
    grid[(23, 13)] = (2, 0.0)
    
    # 11. Subida norte hacia el canon (X=23, Y=12 a 6)
    for y in range(6, 13):
        grid[(23, y)] = (1, 0.0)
        
    # 12. Curva cerrada hacia el oeste en la cumbre norte (X=23, Y=5) -> Corner Small 90 deg
    grid[(23, 5)] = (2, 90.0)
    
    # 13. Gran recta norte entre canones de arenisca (X=22 a 5, Y=5)
    for x in range(5, 23):
        grid[(x, 5)] = (1, 90.0)
        
    # 14. Curva oeste hacia el sur (X=4, Y=5) -> Corner Small 180 deg
    grid[(4, 5)] = (2, 180.0)
    
    # 15. Bajada sur oeste (X=4, Y=6 a 10)
    for y in range(6, 11):
        grid[(4, y)] = (1, 0.0)
        
    # 16. Curva hacia el este (X=4, Y=11) -> Corner Small 270 deg
    grid[(4, 11)] = (2, 270.0)
    
    # 17. Tramo hacia el este hacia el oasis (X=5 a 12, Y=11)
    for x in range(5, 13):
        grid[(x, 11)] = (1, 90.0)
        
    # 18. Horquilla del oasis: giro sur (X=13, Y=11) -> Corner Small 0 deg
    grid[(13, 11)] = (2, 0.0)
    
    # 19. Tramo sur (X=13, Y=12 a 15)
    for y in range(12, 16):
        grid[(13, y)] = (1, 0.0)
        
    # 20. Giro oeste (X=13, Y=16) -> Corner Small 90 deg
    grid[(13, 16)] = (2, 90.0)
    
    # 21. Recta oeste (X=12 a 5, Y=16)
    for x in range(5, 13):
        grid[(x, 16)] = (1, 90.0)
        
    # 22. Curva sur final hacia la recta de meta (X=4, Y=16) -> Corner Small 180 deg
    grid[(4, 16)] = (2, 180.0)
    
    # 23. Bajada sur final conectando con la meta (X=4, Y=17 a 20)
    for y in range(17, 21):
        grid[(4, y)] = (1, 0.0)
        
    # 24. Curva final conectando con recta de meta (X=4, Y=21) -> Corner Small 270 deg
    # Nota: (4, 21) es inicio de la recta sur
    grid[(4, 21)] = (2, 270.0)
    
    print(f"Total losas construidas: {len(grid)}")
    
    # Escribir archivo de nivel
    outputPath = "assets/levels/track_desert_dunes.txt"
    with open(outputPath, "w") as f:
        f.write("# Mario Kart CGI - Valle del Desierto UCV (Desert Rally GP)\n")
        f.write(f"GRID {cols} {rows}\n")
        for (x, y), (tileType, rot) in sorted(grid.items(), key=lambda k: (k[0][1], k[0][0])):
            f.write(f"TILE {x} {y} {tileType} {rot:.1f} 0\n")
            
    print(f"Circuito exportado con exito a: {outputPath}")

if __name__ == "__main__":
    build_desert_track()
