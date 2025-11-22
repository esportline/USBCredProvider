# USB Credential Provider - Résumé du Projet

## 📋 Vue d'ensemble

Credential Provider Windows complet permettant l'authentification par clé USB avec :
- Configuration HTTP avec fallback registre
- Détection automatique des périphériques USB
- Scripts d'installation automatisés
- Outil de diagnostic inclus

## 📁 Structure du projet

```
USBCredProvider/
│
├── src/                                 # Code source principal
│   ├── USBCredentialProvider.cpp/h     # Provider COM (ICredentialProvider)
│   ├── USBCredential.cpp/h             # Credential individuelle
│   ├── usb_detector.cpp/h              # Détection USB avec SetupAPI
│   ├── config.cpp/h                    # Configuration HTTP + registre
│   ├── dll.cpp/h                       # DLL entry point et factory
│   ├── helpers.cpp/h                   # Fonctions utilitaires Windows
│   ├── common.h                        # Champs UI et descripteurs
│   ├── guid.cpp/h                      # GUID du provider
│   ├── resource.h                      # Ressources
│   └── USBCredProvider.def             # Exports DLL
│
├── tools/
│   └── list_usb_devices.cpp            # Outil CLI pour lister les USB
│
├── build/                               # Généré par compilation
│   ├── bin/
│   │   ├── USBCredProvider.dll         # Le credential provider
│   │   └── tools/
│   │       └── list_usb_devices.exe    # Outil de diagnostic
│   └── obj/                            # Fichiers objets
│
├── Installation Windows
│   ├── Install.bat                     # Installation automatique
│   ├── Uninstall.bat                   # Désinstallation
│   ├── Register.reg                    # Enregistrement registre
│   └── Unregister.reg                  # Désenregistrement
│
├── Configuration serveur
│   └── server_example.py               # Serveur HTTP d'exemple
│
├── Compilation
│   ├── Makefile                        # Compilation Make (Linux/Windows)
│   ├── CMakeLists.txt                  # Compilation CMake
│   ├── mingw-toolchain.cmake           # Toolchain cross-compilation
│   └── build.sh                        # Script de build interactif
│
└── Documentation
    ├── README.md                        # Documentation complète
    ├── QUICK_START.md                   # Guide de démarrage rapide
    └── PROJECT_SUMMARY.md               # Ce fichier
```

## 🔧 Technologies utilisées

### Langage et compilateur
- **C++20** : Standard moderne
- **MinGW-w64** : Cross-compilation Windows depuis Linux
- **MSVC** : Support compilation native Windows (optionnel)

### APIs Windows
- **Credential Provider API** : Interface d'authentification Windows
- **SetupAPI** : Énumération et détection des périphériques USB
- **Configuration Manager (cfgmgr32)** : Gestion des périphériques
- **WinHTTP** : Communication HTTP pour configuration
- **Registry API (advapi32)** : Stockage de configuration
- **COM/OLE** : Interfaces COM pour le provider

### Outils de build
- **Make** : Compilation rapide et légère
- **CMake** : Support IDE (CLion, VS Code, Visual Studio)
- **Bash scripts** : Automatisation Linux

## 🎯 Flux de fonctionnement

### 1. Chargement du provider
```
Windows Logon
    ↓
Charge USBCredProvider.dll
    ↓
DllGetClassObject() → CClassFactory
    ↓
CreateInstance() → CUSBCredentialProvider
```

### 2. Énumération des credentials
```
SetUsageScenario(CPUS_LOGON)
    ↓
GetCredentialCount()
    ↓
_EnumerateCredentials()
    ↓
Crée CUSBCredential
```

### 3. Vérification USB à la sélection
```
SetSelected()
    ↓
CheckUSBPresence()
    ↓
ConfigManager::LoadAuthorizedSerials()
    ├─→ [HTTP] GET serveur/serials → Parse réponse
    └─→ [FALLBACK] Lit registre AuthorizedSerials
    ↓
USBDetector::GetConnectedUSBDevices()
    ↓
Compare serials
    ↓
Retourne OK ou FAIL
```

### 4. Authentification
```
GetSerialization()
    ↓
CheckUSBPresence() encore
    ↓
Si OK: KerbInteractiveUnlockLogonPack()
    ↓
Retourne credentials à Windows
    ↓
Windows authentifie l'utilisateur
```

## 🚀 Commandes principales

### Compilation (Linux)
```bash
make                    # Compiler tout
make clean              # Nettoyer
make rebuild            # Recompiler
make check-mingw        # Vérifier MinGW
./build.sh              # Script interactif
```

### Installation (Windows - Admin requis)
```cmd
list_usb_devices.exe    # 1. Obtenir serials USB
Install.bat             # 2. Installer le provider
shutdown /r /t 0        # 3. Redémarrer
```

### Serveur de configuration
```bash
# Éditer AUTHORIZED_SERIALS dans server_example.py
python server_example.py 8080
```

### Test et debug
```cmd
# Event Viewer pour logs
eventvwr.msc

# Vérifier enregistrement
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{5fd3d285-0dd9-4362-8855-e0abaacd4af6}"

# Vérifier config
reg query "HKLM\SOFTWARE\USBCredProvider"
```

## 📊 Diagramme de classes

```
┌─────────────────────────────────┐
│   ICredentialProvider           │
└────────────┬────────────────────┘
             │ implements
             ▼
┌─────────────────────────────────┐
│  CUSBCredentialProvider         │
├─────────────────────────────────┤
│ - _pCredential                  │
│ - _cpus                         │
├─────────────────────────────────┤
│ + SetUsageScenario()            │
│ + GetCredentialCount()          │
│ + GetCredentialAt()             │
│ - _EnumerateCredentials()       │
└────────────┬────────────────────┘
             │ creates
             ▼
┌─────────────────────────────────┐
│ ICredentialProviderCredential   │
└────────────┬────────────────────┘
             │ implements
             ▼
┌─────────────────────────────────┐
│  CUSBCredential                 │
├─────────────────────────────────┤
│ - _rgFieldStrings[]             │
│ - _pwzUsername                  │
│ - _cpus                         │
├─────────────────────────────────┤
│ + SetSelected()                 │
│ + GetSerialization()            │
│ + GetFieldState()               │
│ - CheckUSBPresence()            │
└────────────┬────────────────────┘
             │ uses
             ▼
┌──────────────────┐    ┌──────────────────┐
│  USBDetector     │    │  ConfigManager   │
├──────────────────┤    ├──────────────────┤
│ + GetConnected   │    │ + LoadAuthorized │
│   USBDevices()   │    │   Serials()      │
│ + IsSerialPresent│    │ + IsSerialAuth() │
└──────────────────┘    └──────────────────┘
```

## 🔐 Configuration du registre

### Clés créées
```
HKEY_LOCAL_MACHINE\
├── SOFTWARE\
│   ├── Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\
│   │   └── {5fd3d285-0dd9-4362-8855-e0abaacd4af6}\
│   │       └── (Default) = "USBCredentialProvider"
│   │
│   └── USBCredProvider\
│       ├── ServerUrl (REG_SZ) = "http://serveur:8080/serials"
│       └── AuthorizedSerials (REG_MULTI_SZ) = ["SERIAL1", "SERIAL2", ...]
│
└── CLASSES_ROOT\
    └── CLSID\
        └── {5fd3d285-0dd9-4362-8855-e0abaacd4af6}\
            ├── (Default) = "USBCredentialProvider"
            └── InprocServer32\
                ├── (Default) = "C:\Program Files\USBCredProvider\USBCredProvider.dll"
                └── ThreadingModel = "Apartment"
```

## 🧪 Tests recommandés

### 1. Test de compilation
```bash
make clean && make
# Vérifier : build/bin/USBCredProvider.dll existe
```

### 2. Test de détection USB
```bash
# Sur Windows
list_usb_devices.exe
# Vérifier : Les serials s'affichent
```

### 3. Test du serveur
```bash
python server_example.py 8080
curl http://localhost:8080/serials
# Vérifier : Retourne la liste des serials
```

### 4. Test du provider (dans VM Windows)
1. Installer avec Install.bat
2. Redémarrer
3. Vérifier présence sur écran de connexion
4. Brancher clé USB autorisée
5. Vérifier message "Clé USB détectée"
6. Tenter connexion

## 📝 Checklist avant production

- [ ] Compiler en mode Release (`-O2` ou `/O2`)
- [ ] Tester dans une VM Windows propre
- [ ] Configurer HTTPS pour le serveur (pas HTTP)
- [ ] Implémenter authentification serveur
- [ ] Activer les logs Windows (Event Viewer)
- [ ] Documenter les serials autorisés
- [ ] Prévoir méthode de connexion de secours
- [ ] Tester déconnexion/reconnexion USB
- [ ] Tester avec plusieurs clés USB
- [ ] Vérifier les permissions du registre

## 🐛 Points d'attention

### Sécurité
- ⚠️ HTTP non chiffré = serials visibles sur le réseau
- ⚠️ Pas d'authentification serveur = n'importe qui peut requêter
- ⚠️ Serials en clair dans le registre
- ⚠️ Pas de rate limiting sur les tentatives

### Performance
- USBDetector scanne tous les périphériques à chaque SetSelected()
- ConfigManager recharge depuis HTTP à chaque LoadAuthorizedSerials()
- Pas de cache des devices USB

### Compatibilité
- Windows 7+ requis pour Credential Provider API
- Nécessite privilèges admin pour installation
- Certains USB peuvent ne pas exposer de serial

## 📚 Ressources utiles

- [Microsoft Credential Provider Documentation](https://docs.microsoft.com/en-us/windows/win32/secauthn/credential-providers-in-windows)
- [SetupAPI Reference](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/setupapi)
- [MinGW-w64 Project](https://www.mingw-w64.org/)

## 🤝 Support

Pour les problèmes :
1. Consulter [README.md](README.md) et [QUICK_START.md](QUICK_START.md)
2. Vérifier Event Viewer Windows
3. Utiliser `list_usb_devices.exe` pour diagnostic USB
4. Vérifier les clés de registre

## ✅ Statut du projet

- [x] Détection USB fonctionnelle
- [x] Configuration HTTP avec fallback
- [x] Provider COM complet
- [x] UI simple et claire
- [x] Scripts d'installation
- [x] Outil de diagnostic
- [x] Documentation complète
- [x] Support Make et CMake
- [x] Cross-compilation Linux → Windows

**Le projet est prêt pour la compilation et les tests !** 🎉
