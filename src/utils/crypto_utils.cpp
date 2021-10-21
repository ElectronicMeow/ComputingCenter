//
// Created on 2021/10/3.
// Algorithm developed in Python by @D33BaT0
// Exported to C by @XWanan
// Refactored by @Reverier-Xu
//

#include "crypto_utils.h"

#include <QMap>
#include <gmp.h>
#include <unistd.h>
#include <fcntl.h>

#define DEFAULT_MASTER_P_ "16D1F12C3F7A38F318C23A0A75491C89" \
                          "629FADF2005B217ADAC7CEFEE96FD2EF" \
                          "40BD70D1E17AB49F547AE04FADF4C64C" \
                          "DBAADA32FD090B98E9C87532DF55A4B7" \
                          "6298725E35AE5E73AF6E7FDA4218E906" \
                          "8ACE591F1E76E63B0CBC46AF434C9DC0" \
                          "87F7E4E61CAE1466E6A3AB478AEF8105" \
                          "8DA6965F21F09CE9B9FC9F008EB8D247"

#define DEFAULT_MASTER_Q_ "39E8F5DDEC7A52EF02AA5E7EC66D351F" \
                          "3A8F747E0526B25966B3CB253544C85A" \
                          "44A32AC4F07AF7389FA226C73BE96623" \
                          "71F2F92AA5318805502F3BCF4307E39A" \
                          "02623E940AD935D391730AA3089FF347" \
                          "33F5C1C0A62B8284DD84AA25B52C49CB" \
                          "3931D52C9D7D99C9C22F9AE88C231161" \
                          "C02CE45D78EB79CE44A8FE0E8A679CA7"

/** MACRO INIT_RANDOM_GENERATOR()
 * 初始化GMP的随机数生成器，工具片段。调用此宏之后可以使用grt变量获取已经初始化完成的随机数生成器。
 */
#define INIT_RANDOM_GENERATOR() \
        gmp_randstate_t grt; \
        gmp_randinit_default(grt); \
        unsigned int seed; \
        int random_fd = open("/dev/urandom", O_RDONLY); \
        read(random_fd, &seed, sizeof(unsigned int)); \
        close(random_fd); \
        gmp_randseed_ui(grt, seed);

namespace MeowCryptoUtils {
    EncryptedPair encrypt(const PublicParameters &param,
                          const QString &publicKey,
                          const QString &plaintext) {
        INIT_RANDOM_GENERATOR();
        mpz_t N, g, N2, r, A, B, pk, m;
        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init_set_str(g, param.g().toStdString().c_str(), 16);
        mpz_init_set_str(pk, publicKey.toStdString().c_str(), 16);
        mpz_init_set_str(m, plaintext.toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_init(r);
        mpz_init(A);
        mpz_init(B);
        mpz_mul(N2, N, N);
        mpz_urandomm(r, grt, N2);
        mpz_powm(A, g, r, N2);

        mpz_t r_tmp;
        mpz_init(r_tmp);
        mpz_powm(B, pk, r, N2);
        mpz_mul(r_tmp, m, N);
        mpz_add_ui(r_tmp, r_tmp, 1);
        mpz_mul(B, B, r_tmp);
        mpz_mod(B, B, N2);

        mpz_clear(N);
        mpz_clear(g);
        mpz_clear(N2);
        mpz_clear(r);
        mpz_clear(pk);
        mpz_clear(m);
        mpz_clear(r_tmp);

        EncryptedPair res;

        res.setA(mpz_get_str(nullptr, 16, A));
        res.setB(mpz_get_str(nullptr, 16, B));
        res.setPublicN(param.N());
        res.setPublicKey(publicKey);

        mpz_clear(A);
        mpz_clear(B);
        return res;
    }

    QString decrypt(const PublicParameters &param,
                    const QString &secretKey,
                    const EncryptedPair &ciphertext) {
        mpz_t N, N2, up, sk_tmp, sk, W, Z;
        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init_set_str(sk, secretKey.toStdString().c_str(), 16);
        mpz_init_set_str(W, ciphertext.A().toStdString().c_str(), 16);
        mpz_init_set_str(Z, ciphertext.B().toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_init(up);
        mpz_init(sk_tmp);
        mpz_mul(N2, N, N);
        mpz_mul_si(sk_tmp, sk, -1);
        mpz_powm(W, W, sk_tmp, N2);
        mpz_mul(Z, Z, W);
        mpz_sub_ui(Z, Z, 1);
        mpz_mod(up, Z, N2);
        mpz_fdiv_q(up, up, N);

        QString res = mpz_get_str(nullptr, 16, up);

        mpz_clear(N);
        mpz_clear(N2);
        mpz_clear(up);
        mpz_clear(sk_tmp);
        mpz_clear(sk);
        mpz_clear(W);
        mpz_clear(Z);

        return res;
    }

    KeyPair keyGen(const PublicParameters &param) {
        INIT_RANDOM_GENERATOR();
        mpz_t N, g, N2, sk, pk;
        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init_set_str(g, param.g().toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_init(sk);
        mpz_init(pk);
        mpz_mul(N2, N, N);
        mpz_urandomm(sk, grt, N2);
        mpz_powm(pk, g, sk, N2);

        KeyPair res;

        res.setSecretKey(mpz_get_str(nullptr, 16, sk));
        res.setPublicKey(mpz_get_str(nullptr, 16, pk));

        mpz_clear(N);
        mpz_clear(g);
        mpz_clear(N2);
        mpz_clear(sk);
        mpz_clear(pk);
        return res;
    }

    bool keyGenMaster(const QString &specP,
                      const QString &specQ,
                      PublicParameters &param,
                      MasterSecretKeyPair &secretKey) {
        mpz_t p, p_tmp;
        mpz_init(p);
        mpz_init(p_tmp);
        if (!specP.isEmpty())
            mpz_set_str(p, specP.toStdString().c_str(), 16);
        else
            mpz_set_str(p, DEFAULT_MASTER_P_, 16);

        mpz_sub_ui(p_tmp, p, 1);
        mpz_fdiv_q_ui(p_tmp, p_tmp, 2);

        mpz_t q, q_tmp;
        mpz_init(q);
        mpz_init(q_tmp);
        if (!specQ.isEmpty())
            mpz_set_str(q, specQ.toStdString().c_str(), 16);
        else
            mpz_set_str(q, DEFAULT_MASTER_Q_, 16);

        mpz_sub_ui(q_tmp, q, 1);
        mpz_fdiv_q_ui(q_tmp, q_tmp, 2);

        mpz_t N;
        mpz_init(N);
        mpz_mul(N, p, q);

        mpz_t N2;
        mpz_init(N2);
        mpz_mul(N2, N, N);

        mpz_t pt_qt;
        mpz_init(pt_qt);
        mpz_mul(pt_qt, p_tmp, q_tmp);

        mpz_t g, r_tmp, r_mod, r_div, N_tmp;
        mpz_init(g);
        mpz_init(r_tmp);
        mpz_init(r_mod);
        mpz_init(r_div);
        mpz_init(N_tmp);

        INIT_RANDOM_GENERATOR();

        while (true) {
            mpz_urandomm(g, grt, N2);
            mpz_powm(r_tmp, g, pt_qt, N2);
            mpz_sub_ui(r_tmp, r_tmp, 1);
            mpz_mod(r_mod, r_tmp, N);
            mpz_fdiv_q(r_div, r_tmp, N);
            mpz_sub_ui(N_tmp, N, 1);
            if (mpz_cmp_ui(r_mod, 0) == 0 &&
                mpz_cmp_ui(r_div, 1) >= 0 &&
                mpz_cmp(r_div, N_tmp) <= 0) {
                break;
            }
        }
        mpz_clear(r_mod);
        mpz_clear(r_div);
        mpz_clear(N_tmp);
        mpz_clear(pt_qt);

        mpz_t k;
        mpz_init(k);
        mpz_init(r_tmp);
        mpz_mul(r_tmp, p_tmp, q_tmp);
        mpz_powm(r_tmp, g, r_tmp, N2);
        mpz_sub_ui(r_tmp, r_tmp, 1);
        mpz_fdiv_q(k, r_tmp, N);

        // Returns here.
        secretKey.setA(mpz_get_str(nullptr, 16, p_tmp));
        secretKey.setB(mpz_get_str(nullptr, 16, q_tmp));

        param.setN(mpz_get_str(nullptr, 16, N));
        param.setK(mpz_get_str(nullptr, 16, k));
        param.setG(mpz_get_str(nullptr, 16, g));

        mpz_clear(N);
        mpz_clear(k);
        mpz_clear(g);
        mpz_clear(p_tmp);
        mpz_clear(q_tmp);
        mpz_clear(r_tmp);
        mpz_clear(p);
        mpz_clear(q);

        return true;
    }

    QString masterUniqueDecrypt(const PublicParameters &param,
                                const MasterSecretKeyPair &secretKey, const EncryptedPair &ciphertext) {
        // 初始化参数
        mpz_t p_sec, q_sec, N, k, g, h, N2, k_1;

        mpz_t pk, A, B;
        mpz_init_set_str(pk, ciphertext.publicKey().toStdString().c_str(), 16);
        mpz_init_set_str(A, ciphertext.A().toStdString().c_str(), 16);
        mpz_init_set_str(B, ciphertext.B().toStdString().c_str(), 16);

        mpz_init_set_str(p_sec, secretKey.A().toStdString().c_str(), 16);
        mpz_init_set_str(q_sec, secretKey.B().toStdString().c_str(), 16);

        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init_set_str(k, param.k().toStdString().c_str(), 16);
        mpz_init_set_str(g, param.g().toStdString().c_str(), 16);

        mpz_init(N2);
        mpz_mul(N2, N, N);

        mpz_init_set(h, pk);

        mpz_init(k_1);
        mpz_invert(k_1, k, N);

        //计算a, r, delta, gamma
        mpz_t r_tmp, ps_qs, a, r, delta, gamma;
        mpz_init(ps_qs);
        mpz_mul(ps_qs, p_sec, q_sec);

        mpz_init(a);
        mpz_init(r_tmp);
        mpz_powm(r_tmp, h, ps_qs, N2);
        mpz_sub_ui(r_tmp, r_tmp, 1);
        mpz_mod(r_tmp, r_tmp, N2);
        mpz_fdiv_q(r_tmp, r_tmp, N);
        mpz_mul(r_tmp, r_tmp, k_1);
        mpz_mod(a, r_tmp, N);

        mpz_init(r);
        mpz_init(r_tmp);
        mpz_powm(r_tmp, A, ps_qs, N2);
        mpz_sub_ui(r_tmp, r_tmp, 1);
        mpz_mod(r_tmp, r_tmp, N2);
        mpz_fdiv_q(r_tmp, r_tmp, N);
        mpz_mul(r_tmp, r_tmp, k_1);
        mpz_mod(r, r_tmp, N);

        mpz_init(delta);
        mpz_init(gamma);
        mpz_invert(delta, ps_qs, N);

        mpz_init(r_tmp);
        mpz_mul(r_tmp, a, r);
        mpz_mod(gamma, r_tmp, N);

        // 解密消息
        // m = ((((pow(secretKey * invert(powmod(g,gamma,N2),N2),p_sec * q_sec, N2) - 1 ) %N2) // N) * delta )%N
        mpz_t m;
        mpz_init(r_tmp);
        mpz_init(m);
        mpz_powm(r_tmp, g, gamma, N2);
        mpz_invert(r_tmp, r_tmp, N2);
        mpz_mul(r_tmp, r_tmp, B);
        mpz_powm(r_tmp, r_tmp, ps_qs, N2);
        mpz_sub_ui(r_tmp, r_tmp, 1);
        mpz_mod(r_tmp, r_tmp, N2);
        mpz_fdiv_q(r_tmp, r_tmp, N);
        mpz_mul(r_tmp, r_tmp, delta);
        mpz_mod(m, r_tmp, N2);

        //    gmp_printf("message= %Zd\n\n", m);
        QString res = mpz_get_str(nullptr, 16, m);

        mpz_clear(p_sec);
        mpz_clear(q_sec);
        mpz_clear(N);
        mpz_clear(k);
        mpz_clear(g);
        mpz_clear(h);
        mpz_clear(N2);
        mpz_clear(k_1);
        mpz_clear(ps_qs);
        mpz_clear(a);
        mpz_clear(r);
        mpz_clear(delta);
        mpz_clear(gamma);
        mpz_clear(r_tmp);
        mpz_clear(m);
        mpz_clear(A);
        mpz_clear(B);
        mpz_clear(pk);
        return res;
    }

    QString getProdKey(const PublicParameters &param, const QStringList &pks) {
        mpz_t N, N2, prod_pk_mpz;
        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_mul(N2, N, N);
        mpz_init(prod_pk_mpz);
        mpz_add_ui(prod_pk_mpz, prod_pk_mpz, 1);
        mpz_t temp_pk;
        mpz_init(temp_pk);
        for (int i = 0; i < pks.length(); ++i) {
            mpz_set_str(temp_pk, pks[i].toStdString().c_str(), 16);
            mpz_mul(prod_pk_mpz, prod_pk_mpz, temp_pk);
            mpz_mod(prod_pk_mpz, prod_pk_mpz, N2);
        }
        mpz_clear(N);
        mpz_clear(N2);
        mpz_clear(temp_pk);
        QString res = mpz_get_str(nullptr, 16, prod_pk_mpz);
        mpz_clear(prod_pk_mpz);
        return res;
    }

    EncryptedPair masterTransform(const PublicParameters &param,
                                  const MasterSecretKeyPair &secretKey,
                                  const QString &destPublicKey,
                                  const EncryptedPair &srcCiphertext) {
        return encrypt(param, destPublicKey,
                       masterUniqueDecrypt(param, secretKey, srcCiphertext));
    }

    void specGen(QString &specP, QString &specQ) {
        int k_bits = 2048;

        int p_bit = k_bits / 2;
        int q_bit = k_bits - p_bit;

        INIT_RANDOM_GENERATOR();

        mpz_t p, p_tmp;
        mpz_init(p);
        mpz_init(p_tmp);
        mpz_urandomb(p, grt, p_bit);
        while (true) {
            mpz_nextprime(p, p);
            mpz_sub_ui(p_tmp, p, 1);
            mpz_fdiv_q_ui(p_tmp, p_tmp, 2);
            if (mpz_probab_prime_p(p_tmp, 5) != 0) {
                break;
            }
        }

        mpz_t q, q_tmp;
        mpz_init(q);
        mpz_init(q_tmp);
        mpz_urandomb(q, grt, q_bit);
        while (true) {
            mpz_nextprime(q, q);
            mpz_sub_ui(q_tmp, q, 1);
            mpz_fdiv_q_ui(q_tmp, q_tmp, 2);
            if (mpz_probab_prime_p(q_tmp, 5) != 0) {
                break;
            }
        }

        specP = (mpz_get_str(nullptr, 16, p));
        specQ = (mpz_get_str(nullptr, 16, q));

        mpz_clear(p);
        mpz_clear(q);
        mpz_clear(p_tmp);
        mpz_clear(q_tmp);
    }

    EncryptedPair EncryptedPair::operator+(const EncryptedPair &another) const {
        EncryptedPair res;
        if (this->publicKey() != another.publicKey() ||
            this->publicN() != another.publicN()) {
            throw std::runtime_error("could not add encrypted pairs with different public key or public params.");
        }
        res.setPublicN(this->publicN());
        res.setPublicKey(this->publicKey());
        mpz_t N, N2;
        mpz_init_set_str(N, this->publicN().toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_mul(N2, N, N);

        mpz_t A, B;
        mpz_init(A);
        mpz_init(B);

        mpz_t A1, B1, A2, B2;
        mpz_init_set_str(A1, this->A().toStdString().c_str(), 16);
        mpz_init_set_str(B1, this->B().toStdString().c_str(), 16);
        mpz_init_set_str(A2, another.A().toStdString().c_str(), 16);
        mpz_init_set_str(B2, another.B().toStdString().c_str(), 16);

        mpz_mul(A, A1, A2);
        mpz_mod(A, A, N2);
        mpz_mul(B, B1, B2);
        mpz_mod(B, B, N2);

        res.setA(mpz_get_str(nullptr, 16, A));
        res.setB(mpz_get_str(nullptr, 16, B));

        return res;
    }

    bool ccTransform(const PublicParameters &param,
                     const EncryptedPair &srcCiphertext,
                     EncryptedPair &destCiphertext,
                     QString &destRandSnip) {
        INIT_RANDOM_GENERATOR();
        mpz_t rand_num;
        mpz_init(rand_num);
        mpz_t public_N;
        mpz_init_set_str(public_N, param.N().toStdString().c_str(), 16);
        mpz_urandomm(rand_num, grt, public_N);
        destCiphertext = encrypt(param,
                                 srcCiphertext.publicKey(),
                                 mpz_get_str(nullptr, 16, rand_num))
                         + srcCiphertext;
        return true;
    }

    EncryptedPair ccComputing(const PublicParameters &param,
                              const QString &prod_pk,
                              const QMap<QString, EncryptedPair> &enc_map,
                              const QMap<QString, QString> &r) {
        EncryptedPair res;
        res.setPublicKey(prod_pk);
        res.setPublicN(param.N());

        mpz_t tempA, tempB, temp1, temp2;
        mpz_init_set_d(tempA, 1);
        mpz_init_set_d(tempB, 1);
        mpz_init(temp1);
        mpz_init(temp2);
        mpz_t N, N2;
        mpz_init_set_str(N, param.N().toStdString().c_str(), 16);
        mpz_init(N2);
        mpz_mul(N2, N, N);

        EncryptedPair temp;
        for (auto &tag : enc_map.keys()) {
            temp = enc_map[tag] + encrypt(param, prod_pk, "-"+r[tag]);
            mpz_set_str(temp1, temp.A().toStdString().c_str(), 16);
            mpz_set_str(temp2, temp.B().toStdString().c_str(), 16);
            mpz_mul(temp1, tempA, temp1);
            mpz_mod(tempA, temp1, N2);
            mpz_mul(temp2, tempB, temp2);
            mpz_mod(tempB, temp2, N2);
        }
        res.setA(mpz_get_str(nullptr, 16, tempA));
        res.setB(mpz_get_str(nullptr, 16, tempB));
        return res;
    }
};
