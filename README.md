# USB Credential Provider pour Windows

Credential Provider Windows permettant l'authentification par clé USB basée sur le numéro de série du périphérique.

## Fonctionnalités

- **Authentification par clé USB** : Connexion Windows basée sur la détection de clés USB autorisées
- **Configuration via serveur HTTP** : Liste des numéros de série autorisés récupérée depuis un serveur HTTP
- **Fallback sur registre** : En cas d'indisponibilité du serveur, utilise les numéros de série stockés localement
- **Détection automatique** : Scan des périphériques USB connectés et vérification des numéros de série

## Compilation rapide

```bash
# Linux
make

# Ou avec le script interactif
./build.sh
```

## Installation

Voir [QUICK_START.md](QUICK_START.md) pour un guide complet.

## Documentation

- [QUICK_START.md](QUICK_START.md) - Guide de démarrage rapide
- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - Vue d'ensemble technique

## Sécurité

⚠️ Ce projet est fourni à des fins éducatives. En production:
- Utilisez HTTPS pour le serveur
- Implémentez une authentification serveur
- Activez les logs
- Ne comptez pas uniquement sur la clé USB
