@echo off
REM Script d'installation du USB Credential Provider
REM Doit être exécuté en tant qu'administrateur

echo ================================================
echo   Installation du USB Credential Provider
echo ================================================
echo.

REM Vérifier les privilèges administrateur
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERREUR: Ce script doit etre execute en tant qu'administrateur
    echo Faites un clic droit sur le fichier et selectionnez "Executer en tant qu'administrateur"
    pause
    exit /b 1
)

echo [1/4] Creation du dossier d'installation...
if not exist "C:\Program Files\USBCredProvider\" (
    mkdir "C:\Program Files\USBCredProvider"
    echo      OK - Dossier cree
) else (
    echo      OK - Dossier existe deja
)

echo.
echo [2/4] Copie de la DLL...
if exist "build\bin\USBCredProvider.dll" (
    copy /Y "build\bin\USBCredProvider.dll" "C:\Program Files\USBCredProvider\"
    echo      OK - DLL copiee
) else (
    echo      ERREUR: build\bin\USBCredProvider.dll n'existe pas
    echo      Veuillez compiler le projet d'abord
    pause
    exit /b 1
)

echo.
echo [3/4] Enregistrement dans le registre...
regedit /s Register.reg
echo      OK - Registre mis a jour

echo.
echo [4/4] Copie de l'outil de diagnostic...
if exist "build\bin\tools\list_usb_devices.exe" (
    copy /Y "build\bin\tools\list_usb_devices.exe" "C:\Program Files\USBCredProvider\"
    echo      OK - Outil copie
) else (
    echo      ATTENTION: Outil de diagnostic non trouve (non critique)
)

echo.
echo ================================================
echo   Installation terminee avec succes!
echo ================================================
echo.
echo IMPORTANT:
echo   1. Configurez l'URL du serveur HTTP dans le registre si necessaire
echo   2. Redemarrez Windows pour activer le provider
echo   3. Utilisez list_usb_devices.exe pour obtenir les numeros de serie
echo.
echo Chemin d'installation: C:\Program Files\USBCredProvider\
echo.
pause
