#!/bin/bash
# Script de compilation rapide pour USB Credential Provider

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║    USB Credential Provider - Script de compilation      ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════╝${NC}"
echo ""

# Vérifier si MinGW est installé
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo -e "${RED}ERREUR: MinGW-w64 n'est pas installé${NC}"
    echo ""
    echo "Installation sur Debian/Ubuntu:"
    echo "  sudo apt-get install mingw-w64"
    echo ""
    echo "Installation sur Fedora:"
    echo "  sudo dnf install mingw64-gcc mingw64-gcc-c++"
    echo ""
    echo "Installation sur Arch:"
    echo "  sudo pacman -S mingw-w64-gcc"
    exit 1
fi

echo -e "${GREEN}✓ MinGW-w64 trouvé${NC}"
x86_64-w64-mingw32-g++ --version | head -n1
echo ""

# Choix de la méthode de compilation
echo -e "${YELLOW}Méthode de compilation:${NC}"
echo "  1) Make (rapide)"
echo "  2) CMake (recommandé)"
echo ""
read -p "Choisir [1 ou 2, défaut: 1]: " choice
choice=${choice:-1}

if [ "$choice" = "2" ]; then
    # Compilation avec CMake
    echo ""
    echo -e "${CYAN}Compilation avec CMake...${NC}"

    mkdir -p build
    cd build

    cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake
    cmake --build .

    cd ..

    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║              Compilation réussie (CMake) !               ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${CYAN}Fichiers générés:${NC}"
    echo "  • build/bin/USBCredProvider.dll"
    echo "  • build/bin/tools/list_usb_devices.exe"
else
    # Compilation avec Make
    echo ""
    echo -e "${CYAN}Compilation avec Make...${NC}"

    make clean 2>/dev/null || true
    make -j$(nproc)

    echo ""
fi

echo ""
echo -e "${YELLOW}Prochaines étapes:${NC}"
echo "  1. Transférer les fichiers sur Windows:"
echo "     - build/bin/USBCredProvider.dll"
echo "     - build/bin/tools/list_usb_devices.exe"
echo "     - Install.bat"
echo "     - Register.reg"
echo "     - server_example.py (optionnel)"
echo ""
echo "  2. Sur Windows, exécuter Install.bat en tant qu'administrateur"
echo ""
echo "  3. Configurer le serveur HTTP dans le registre:"
echo "     HKLM\\SOFTWARE\\USBCredProvider\\ServerUrl"
echo ""
echo "  4. Redémarrer Windows"
echo ""
