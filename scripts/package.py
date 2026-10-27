#!/usr/bin/env python3
"""
Script de Empaquetado multiplataforma para Entrega UCV - Computación Gráfica
Genera el archivo ZIP con el formato PROY3_CEDULA1_CEDULA2[_CEDULA3].zip
excluyendo directorios de compilación, control de versiones y temporales.
"""

import os
import sys
import zipfile
from pathlib import Path

EXCLUDED_DIRS = {
    ".git", "build", ".cache", ".vs", ".vscode", ".idea", "__pycache__", ".agents"
}

EXCLUDED_EXTENSIONS = {
    ".exe", ".obj", ".o", ".a", ".lib", ".dll", ".pdb", ".ilk", ".exp", ".zip", ".tar", ".gz"
}

EXCLUDED_FILENAMES = {
    ".gitkeep", ".gitignore", ".DS_Store"
}

INCLUDED_ROOT_PATHS = [
    "src",
    "include",
    "shaders",
    "assets",
    "vendor",
    "scripts",
    "tests",
    "docs",
    "CMakeLists.txt",
    "README.md"
]

def main():
    root_dir = Path(__file__).resolve().parent.parent
    os.chdir(root_dir)

    args = sys.argv[1:]
    if len(args) == 0:
        zip_name = "PROY3_CEDULA1_CEDULA2_CEDULA3.zip"
        print(f"Aviso: No se indicaron cédulas. Usando nombre genérico: {zip_name}")
        print("Uso: python scripts/package.py <CEDULA1> <CEDULA2> [CEDULA3]")
    elif len(args) == 1:
        zip_name = f"PROY3_{args[0]}.zip"
    elif len(args) == 2:
        zip_name = f"PROY3_{args[0]}_{args[1]}.zip"
    else:
        zip_name = f"PROY3_{args[0]}_{args[1]}_{args[2]}.zip"

    zip_path = root_dir / zip_name
    if zip_path.exists():
        zip_path.unlink()

    print("=" * 60)
    print(f" Empaquetando proyecto para entrega UCV: {zip_name}")
    print("=" * 60)

    file_count = 0
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for item_name in INCLUDED_ROOT_PATHS:
            item_path = root_dir / item_name
            if not item_path.exists():
                continue

            if item_path.is_file():
                zipf.write(item_path, arcname=item_name)
                file_count += 1
            elif item_path.is_dir():
                for dirpath, dirnames, filenames in os.walk(item_path):
                    # Filtrar directorios excluidos
                    dirnames[:] = [d for d in dirnames if d not in EXCLUDED_DIRS]
                    rel_dir = Path(dirpath).relative_to(root_dir)

                    for filename in filenames:
                        if filename in EXCLUDED_FILENAMES:
                            continue
                        file_ext = os.path.splitext(filename)[1].lower()
                        if file_ext in EXCLUDED_EXTENSIONS:
                            # Los modelos 3D Wavefront (.obj) dentro de assets/ son esenciales para el juego
                            if file_ext == ".obj" and "assets" in rel_dir.parts:
                                pass
                            else:
                                continue
                        full_path = Path(dirpath) / filename
                        arcname = (rel_dir / filename).as_posix()
                        zipf.write(full_path, arcname=arcname)
                        file_count += 1

    size_mb = zip_path.stat().st_size / (1024 * 1024)
    print(f"[OK] Archivo comprimido generado: {zip_name}")
    print(f"Total de archivos empaquetados: {file_count}")
    print(f"Tamaño final: {size_mb:.2f} MB")
    print(f"Destinatario: bryansilva.dev@gmail.com")
    print(f"Asunto del correo: {zip_name}")
    print("=" * 60)

if __name__ == "__main__":
    main()
