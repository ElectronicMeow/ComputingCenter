//
// Created on 2021/10/3.
// Algorithm developed in Python by @D33BaT0
// Exported to C by @XWanan
// Refactored by @Reverier-Xu
//

#ifndef MEOW_CRYPTO_UTILS_H
#define MEOW_CRYPTO_UTILS_H

#include <gmp.h>
#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>
#include <utility>
#include <unistd.h>
#include <fcntl.h>

namespace MeowCryptoUtils {

    class PublicParameters : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString N MEMBER m_N READ N WRITE setN)
        Q_PROPERTY(QString g MEMBER m_g READ g WRITE setG)
        Q_PROPERTY(QString k MEMBER m_k READ k WRITE setK)
    private:
        QString m_N;
        QString m_g;
        QString m_k;
    public:
        ~PublicParameters() override = default;

        explicit PublicParameters(QString n = "",
                                  QString k = "",
                                  QString g = "") :
                m_N(std::move(n)),
                m_k(std::move(k)),
                m_g(std::move(g)) { }

        PublicParameters(PublicParameters &parameters) {
            this->m_k = parameters.k();
            this->m_N = parameters.N();
            this->m_g = parameters.g();
        }

        PublicParameters &operator=(const PublicParameters &parameters) {
            this->m_k = parameters.k();
            this->m_N = parameters.N();
            this->m_g = parameters.g();
            return *this;
        }

        [[nodiscard]] QString N() const { return this->m_N; }

        void setN(const QString &n) {
            this->m_N = n;
        }

        [[nodiscard]] QString k() const { return this->m_k; }

        void setK(const QString &n) {
            this->m_k = n;
        }

        [[nodiscard]] QString g() const { return this->m_g; }

        void setG(const QString &n) {
            this->m_g = n;
        }
    };

    class EncryptedPair : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString A MEMBER m_A READ A WRITE setA)
        Q_PROPERTY(QString B MEMBER m_B READ B WRITE setB)
    private:
        QString m_A;
        QString m_B;
        QString m_publicN;
        QString m_publicKey;
    public:
        EncryptedPair(const EncryptedPair &encryptedPair) {
            this->m_A = encryptedPair.A();
            this->m_B = encryptedPair.B();
            this->m_publicN = encryptedPair.publicN();
            this->m_publicKey = encryptedPair.publicKey();
        }

        explicit EncryptedPair(QString a = "",
                               QString b = "",
                               QString n = "",
                               QString pk = "") :
                m_A(std::move(a)),
                m_B(std::move(b)),
                m_publicN(std::move(n)),
                m_publicKey(std::move(pk)) { }

        EncryptedPair &operator=(const EncryptedPair &pair) {
            this->m_A = pair.A();
            this->m_B = pair.B();
            this->m_publicN = pair.publicN();
            this->m_publicKey = pair.publicKey();
            return *this;
        }

        [[nodiscard]] QString A() const { return this->m_A; }

        void setA(const QString &n) {
            this->m_A = n;
        }

        [[nodiscard]] QString B() const { return this->m_B; }

        void setB(const QString &n) {
            this->m_B = n;
        }

        [[nodiscard]] QString publicN() const { return this->m_publicN; }

        void setPublicN(const QString &n) {
            this->m_publicN = n;
        }

        [[nodiscard]] QString publicKey() const { return this->m_publicKey; }

        void setPublicKey(const QString &n) {
            this->m_publicKey = n;
        }

        EncryptedPair operator+(const EncryptedPair &another) const;
    };

    class MasterSecretKeyPair : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString A MEMBER m_A READ A WRITE setA)
        Q_PROPERTY(QString B MEMBER m_B READ B WRITE setB)
    private:
        QString m_A;
        QString m_B;
    public:
        MasterSecretKeyPair(const MasterSecretKeyPair &SecretKeyPair) {
            this->m_A = SecretKeyPair.A();
            this->m_B = SecretKeyPair.B();
        }

        explicit MasterSecretKeyPair(QString a = "", QString b = "") : m_A(std::move(a)), m_B(std::move(b)) { }

        MasterSecretKeyPair &operator=(const MasterSecretKeyPair &pair) {
            this->m_A = pair.A();
            this->m_B = pair.B();
            return *this;
        }

        [[nodiscard]] QString A() const { return this->m_A; }

        void setA(const QString &n) {
            this->m_A = n;
        }

        [[nodiscard]] QString B() const { return this->m_B; }

        void setB(const QString &n) {
            this->m_B = n;
        }
    };

    class KeyPair : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString publicKey MEMBER m_publicKey READ publicKey WRITE setPublicKey)
        Q_PROPERTY(QString secretKey MEMBER m_secretKey READ secretKey WRITE setSecretKey)
    private:
        QString m_publicKey;
        QString m_secretKey;
    public:
        KeyPair(const KeyPair &KeyPair) {
            this->m_publicKey = KeyPair.publicKey();
            this->m_secretKey = KeyPair.secretKey();
        }

        explicit KeyPair(QString pk = "", QString sk = "") : m_publicKey(std::move(pk)), m_secretKey(std::move(sk)) { }

        KeyPair &operator=(const KeyPair &pair) {
            this->m_publicKey = pair.publicKey();
            this->m_secretKey = pair.secretKey();
            return *this;
        }

        [[nodiscard]] QString publicKey() const { return this->m_publicKey; }

        void setPublicKey(const QString &pk) {
            this->m_publicKey = pk;
        }

        [[nodiscard]] QString secretKey() const { return this->m_secretKey; }

        void setSecretKey(const QString &sk) {
            this->m_secretKey = sk;
        }
    };

    /* === GENERAL === */

    [[nodiscard]] EncryptedPair encrypt(const PublicParameters &param,
                                        const QString &publicKey,
                                        const QString &plaintext);

    [[nodiscard]] QString decrypt(const PublicParameters &param,
                                  const QString &secretKey,
                                  const EncryptedPair &ciphertext);

    [[nodiscard]] QString getProdKey(const PublicParameters &param, const QStringList &pks);

    /* === CLIENT === */

    [[nodiscard]] KeyPair keyGen(const PublicParameters &param);

    /* === MASTER === */

    [[nodiscard]] bool keyGenMaster(const QString &specP, const QString &specQ, PublicParameters &param,
                                    MasterSecretKeyPair &secretKey);

    [[nodiscard]] QString masterUniqueDecrypt(const PublicParameters &param,
                                              const MasterSecretKeyPair &secretKey, const EncryptedPair &ciphertext);

    [[nodiscard]] EncryptedPair masterTransform(const PublicParameters &param,
                                                const MasterSecretKeyPair &secretKey,
                                                const QString &destPublicKey,
                                                const EncryptedPair &srcCiphertext);

    void specGen(QString &specP, QString &specQ);

    /* === COMPUTING CENTER === */

    [[nodiscard]] bool ccTransform(const PublicParameters &param,
                                   const EncryptedPair &srcCiphertext,
                                   EncryptedPair &destCiphertext,
                                   QString &destRandSnip);

    [[nodiscard]] EncryptedPair ccComputing(const PublicParameters &param,
                                            const QString &prod_pk,
                                            const QMap<QString, EncryptedPair> &enc_map,
                                            const QMap<QString, QString> &r);
}


#endif //MEOW_CRYPTO_UTILS_H
