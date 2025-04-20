#ifndef __SECURE_MEMORY__
#define __SECURE_MEMORY__

#include <QByteArray>
#include <openssl/rand.h>
#include <windows.h>
#include <QString>
#include <QDebug>

#include "../Cryptographic/AES/AES.hpp"

class SecureMemory {
public:
    // Alloue un buffer sécurisé qui ne sera pas écrit sur le swap
    static QByteArray allocSecure(int size) {
        QByteArray buffer(size, 0);
        // Verrouiller la mémoire pour éviter le swap sur disque (équivalent Windows de mlock)
        if (!VirtualLock(buffer.data(), buffer.size())) {
            qWarning() << "Impossible de verrouiller la mémoire (VirtualLock):" << GetLastError();
        }
        return buffer;
    }

    // Protège une clé existante
    static void protectKey(QByteArray& key) {
        if (key.isEmpty()) return;
        // Verrouiller la mémoire pour éviter le swap
        if (!VirtualLock(key.data(), key.size())) {
            qWarning() << "Impossible de verrouiller la mémoire pour la clé (VirtualLock):" << GetLastError();
        }
    }

    // Efface et libère une clé de manière sécurisée
    static void releaseKey(QByteArray& key) {
        if (key.isEmpty()) return;
        // Effacement multi-passes pour éviter les résidus en mémoire
        // 1. Remplir avec des bytes aléatoires
        RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), key.size());
        // 2. Remplir avec des 1
        key.fill(0xFF);
        // 3. Remplir avec des 0
        key.fill(0);
        // Déverrouiller la mémoire (équivalent Windows de munlock)
        VirtualUnlock(key.data(), key.size());
        // Réinitialiser le QByteArray
        key.clear();
    }

    // Désactiver les core dumps (équivalent Windows)
    static void disableCoreFiles() {
        // Windows utilise des fichiers "minidump" au lieu des core dumps Unix
        // On peut les désactiver en modifiant le registry ou via API
        SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);

        // Alternative: désactiver en modifiant le registre de Dr. Watson
        // Cette partie peut nécessiter des droits admin et est optionnelle
        // La méthode SetErrorMode ci-dessus est généralement suffisante
    }
};


class PBKDF2;

// Classe RAII pour la gestion sécurisée des clés cryptographiques
class SecureKey {
private:
    QByteArray key;
    bool isProtected;

public:
    // Constructeur avec clé existante
    SecureKey(const QByteArray& existingKey) : isProtected(false) {
        key = existingKey;
        SecureMemory::protectKey(key);
        isProtected = true;
    }

    // Constructeur avec dérivation PBKDF2
    SecureKey(const QByteArray& password, const QByteArray& salt) : isProtected(false) {

        
        // Utiliser PBKDF2 pour dériver la clé
        key = PBKDF2::deriveKey(password, salt);
        SecureMemory::protectKey(key);
        isProtected = true;
    }

    // Destructeur - nettoyage sécurisé de la clé
    ~SecureKey() {
        if (isProtected) {
            SecureMemory::releaseKey(key);
        }
    }

    // Obtenir une référence constante à la clé
    const QByteArray& getKey() const {
        return key;
    }

    // Interdire la copie pour éviter la duplication de la clé en mémoire
    SecureKey(const SecureKey&) = delete;
    SecureKey& operator=(const SecureKey&) = delete;
};

#endif