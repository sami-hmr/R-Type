# Architecture Réseau TCP/UDP pour le Moteur de Jeu

## Vue d'ensemble

Ce document explique l'architecture réseau hybride TCP/UDP pour un moteur de jeu C++ capable de gérer des jeux type R-Type (shoot'em up multijoueur).

---

## Pourquoi TCP ET UDP ?

### Le dilemme du réseau en jeu vidéo

Les jeux en temps réel ont deux besoins contradictoires :

1. **Fiabilité** : Certaines informations sont critiques et ne peuvent pas être perdues
2. **Vitesse** : D'autres informations doivent arriver le plus vite possible, même au prix de pertes occasionnelles

Aucun protocole seul ne peut satisfaire ces deux besoins simultanément. D'où l'approche hybride.

---

## TCP : Pour les données critiques

### Qu'est-ce que le TCP ?

**Transmission Control Protocol**
Un protocole orienté connexion qui garantit :
- La livraison de tous les paquets
- L'ordre d'arrivée des paquets
- La détection et correction des erreurs

### Quand utiliser le TCP ?

Nous utiliserons le TCP pour toute donnée dont la perte causerait une **désynchronisation du game state** :

#### ✅ Événements à envoyer en TCP

- **Suppression d'une entité**

### Pourquoi ces événements seraient-ils gérés en TCP ?

**Exemple concret (R-Type) :**

Si un joueur collecte un power-up laser et que le paquet UDP se perd :
- ❌ Le serveur pense que le joueur n'a pas le laser
- ❌ Le client pense qu'il l'a
- ❌ Résultat : désynchronisation totale

Avec TCP, la collecte est **garantie** d'arriver, même si cela prend 20ms de plus.

### Inconvénients du TCP

- **Latence variable** : Si un paquet est perdu, TCP attend la retransmission (head-of-line blocking)
- **Overhead** : Headers plus lourds, mécanismes d'ACK
- Pas adapté pour les données qui deviennent obsolètes rapidement

---

## UDP : Pour les données temps réel

### Qu'est-ce que l'UDP ?

**User Datagram Protocol** Un protocole sans connexion qui offre :
- Envoi rapide sans confirmation
- Pas de garantie de livraison
- Pas de garantie d'ordre
- Headers minimalistes

### Quand utiliser l'UDP ?

Nous utiliserons l'UDP pour les données **fréquentes et tolérantes aux pertes** :

#### ✅ Événements à envoyer en UDP

- **Position et mouvement**
  - Position du joueur (envoyée 20-60 fois/sec)
  - Vélocité
  - Rotation / orientation

- **Animations et visuels**
  - État d'animation
  - Effets de particules
  - Trails visuels

- **Projectiles non-critiques**
  - Balles standard (sauf si elles font des dégâts importants)
  - Projectiles décoratifs

- **Audio**
  - Déclenchement de sons d'ambiance
  - Effets sonores répétitifs

- **Input en temps réel**
  - Pression des touches (si envoyé à haute fréquence)

### Pourquoi ces événements seraient-ils gérés en UDP ?

**Exemple concret (R-Type) :**

Le vaisseau du joueur envoie sa position 30 fois par seconde.

Si 1 paquet sur 30 se perd :
- ✅ Ce n'est pas grave, le suivant arrive 33ms après
- ✅ Le client peut interpoler la position manquante
- ✅ La position d'il y a 33ms est de toute façon obsolète

Attendre une retransmission TCP serait **pire** car la position reconstruite serait déjà périmée.

### Techniques de compensation UDP

Pour gérer les pertes de paquets UDP :

1. **Client-side prediction** : Le client prédit son propre mouvement
2. **Interpolation** : Lissage entre les positions reçues
3. **Redundancy** : Envoyer les infos critiques plusieurs fois (ex: 3 frames consécutives)
4. **Sequence numbers** : Détecter les paquets perdus ou en désordre

---

## Protocole Binaire vs Textuel

### Pourquoi un protocole binaire ?

Pour un moteur de jeu comme R-Type, un protocole binaire est **fortement recommandé**.

#### Comparaison Textuel (JSON/XML) vs Binaire

**Exemple : Position du joueur**

```json
// Textuel (JSON) - 59 bytes
{"type":"pos","id":42,"x":125.5,"y":340.2,"r":1.57}
```

```cpp
// Binaire - 17 bytes - exemple en CPP
struct PlayerPos {
    uint8_t  type;    // 1 byte : type de message
    uint32_t id;      // 4 bytes : ID joueur
    float    x;       // 4 bytes
    float    y;       // 4 bytes
    float    r;       // 4 bytes (rotation)
};
```

#### Avantages du protocole binaire

1. **Bande passante réduite**
   - Critique quand on envoie 30-60 updates/sec par joueur
   - Pour 10 joueurs : économie de ~4 KB/sec par joueur

2. **Parsing ultra-rapide**
   - Pas de parsing de texte
   - Simple `memcpy` ou cast de structure
   - Pas d'allocation mémoire dynamique

3. **Latence minimale**
   - Moins de données = transmission plus rapide
   - Parsing instantané = traitement plus rapide

4. **Prédictibilité**
   - Taille fixe des messages = buffer management simple
   - Performance constante, pas de surprises

#### Inconvénients du protocole binaire

- ❌ Moins lisible pour le debugging (mais on utilise des outils comme Wireshark)
- ❌ Nécessite une documentation stricte du protocole
- ❌ Problèmes d'endianness (big-endian vs little-endian) sur différentes architectures
- ❌ Versioning plus complexe

#### Solutions aux inconvénients

```cpp
// 1. Logger en hexadécimal pour debug
void LogPacket(const uint8_t* data, size_t len) {
    for(size_t i = 0; i < len; ++i)
        printf("%02X ", data[i]);
}

// 2. Gérer l'endianness
uint32_t htonl_custom(uint32_t hostlong);
uint32_t ntohl_custom(uint32_t netlong);

// 3. Versioning dans le header
struct PacketHeader {
    uint8_t protocol_version;  // Permet d'évoluer
    uint8_t message_type;
    uint16_t payload_size;
};
```

---

## Architecture

### Structure des paquets

```cpp
// Header commun (TCP et UDP)
struct PacketHeader {
    uint8_t  protocol_version;  // 1 byte
    uint8_t  message_type;      // 1 byte
    uint16_t payload_size;      // 2 bytes
    uint32_t sequence_number;   // 4 bytes (important pour UDP)
};

// Exemple de message TCP : Collecte d'item
struct ItemCollectedMessage {
    PacketHeader header;
    uint32_t player_id;
    uint32_t item_id;
    uint8_t  item_type;
    uint32_t timestamp;
};

// Exemple de message UDP : Position
struct PlayerPositionMessage {
    PacketHeader header;
    uint32_t player_id;
    float    x, y;
    float    velocity_x, velocity_y;
    float    rotation;
    uint16_t animation_state;
};
```

### Gestion des deux sockets

```cpp
class NetworkManager {
private:
    int tcp_socket;
    int udp_socket;
    
public:
    // Envoyer des données critiques
    void SendReliable(const void* data, size_t len) {
        send(tcp_socket, data, len, 0);
    }
    
    // Envoyer des données temps réel
    void SendUnreliable(const void* data, size_t len) {
        sendto(udp_socket, data, len, 0, &server_addr, sizeof(server_addr));
    }
    
    // Boucle de réception (threads séparés)
    void ReceiveTCP();  // Thread 1
    void ReceiveUDP();  // Thread 2
};
```

---

## Exemple pratique : R-Type

### Scénario de jeu

1. **Connexion du joueur** (TCP)
   - Authentification
   - Réception de l'état initial du monde

2. **Gameplay** (Mixte)
   - **UDP** : Position du vaisseau envoyée 30 fois/sec
   - **UDP** : Tir de balles simples
   - **TCP** : Collecte de power-up laser
   - **TCP** : Destruction d'un boss (gain de points)
   - **UDP** : Animation de l'explosion

3. **Mort du joueur** (TCP)
   - Notification de mort
   - Mise à jour du score final
   - Préparation du respawn

### Timeline réseau typique

```
T=0ms    : [UDP] Position joueur 1 (x:100, y:200)
T=16ms   : [UDP] Position joueur 1 (x:105, y:200)
T=33ms   : [UDP] Position joueur 1 (x:110, y:200)
T=42ms   : [TCP] Joueur 1 collecte power-up ID:5
T=50ms   : [UDP] Position joueur 1 (x:115, y:200)
T=67ms   : [UDP] Position joueur 1 (x:120, y:200)
T=100ms  : [TCP] Boss détruit, +1000 points
```

---

## Conclusion

### Règle d'or

**Si la perte du message casse le jeu → TCP**  
**Si le message devient obsolète en <100ms → UDP**

### Check-list de décision

Pour chaque type de message :

1. ❓ Que se passe-t-il si ce message se perd ?
   - Désynchronisation grave → **TCP**
   - Légère saccade visuelle → **UDP**

2. ❓ Ce message sera-t-il remplacé rapidement par un nouveau ?
   - Oui (position update) → **UDP**
   - Non (mort du joueur) → **TCP**

3. ❓ L'ordre d'arrivée est-il critique ?
   - Oui (séquence d'actions) → **TCP**
   - Non (positions indépendantes) → **UDP**

### Protocole binaire : meilleur pour les jeux en temps réel

- ✅ Performance maximale
- ✅ Bande passante minimale
- ✅ Latence réduite
- ✅ Prédictibilité

L'overhead du parsing JSON/XML est inacceptable pour un jeu à 60 FPS envoyant des dizaines de messages par seconde.
