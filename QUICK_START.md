# Quick Start - USB Credential Provider

Guide de démarrage rapide pour compiler et installer le USB Credential Provider.

## 🚀 Compilation rapide (Linux)

```bash
# 1. Vérifier MinGW
make check-mingw

# 2. Compiler
make

# Ou utiliser le script automatisé
./build.sh
```

**Résultat** : Les fichiers seront dans `build/bin/`

## 📦 Installation sur Windows

### Étape 1 : Transférer les fichiers

Copier sur votre machine Windows :
```
build/bin/USBCredProvider.dll
build/bin/tools/list_usb_devices.exe
Install.bat
Register.reg
Unregister.reg
server_example.py (optionnel)
```

### Étape 2 : Obtenir les numéros de série USB

```cmd
REM Lancer l'outil de détection
cd chemin\vers\les\fichiers
list_usb_devices.exe
```

Notez les numéros de série affichés (ex: `AA1234567890`)

### Étape 3 : Configurer le serveur HTTP (Optionnel)

**Option A - Serveur Python local** :

```bash
# Éditer server_example.py et ajouter vos serials
python server_example.py 8080
```

**Option B - Votre propre serveur** :

Créez un endpoint qui retourne :
```
SERIAL_NUMBER_1
SERIAL_NUMBER_2
SERIAL_NUMBER_3
```

### Étape 4 : Configurer le registre

Éditer `Register.reg` avant installation :

```reg
[HKEY_LOCAL_MACHINE\SOFTWARE\USBCredProvider]
"ServerUrl"="http://localhost:8080/serials"
```

**OU** configurer les serials en fallback direct dans le registre :

1. Ouvrir `regedit`
2. Naviguer vers `HKEY_LOCAL_MACHINE\SOFTWARE\USBCredProvider`
3. Créer une valeur REG_MULTI_SZ nommée `AuthorizedSerials`
4. Entrer les serials (un par ligne)

### Étape 5 : Installation

```cmd
REM Clic droit sur Install.bat
REM → "Exécuter en tant qu'administrateur"
Install.bat
```

### Étape 6 : Redémarrer

```cmd
shutdown /r /t 0
```

## ✅ Vérification

Après redémarrage :
1. Sur l'écran de connexion, vous devriez voir "USB Credential Provider"
2. Le statut affichera si votre clé USB est détectée
3. Si la clé est autorisée, cliquez sur "Se connecter"

## 🔧 Dépannage rapide

### La DLL ne compile pas
```bash
# Vérifier MinGW
which x86_64-w64-mingw32-g++

# Réinstaller si nécessaire
sudo apt-get install --reinstall mingw-w64
```

### Le provider n'apparaît pas
```cmd
REM Vérifier l'enregistrement
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{5fd3d285-0dd9-4362-8855-e0abaacd4af6}"

REM Vérifier que la DLL existe
dir "C:\Program Files\USBCredProvider\USBCredProvider.dll"
```

### La clé n'est pas détectée
```cmd
REM Re-lancer l'outil de détection
"C:\Program Files\USBCredProvider\list_usb_devices.exe"

REM Vérifier la configuration
reg query "HKLM\SOFTWARE\USBCredProvider" /v AuthorizedSerials
```

### Le serveur HTTP est inaccessible
Le provider utilisera automatiquement le fallback du registre. Assurez-vous que `AuthorizedSerials` est configuré.

## 🗑️ Désinstallation

```cmd
REM En tant qu'administrateur
Uninstall.bat

REM Redémarrer
shutdown /r /t 0
```

## 📖 Documentation complète

Pour plus de détails, consultez le [README.md](README.md) complet.

## 🎯 Résumé des commandes

### Linux
```bash
make                # Compiler
make clean          # Nettoyer
make rebuild        # Recompiler
make check-mingw    # Vérifier MinGW
make help           # Aide
./build.sh          # Script interactif
```

### Windows
```cmd
list_usb_devices.exe    # Lister les USB
Install.bat             # Installer (admin)
Uninstall.bat           # Désinstaller (admin)
```

### Python (serveur)
```bash
python server_example.py 8080    # Lancer serveur
curl http://localhost:8080/serials    # Tester
```

## ⚠️ Important

1. **Toujours exécuter Install.bat en tant qu'administrateur**
2. **Redémarrer Windows après installation/désinstallation**
3. **Avoir au moins une méthode de connexion de secours** (mot de passe, etc.)
4. **Tester d'abord dans une VM Windows**

## 🔐 Sécurité

- En production, utilisez HTTPS pour le serveur
- Protégez le serveur avec une authentification
- Loggez tous les accès
- Ne comptez pas uniquement sur la clé USB pour l'authentification
