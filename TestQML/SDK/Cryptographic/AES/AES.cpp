#include "../../Memory/SecureMemory.hpp"

#include "AES.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

// Désactiver les core dumps au chargement de la bibliothèque
namespace {
    class CoreDumpDisabler {
    public:
        CoreDumpDisabler() {
            SecureMemory::disableCoreFiles();
        }
    };

    // Objet statique pour exécuter le constructeur au chargement
    static CoreDumpDisabler disabler;
}

QByteArray AESGCM::encrypt(const QByteArray& plainText, const SecureKey& secureKey, QByteArray& ivOut, QByteArray& tagOut)
{
    const QByteArray& key = secureKey.getKey();

    // Vérification de la taille de la clé
    if (key.size() != 32) {
        qWarning() << "La clé doit faire exactement 32 octets (256 bits)";
        return QByteArray();
    }

    // Génération d'un IV aléatoire de 12 octets (96 bits) recommandé pour GCM
    ivOut = SecureMemory::allocSecure(12);
    RAND_bytes(reinterpret_cast<unsigned char*>(ivOut.data()), ivOut.size());

    // Buffer pour les données chiffrées
    QByteArray cipherText;
    cipherText.resize(plainText.size() + EVP_MAX_BLOCK_LENGTH);

    // Tag d'authentification (16 octets / 128 bits)
    tagOut = SecureMemory::allocSecure(16);

    // Contexte OpenSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Erreur lors de la création du contexte OpenSSL";
        return QByteArray();
    }

    int outLen = 0;
    int tmpLen = 0;

    // Initialisation du chiffrement
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
        reinterpret_cast<const unsigned char*>(key.constData()),
        reinterpret_cast<const unsigned char*>(ivOut.constData())) != 1) {
        qWarning() << "Erreur lors de l'initialisation du chiffrement";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    // Chiffrement des données
    if (EVP_EncryptUpdate(ctx,
        reinterpret_cast<unsigned char*>(cipherText.data()), &outLen,
        reinterpret_cast<const unsigned char*>(plainText.constData()), plainText.size()) != 1) {
        qWarning() << "Erreur lors du chiffrement";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    // Finalisation du chiffrement
    if (EVP_EncryptFinal_ex(ctx,
        reinterpret_cast<unsigned char*>(cipherText.data()) + outLen, &tmpLen) != 1) {
        qWarning() << "Erreur lors de la finalisation du chiffrement";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    outLen += tmpLen;

    // Récupération du tag d'authentification
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tagOut.size(),
        reinterpret_cast<unsigned char*>(tagOut.data())) != 1) {
        qWarning() << "Erreur lors de la récupération du tag d'authentification";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    EVP_CIPHER_CTX_free(ctx);
    cipherText.resize(outLen);
    return cipherText;
}

// Fonction pour chiffrer avec PBKDF2 et AES-GCM pour PHP
QByteArray AESGCM::encryptForPHP(const QByteArray& plainText, const QByteArray& password)
{
    // Générer un sel aléatoire
    QByteArray salt = SecureMemory::allocSecure(16);
    RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), salt.size());

    // Scope limité pour la clé
    {
        // Dériver la clé avec PBKDF2 et la stocker de manière sécurisée
        SecureKey secureKey(password, salt);

        // IV et tag pour GCM
        QByteArray iv, tag;

        // Chiffrer les données
        QByteArray encryptedData = encrypt(plainText, secureKey.getKey(), iv, tag);

        // Format: salt_hex + iv_hex + tag_hex + encrypted_hex
        QByteArray result = salt.toHex() + iv.toHex() + tag.toHex() + encryptedData.toHex();

        // Nettoyer les données sensibles
        SecureMemory::releaseKey(iv);
        SecureMemory::releaseKey(tag);

        return result;
    } // secureKey est automatiquement nettoyée ici grâce au destructeur RAII
}

QByteArray PBKDF2::deriveKey(const QByteArray& password, const QByteArray& salt,
    int iterations, int keyLength)
{
    QByteArray derivedKey;
    derivedKey.resize(keyLength);

    PKCS5_PBKDF2_HMAC(
        password.constData(),
        password.size(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.size(),
        iterations,
        EVP_sha256(),
        keyLength,
        reinterpret_cast<unsigned char*>(derivedKey.data())
    );

    return derivedKey;
}

QByteArray PBKDF2::generateSalt(int length)
{
    QByteArray salt(length, 0);
    RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), length);
    return salt;
}

// Fonction pour chiffrer et générer une sortie formatée pour PHP
QByteArray AES::encryptForPHP(const QByteArray& plainText, const QByteArray& password, int iterations)
{
    // Générer un sel aléatoire
    QByteArray salt = PBKDF2::generateSalt(16);

    // Dériver la clé à partir du mot de passe et du sel
    QByteArray key = PBKDF2::deriveKey(password, salt, iterations);

    // Générer un IV aléatoire
    QByteArray iv(16, 0);
    RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size());

    // Chiffrer les données
    QByteArray encryptedData = encrypt(plainText, iv, key);

    // Format: salt_hex + iv_hex + encrypted_hex
    QByteArray result = salt.toHex() + iv.toHex() + encryptedData.toHex();

    return result;
}

QByteArray AES::encrypt(const QByteArray& plainText, const QByteArray& iv, QByteArray key)
{
    QByteArray encrypted;
    encrypted.resize(plainText.size() + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int outLen1 = 0, outLen2 = 0;

    if (!ctx)
        return QByteArray();

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
        reinterpret_cast<const unsigned char*>(key.constData()),
        reinterpret_cast<const unsigned char*>(iv.constData())))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (!EVP_EncryptUpdate(ctx,
        reinterpret_cast<unsigned char*>(encrypted.data()), &outLen1,
        reinterpret_cast<const unsigned char*>(plainText.constData()), plainText.size()))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (!EVP_EncryptFinal_ex(ctx,
        reinterpret_cast<unsigned char*>(encrypted.data()) + outLen1, &outLen2))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(outLen1 + outLen2);
    return encrypted;
}