#include "JWTUtils.h"

#include "json.hpp"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

void JWTUtils::generateRSAKeys(std::string& private_key, std::string& public_key) {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
    EVP_PKEY_keygen(ctx, &pkey);
    EVP_PKEY_CTX_free(ctx);
    
    BIO* priv_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(priv_bio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    
    char* priv_data;
    long priv_len = BIO_get_mem_data(priv_bio, &priv_data);
    private_key = std::string(priv_data, priv_len);
    
    BIO* pub_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(pub_bio, pkey);
    
    char* pub_data;
    long pub_len = BIO_get_mem_data(pub_bio, &pub_data);
    public_key = std::string(pub_data, pub_len);
    
    BIO_free(priv_bio);
    BIO_free(pub_bio);
    EVP_PKEY_free(pkey);
}

std::string JWTUtils::generateToken(const std::string& user_id, const std::string& game_id, const std::string& private_key)
{
    nlohmann::json header = {
        {"alg", "RS256"},
        {"typ", "JWT"}
    };
    
    nlohmann::json payload = {
        {"user_id", user_id},
        {"game_id", game_id}
    };
    
    std::string header_b64 = base64_encode(header.dump());
    std::string payload_b64 = base64_encode(payload.dump());
    
    std::string data = header_b64 + "." + payload_b64;
    std::string signature = rsa_sign(private_key, data);
    std::string signature_b64 = base64_encode(signature);
    
    return data + "." + signature_b64;
}

bool JWTUtils::verifyToken(const std::string& token, const std::string& public_key)
{
    try
    {
        size_t dot1 = token.find('.');
        size_t dot2 = token.find('.', dot1 + 1);
        
        if (dot1 == std::string::npos || dot2 == std::string::npos) {
            return false;
        }
        
        std::string header_b64 = token.substr(0, dot1);
        std::string payload_b64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
        std::string signature_b64 = token.substr(dot2 + 1);
        
        std::string data = header_b64 + "." + payload_b64;
        std::string signature = base64_decode(signature_b64);
        
        if (!rsa_verify(public_key, data, signature)) {
            return false;
        }
        
        return true;
        
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::string JWTUtils::base64_encode(const std::string& in)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, in.c_str(), in.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);
    return result;
}

std::string JWTUtils::base64_decode(const std::string& in)
{
    BIO *mem = BIO_new_mem_buf(in.data(), static_cast<int>(in.length()));
    if (!mem) 
    {
        return "";
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    std::vector<char> buffer(in.length());
    int decoded_length = BIO_read(b64, static_cast<void*>(buffer.data()), static_cast<int>(buffer.size()));
    
    std::string result;
    if (decoded_length > 0) {
        result.assign(buffer.data(), decoded_length);
    }

    BIO_free_all(b64);
    return result;
}

bool JWTUtils::rsa_verify(const std::string& public_key, const std::string& data, const std::string& signature)
{
    BIO* bio = BIO_new_mem_buf(public_key.data(), public_key.size());
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) return false;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_VerifyInit(ctx, EVP_sha256());
    EVP_VerifyUpdate(ctx, data.data(), data.size());
    
    bool ok = EVP_VerifyFinal(ctx, (unsigned char*)signature.data(), 
                             signature.size(), pkey) == 1;
    
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    
    return ok;
}

std::string JWTUtils::rsa_sign(const std::string& private_key, const std::string& data) {
    BIO* bio = BIO_new_mem_buf(private_key.data(), private_key.size());
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) return "";
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_SignInit(ctx, EVP_sha256());
    EVP_SignUpdate(ctx, data.data(), data.size());
    
    unsigned char sig[4096];
    unsigned int sig_len;
    
    if (EVP_SignFinal(ctx, sig, &sig_len, pkey) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return "";
    }
    
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    
    return std::string((char*)sig, sig_len);
}
