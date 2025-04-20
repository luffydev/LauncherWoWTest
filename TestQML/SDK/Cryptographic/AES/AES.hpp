#ifndef __AES_HPP__
#define __AES_HPP__

#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QtCore>

class SecureKey;

#define AES_ENCRYPT_KEY "igO3B76qcSIe9VZ14C9fnihffcvM3k0M"
#define AES_PASSWORD_SALT "aabbccddeeff00112233445566778899"

class AESGCM {
public:
    // Chiffrement AES-GCM standard
    static QByteArray encrypt(const QByteArray& plainText, const SecureKey& key, QByteArray& ivOut, QByteArray& tagOut);

    // Déchiffrement AES-GCM standard
    static QByteArray decrypt(const QByteArray& cipherText, const QByteArray& key, const QByteArray& iv, const QByteArray& tag);

    // Chiffrement avec PBKDF2 pour utilisation avec PHP
    static QByteArray encryptForPHP(const QByteArray& plainText, const QByteArray& password);
};


class PBKDF2 {
public:
    // Dérive une clé à partir d'un mot de passe en utilisant PBKDF2
    static QByteArray deriveKey(const QByteArray& password, const QByteArray& salt,
        int iterations = 10000, int keyLength = 32);

    // Génère un sel aléatoire pour PBKDF2
    static QByteArray generateSalt(int length = 16);
};


class AES {
public:
    // Chiffrement standard
    static QByteArray encrypt(const QByteArray& plainText, const QByteArray& iv, QByteArray key);

    // Fonction spéciale pour chiffrer des données pour une utilisation avec PHP
    // Renvoie une chaîne hexadécimale formatée: salt_hex + iv_hex + encrypted_hex
    static QByteArray encryptForPHP(const QByteArray& plainText, const QByteArray& password, int iterations = 10000);
};

#endif // AES_HPP
