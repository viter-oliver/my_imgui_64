/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#ifndef __exlcm_HMI_Lane_Lines_hpp__
#define __exlcm_HMI_Lane_Lines_hpp__

#include <lcm/lcm_coretypes.h>

#include "exlcm\HMI_Lane_Line.hpp"
#include "exlcm\HMI_Lane_Line.hpp"
#include "exlcm\HMI_Lane_Line.hpp"
#include "exlcm\HMI_Lane_Line.hpp"

namespace exlcm
{

class HMI_Lane_Lines
{
    public:
        exlcm::HMI_Lane_Line left_line;

        /// 左车道线				
        exlcm::HMI_Lane_Line right_line;

        /// 右车道线				
        exlcm::HMI_Lane_Line next_left_line;

        /// 左侧车道左车道线		
        exlcm::HMI_Lane_Line next_right_line;

    public:
        /**
         * Encode a message into binary form.
         *
         * @param buf The output buffer.
         * @param offset Encoding starts at thie byte offset into @p buf.
         * @param maxlen Maximum number of bytes to write.  This should generally be
         *  equal to getEncodedSize().
         * @return The number of bytes encoded, or <0 on error.
         */
        inline int encode(void *buf, int offset, int maxlen) const;

        /**
         * Check how many bytes are required to encode this message.
         */
        inline int getEncodedSize() const;

        /**
         * Decode a message from binary form into this instance.
         *
         * @param buf The buffer containing the encoded message.
         * @param offset The byte offset into @p buf where the encoded message starts.
         * @param maxlen The maximum number of bytes to read while decoding.
         * @return The number of bytes decoded, or <0 if an error occured.
         */
        inline int decode(const void *buf, int offset, int maxlen);

        /**
         * Retrieve the 64-bit fingerprint identifying the structure of the message.
         * Note that the fingerprint is the same for all instances of the same
         * message type, and is a fingerprint on the message type definition, not on
         * the message contents.
         */
        inline static int64_t getHash();

        /**
         * Returns "HMI_Lane_Lines"
         */
        inline static const char* getTypeName();

        // LCM support functions. Users should not call these
        inline int _encodeNoHash(void *buf, int offset, int maxlen) const;
        inline int _getEncodedSizeNoHash() const;
        inline int _decodeNoHash(const void *buf, int offset, int maxlen);
        inline static uint64_t _computeHash(const __lcm_hash_ptr *p);
};

int HMI_Lane_Lines::encode(void *buf, int offset, int maxlen) const
{
    int pos = 0, tlen;
    int64_t hash = getHash();

    tlen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->_encodeNoHash(buf, offset + pos, maxlen - pos);
    if (tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int HMI_Lane_Lines::decode(const void *buf, int offset, int maxlen)
{
    int pos = 0, thislen;

    int64_t msg_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &msg_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (msg_hash != getHash()) return -1;

    thislen = this->_decodeNoHash(buf, offset + pos, maxlen - pos);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int HMI_Lane_Lines::getEncodedSize() const
{
    return 8 + _getEncodedSizeNoHash();
}

int64_t HMI_Lane_Lines::getHash()
{
    static int64_t hash = static_cast<int64_t>(_computeHash(NULL));
    return hash;
}

const char* HMI_Lane_Lines::getTypeName()
{
    return "HMI_Lane_Lines";
}

int HMI_Lane_Lines::_encodeNoHash(void *buf, int offset, int maxlen) const
{
    int pos = 0, tlen;

    tlen = this->left_line._encodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->right_line._encodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->next_left_line._encodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->next_right_line._encodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int HMI_Lane_Lines::_decodeNoHash(const void *buf, int offset, int maxlen)
{
    int pos = 0, tlen;

    tlen = this->left_line._decodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->right_line._decodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->next_left_line._decodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->next_right_line._decodeNoHash(buf, offset + pos, maxlen - pos);
    if(tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int HMI_Lane_Lines::_getEncodedSizeNoHash() const
{
    int enc_size = 0;
    enc_size += this->left_line._getEncodedSizeNoHash();
    enc_size += this->right_line._getEncodedSizeNoHash();
    enc_size += this->next_left_line._getEncodedSizeNoHash();
    enc_size += this->next_right_line._getEncodedSizeNoHash();
    return enc_size;
}

uint64_t HMI_Lane_Lines::_computeHash(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for(fp = p; fp != NULL; fp = fp->parent)
        if(fp->v == HMI_Lane_Lines::getHash)
            return 0;
    const __lcm_hash_ptr cp = { p, HMI_Lane_Lines::getHash };

    uint64_t hash = 0x512788b262e951e1LL +
         exlcm::HMI_Lane_Line::_computeHash(&cp) +
         exlcm::HMI_Lane_Line::_computeHash(&cp) +
         exlcm::HMI_Lane_Line::_computeHash(&cp) +
         exlcm::HMI_Lane_Line::_computeHash(&cp);

    return (hash<<1) + ((hash>>63)&1);
}

}

#endif
