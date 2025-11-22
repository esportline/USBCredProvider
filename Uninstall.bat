@echo off
REM Script de désinstallation du USB Credential Provider
REM Doit être exécuté en tant qu'administrateur

echo ================================================
echo  Desinstallation du USB Credential Provider
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

echo [1/2] Desinscription du registre...
regedit /s Unregister.reg
echo      OK - Provider desinscrit

echo.
echo [2/2] Suppression des fichiers...
if exist "C:\Program Files\USBCredProvider\" (
    rmdir /s /q "C:\Program Files\USBCredProvider"
    echo      OK - Fichiers supprimes
) else (
    echo      ATTENTION: Dossier non trouve
)

echo.
echo ================================================
echo   Desinstallation terminee avec succes!
echo ================================================
echo.
echo IMPORTANT: Redemarrez Windows pour terminer la desinstallation
echo.
pause
