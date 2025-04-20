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
    // Alloue un buffer s�curis� qui ne sera pas �crit sur le swap
    static QByteArray allocSecure(int size) {
        QByteArray buffer(size, 0);
        // Verrouiller la m�moire pour �viter le swap sur disque (�quivalent Windows de mlock)
        if (!VirtualLock(buffer.data(), buffer.size())) {
            qWarning() << "Impossible de verrouiller la m�moire (VirtualLock):" << GetLastError();
        }
        return buffer;
    }

    // Prot�ge une cl� existante
    static void protectKey(QByteArray& key) {
        if (key.isEmpty()) return;
        // Verrouiller la m�moire pour �viter le swap
        if (!VirtualLock(key.data(), key.size())) {
            qWarning() << "Impossible de verrouiller la m�moire pour la cl� (VirtualLock):" << GetLastError();
        }
    }

    // Efface et lib�re une cl� de mani�re s�curis�e
    static void releaseKey(QByteArray& key) {
        if (key.isEmpty()) return;
        // Effacement multi-passes pour �viter les r�sidus en m�moire
        // 1. Remplir avec des bytes al�atoires
        RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), key.size());
        // 2. Remplir avec des 1
        key.fill(0xFF);
        // 3. Remplir avec des 0
        key.fill(0);
        // D�verrouiller la m�moire (�quivalent Windows de munlock)
        VirtualUnlock(key.data(), key.size());
        // R�initialiser le QByteArray
        key.clear();
    }

    // D�sactiver les core dumps (�quivalent Windows)
    static void disableCoreFiles() {
        // Windows utilise des fichiers "minidump" au lieu des core dumps Unix
        // On peut les d�sactiver en modifiant le registry ou via API
        SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);

        // Alternative: d�sactiver en modifiant le registre de Dr. Watson
        // Cette partie peut n�cessiter des droits admin et est optionnelle
        // La m�thode SetErrorMode ci-dessus est g�n�ralement suffisante
    }
};


class PBKDF2;

// Classe RAII pour la gestion s�curis�e des cl�s cryptographiques
class SecureKey {
private:
    QByteArray key;
    bool isProtected;

public:
    // Constructeur avec cl� existante
    SecureKey(const QByteArray& existingKey) : isProtected(false) {
        key = existingKey;
        SecureMemory::protectKey(key);
        isProtected = true;
    }

    // Constructeur avec d�rivation PBKDF2
    SecureKey(const QByteArray& password, const QByteArray& salt) : isProtected(false) {

        
        // Utiliser PBKDF2 pour d�river la cl�
        key = PBKDF2::deriveKey(password, salt);
        SecureMemory::protectKey(key);
        isProtected = true;
    }

    // Destructeur - nettoyage s�curis� de la cl�
    ~SecureKey() {
        if (isProtected) {
            SecureMemory::releaseKey(key);
        }
    }

    // Obtenir une r�f�rence constante � la cl�
    const QByteArray& getKey() const {
        return key;
    }

    // Interdire la copie pour �viter la duplication de la cl� en m�moire
    SecureKey(const SecureKey&) = delete;
    SecureKey& operator=(const SecureKey&) = delete;
};

#endif