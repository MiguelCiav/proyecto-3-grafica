#!/usr/bin/env bash
# ==============================================================================
# Script de Empaquetado para Entrega UCV - Computación Gráfica
# Formato exigido: PROY3_CEDULA1_CEDULA2.zip (o PROY3_CEDULA1_CEDULA2_CEDULA3.zip)
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${ROOT_DIR}"

if [ "$#" -lt 2 ]; then
    echo "Uso: $0 <CEDULA1> <CEDULA2> [CEDULA3]"
    echo "Ejemplo: $0 12345678 87654321 11223344"
    echo "Usando plantilla por defecto: PROY3_CEDULA1_CEDULA2_CEDULA3.zip"
    ZIP_NAME="PROY3_CEDULA1_CEDULA2_CEDULA3.zip"
elif [ "$#" -eq 2 ]; then
    ZIP_NAME="PROY3_${1}_${2}.zip"
else
    ZIP_NAME="PROY3_${1}_${2}_${3}.zip"
fi

echo "=========================================================="
echo " Empaquetando proyecto para entrega UCV: ${ZIP_NAME}"
echo "=========================================================="

rm -f "${ZIP_NAME}"

zip -r -q "${ZIP_NAME}" \
    src/ \
    include/ \
    shaders/ \
    assets/ \
    vendor/ \
    docs/ \
    CMakeLists.txt \
    README.md \
    scripts/ \
    -x "*.git*" \
    -x "*build*" \
    -x "*.cache*" \
    -x "*.vs*" \
    -x "*.vscode*" \
    -x "*.idea*" \
    -x "*.exe" \
    -x "*.pdb" \
    -x "*.o" \
    -x "*.a" \
    -x "*.zip"

echo "[OK] Archivo generado exitosamente: ${ZIP_NAME}"
echo "Destinatario: bryansilva.dev@gmail.com"
echo "Asunto de correo: ${ZIP_NAME}"
echo "=========================================================="
