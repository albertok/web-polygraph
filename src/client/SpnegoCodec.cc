
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"
#include "client/SpnegoCodec.h"
#include <string>
#include <vector>
#include <sstream>

const char * const GSSOID_KERBEROS_MECHANISM = "1.2.840.113554.1.2.2";
const char * const GSSOID_MS_KERBEROS_MECHANISM = "1.2.840.48018.1.2.2";
const char * const GSSOID_NTLMSSP_MECHANISM = "1.3.6.1.4.1.311.2.2.10";
const char * const GSSOID_SPNEGO_MECHANISM = "1.3.6.1.5.5.2";

namespace
{

typedef std::vector<char> Bytes;
typedef std::vector<std::string> Strings;

class ByteListReader;
class ByteBuffer;

enum spnego_flags {
    CONSTRUCTED = 0x20,
    APPLICATION = 0x40,
    CONTEXT_SPECIFIC = 0x80,

    BIT_STRING = 0x03,
    OCTET_STRING = 0x04,
    OID = 0x06,
    ENUMERATED = 0x0a,
    SEQUENCE = 0x10,

    NEG_TOKEN_INIT = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x00,
    NEG_TOKEN_TARG = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x01,
    MECHANISM_TYPES = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x00,
    REQUEST_FLAGS = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x01,
    MECHANISM_MATERIAL = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x02,
    MECHANISM_LIST_MIC = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x03,
    RESULT = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x00,
    MECHANISM = 0xa0 /* CONSTRUCTED | CONTEXT_SPECIFIC */ | 0x01
};

// returns either 0 or token type
// use this to confirm that we got InitToken
// in the first received message and NegToken
// in subsequent ones
int readTokenType(ByteListReader & in);

// we probably won't need that unless we try
// to "force" Negotiate/spnego by sending our
// own InitToken.
Bytes createInitiatorMaterial(
    const ByteListReader & mechanismMaterial,
    const std::vector < std::string > & mechanisms,
    const int flags,
    const Bytes & mic);

// use it to encode subsequent responses.
Bytes createTargetMaterial(
    const ByteListReader & mechanismMaterial,
    const std::string & mechanism,
    const int result,
    const Bytes & mic);

// parse InitToken, the first message in sequence.
// IN:
// in - contains the part spnego token after the part with token
//      type (the position after the call to readTokenType)
// OUT:
// mechanismMaterial   Data for preferred mechanism if applicable
//                     (usually kerberos data), otherwise empty.
// mechanisms          List with OIDs of supported auth mechanisms
//                     in printable/readable form. Optional but
//                     usually present.
// flags               Request flags. Typically ignored by spnego
//                     implementations. Optional.
// mic                 Mic value used to sign the list of supported
//                     auth methods. Optional.
// return value        False if obvious errors were detected while
//                     parsing the source material, true otherwise.
bool readInitiatorToken(ByteListReader & in,
                        Bytes & mechanismMaterial,
                        Strings & mechanisms,
                        int & flags,
                        Bytes & mic);

// parse NegToken, second and al subsequent message in the sequence
// IN:
// in - contains the part spnego token after the part with token
//      type (the position after the call to readTokenType)
// OUT:
// mechanismMaterial   Data for chosen mechanism
// mechanism           OID of chosen mechanism
// result              Current state of negotiation.
// mic                 Mic value. Optional.
// return value        False if obvious errors were detected while
//                     parsing the source material, true otherwise.
bool readTargetToken(ByteListReader & in,
                     Bytes & mechanismMaterial,
                     std::string & mechanism,
                     spnego_auth_result & result,
                     Bytes & mic);


Strings readMechanisms(ByteListReader & in);
// NOTE: request flags are usually ignored, we do that too
int readRequestFlags(ByteListReader & in);
spnego_auth_result readResult(ByteListReader & in);
Bytes readOctetString(ByteListReader & in);
Bytes readOID(ByteListReader & in);
Bytes findOID(const Bytes & material);
std::string createMechanism(const Bytes & oid);
Bytes createOID(const std::string & mechanism);
Bytes createFlags(int flags);
Bytes createHeader(const unsigned char type, const int length);
unsigned int readLength(ByteListReader & in);

// start of helper class and function implementations

class ByteListReader
{
private:
    // current position in reader
    unsigned int pos;
    // reference to vector with data (if initialized with vector)
    const Bytes * pvBytes;
    // pointer to the beginning of char array (if initialized with pointer)
    const char * pBytes;
    // size of vector/array
    unsigned int size;
public:

    ByteListReader(const Bytes & bv)
            : pos(0), pvBytes(&bv), pBytes(0), size(bv.size())
    {}
    ;

    ByteListReader(const char * ba, unsigned int asize)
            : pos(0), pvBytes(0), pBytes(ba), size(asize)
    {}
    ;

    int read()
    {
        if (pos < size)
        {
            return pBytes ? pBytes[pos++] : pvBytes->at(pos++);
        }
        // log error if we manage to get here
        return 0;
    };

    // read specified number of bytes and append
    // to the given vector
    int read(Bytes & values, int length)
    {
        unsigned int target = pos + length;
        if (target > size)
        {
            target = size;
        }
        int bytes_read = 0;
        while(pos < target)
        {
            values.push_back(pBytes ? pBytes[pos++] : pvBytes->at(pos++));
            bytes_read++;
        }
        return bytes_read;
    };

    bool hasNext()
    {
        return pos < size;
    };

    unsigned int unreadBytes()
    {
        return size - pos;
    };
};

// simulates prepending of byte sequences to the beginning of the buffer
class ByteBuffer
{
public:

    ByteBuffer() : content()
    {}
    ;

    int size()
    {
        return content.size();
    };

    void put(const Bytes & bytes)
    {
        content.insert(content.end(), bytes.rbegin(), bytes.rend());
    };

    Bytes get()
    {
        return Bytes (content.rbegin(), content.rend());
    };

private:
    Bytes content;
};



int readTokenType(ByteListReader & in)
{
    if (in.read() == (CONSTRUCTED | APPLICATION))
    {
        readLength(in);
        if (in.read() != OID)
        {
            // log error no object identifier
            return 0;
        }
        readLength(in);
        if (createOID(GSSOID_SPNEGO_MECHANISM) != readOID(in))
        {
            // log error Not SPNEGO material.
            return 0;
        }
    }
    int val = in.read();
    if (val == NEG_TOKEN_INIT || val == NEG_TOKEN_TARG)
    {
        return val;
    }

    // log error Unknown material type
    return 0;
}

Bytes createInitiatorMaterial(
    const Bytes & mechanismMaterial,
    const Strings & mechanisms,
    const int flags,
    const Bytes & mic)
{
    ByteBuffer buff;
    int length = 0;
    Bytes bytes;
    length = mic.size();
    if (length)
    {
        buff.put(mic);
        bytes = createHeader(OCTET_STRING, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM_LIST_MIC, length + bytes.size()));
    }
    length = mechanismMaterial.size();
    if (length)
    {
        buff.put(mechanismMaterial);
        bytes = createHeader(OCTET_STRING, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM_MATERIAL, length + bytes.size()));
    }
    if (flags)
    {
        bytes = createFlags(flags);
        buff.put(bytes);
        length = bytes.size();
        bytes = createHeader(BIT_STRING, length);
        buff.put(bytes);
        buff.put(createHeader(REQUEST_FLAGS, length + bytes.size()));
    }
    length = 0;
    if (mechanismMaterial.size())
    {
        bytes = findOID(mechanismMaterial);
        if (mechanisms.size())
        {
            for (int i = mechanisms.size() - 1; i >= 0; i--)
            {
                Bytes oid = createOID(mechanisms[i]);
                if (bytes != oid)
                {
                    buff.put(oid);
                    length += oid.size();
                    oid = createHeader(OID, oid.size());
                    buff.put(oid);
                    length += oid.size();
                }
            }
        }
        if (bytes.size())
        {
            buff.put(bytes);
            length += bytes.size();
            bytes = createHeader(OID, bytes.size());
            buff.put(bytes);
            length += bytes.size();
        }
        bytes = createHeader(CONSTRUCTED | SEQUENCE, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM_TYPES, bytes.size() + length));
    }
    buff.put(createHeader(CONSTRUCTED | SEQUENCE, buff.size()));
    buff.put(createHeader(NEG_TOKEN_INIT, buff.size()));
    Bytes spnego_oid = createOID(GSSOID_SPNEGO_MECHANISM);
    buff.put(spnego_oid);
    buff.put(createHeader(OID, spnego_oid.size()));
    buff.put(createHeader(CONSTRUCTED | APPLICATION, buff.size()));
    return buff.get();
}

// use it to encode subsequent responses.
Bytes createTargetMaterial(
    const Bytes & mechanismMaterial,
    const std::string & mechanism,
    const int result,
    const Bytes & mic)
{
    ByteBuffer buff;
    int length = 0;
    Bytes bytes;
    if (mic.size())
    {
        buff.put(mic);
        length = mic.size();
        bytes = createHeader(OCTET_STRING, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM_LIST_MIC, length + bytes.size()));
    }
    if (mechanismMaterial.size())
    {
        buff.put(mechanismMaterial);
        length = mechanismMaterial.size();
        bytes = createHeader(OCTET_STRING, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM_MATERIAL, length + bytes.size()));
    }
    bytes = createOID(mechanism);
    if (bytes.size())
    {
        buff.put(bytes);
        length = bytes.size();
        bytes = createHeader(OID, length);
        buff.put(bytes);
        buff.put(createHeader(MECHANISM, length + bytes.size()));
    }
    switch (result)
    {
    case ACCEPT_COMPLETED:
    case ACCEPT_INCOMPLETE:
        if (mechanism.size())
        {
            bytes.resize(1);
            bytes[0] = result;
            buff.put(bytes);
            bytes = createHeader(ENUMERATED, 1);
            buff.put(bytes);
            buff.put(createHeader(RESULT, bytes.size() + 1));
        }
        break;
    case REJECTED:
        if (mechanism.size())
        {
            // log error Supplied mechanism with rejection
        }
        bytes.resize(1);
        bytes[0] = result;
        buff.put(bytes);
        bytes = createHeader(ENUMERATED, 1);
        buff.put(bytes);
        buff.put(createHeader(RESULT, bytes.size() + 1));
        break;
    default:
        // log error Invalid result code
        break;
    }
    buff.put(createHeader(CONSTRUCTED | SEQUENCE, buff.size()));
    buff.put(createHeader(NEG_TOKEN_TARG, buff.size()));
    return buff.get();
}

// parse InitToken - the first message in sequence
bool readInitiatorToken(ByteListReader & in,
                        Bytes & mechanismMaterial,
                        Strings & mechanisms,
                        int & flags,
                        Bytes & mic)
{
    int length = readLength(in);
    if (in.read() != (CONSTRUCTED | SEQUENCE))
    {
        // log error no acceptable content
        return false;
    }
    mechanismMaterial.resize(0);
    mechanisms.resize(0);
    flags = 0;
    mic.resize(0);

    length = readLength(in);
    while (length > 0)
    {
        // read type and ignore it
        in.read();
        int partLength = readLength(in);
        switch (in.read())
        {
        case MECHANISM_TYPES:
            mechanisms = readMechanisms(in);
            break;
        case REQUEST_FLAGS:
            flags = readRequestFlags(in);
            break;
        case MECHANISM_MATERIAL:
            mechanismMaterial = readOctetString(in);
            break;
        case MECHANISM_LIST_MIC:
            mic = readOctetString(in);
            break;
        default:
            // log error Unrecognized content
            return false;
        }
        length -= partLength;
    }
    return true;
}

// parse subsequent NegToken message
bool readTargetToken(ByteListReader & in,
                     Bytes & mechanismMaterial,
                     std::string & mechanism,
                     spnego_auth_result & result,
                     Bytes & mic)
{
    int length = readLength(in);
    if (in.read() != (CONSTRUCTED | SEQUENCE))
    {
        // log error unrecognized content
        return false;
    }

    mechanismMaterial.resize(0);
    mechanism.clear();
    result = ACCEPT_COMPLETED;
    mic.resize(0);

    length = readLength(in);
    while (length > 0)
    {
        // read type and ignore it
        in.read();
        int partLength = readLength(in);
        switch (in.read())
        {
        case RESULT:
            result = readResult(in);
            break;
        case MECHANISM:
            mechanism = createMechanism(readOID(in));
            break;
        case MECHANISM_MATERIAL:
            mechanismMaterial = readOctetString(in);
            break;
        case MECHANISM_LIST_MIC:
            mic = readOctetString(in);
            break;
        default:
            // log error unrecognized content
            return false;
        }
        length -= partLength;
    }
    return true;
}

Strings readMechanisms(ByteListReader & in)
{
    Strings mechanisms;
    if (in.read() != (CONSTRUCTED | SEQUENCE))
    {
        // log error No mechanism list content
        return mechanisms;
    }
    unsigned int length = readLength(in);
    Bytes bytes;
    in.read(bytes, length);
    if (length != bytes.size())
    {
        // log error premature end of data
        return mechanisms;
    }
    ByteListReader mech_in(bytes);
    while (mech_in.hasNext())
    {
        mechanisms.push_back(createMechanism(readOID(mech_in)));
    }
    return mechanisms;
}

// BTW, req flags are usually ignored anyway
int readRequestFlags(ByteListReader & in)
{
    if (in.read() != BIT_STRING)
    {
        // log error No request flag content
        return 0;
    }
    // we know the length is 2, and the first octet is 1 (number of shifts).
    int length = readLength(in);
    if (length != 2)
    {
        // log error Unable to fully read flags
        return 0;
    }
    Bytes bytes;
    in.read(bytes, length);
    if (length != 2)
    {
        // log error
        return 0;
    }
    return bytes[1] >> 1;
}

spnego_auth_result readResult(ByteListReader & in)
{
    if (in.read() != ENUMERATED)
    {
        // log error
        return REJECTED;
    }
    readLength(in);
    // length will always be 1
    switch (in.read())
    {
    case ACCEPT_COMPLETED:
        return ACCEPT_COMPLETED;
    case ACCEPT_INCOMPLETE:
        return ACCEPT_INCOMPLETE;
    case REJECTED:
        return REJECTED;
    default:
        // log error;
        return REJECTED;
    };
}

Bytes readOctetString(ByteListReader & in)
{
    Bytes bytes;
    if (in.read() != OCTET_STRING)
    {
        // log error not an octet string
        return bytes;
    }
    unsigned int length = readLength(in);
    in.read(bytes, length);
    if (length != bytes.size())
    {
        // log error premature end of content
    }
    return bytes;
}

Bytes readOID(ByteListReader & in)
{
    Bytes bytes;
    if (in.read() != OID)
    {
        // log error
        return bytes;
    }
    unsigned int length = readLength(in);
    in.read(bytes, length);
    if(bytes.size() != length)
    {
        // log error
    }
    return bytes;
}

Bytes findOID(const Bytes & material)
{
    Bytes bytes;
    ByteListReader in (material);
    if (in.read() != (CONSTRUCTED | APPLICATION))
    {
        // log error here
        return bytes;
    }
    readLength(in);

    if (in.read() != OID)
    {
        // log error
        return bytes;
    }
    unsigned int length = readLength(in);
    in.read(bytes,length);
    if(bytes.size() != length)
    {
        // log error
    }
    return bytes;
}

std::string createMechanism(const Bytes & oid)
{
    if (oid.size() == 0)
    {
        // log error
        return "";
    }
    std::ostringstream oss;
    unsigned char value = oid[0];
    oss << (value / 40) << "." << (value % 40);
    int oid_size = oid.size();
    for (int i = 1; i < oid_size; i++)
    {
        value = 0;
        do
        {
            value = (value << 7) | (oid[i] & 0x7f);
        }
        while ((oid[i++] & 0x80) && i < oid_size);
        oss << "." << value;
    }
    return oss.str();
}

static int dottedAtoi(const char* & str)
{
    int value = 0;
    while (*str && *str != '.')
    {
        value = 10 * value + (*str - '0');
        str++;
    }
    if (*str == '.')
    {
        str++;
    }
    return value;
}

Bytes createOID(const std::string & mechanism)
{
    Bytes bytes;
    const char* str = mechanism.c_str();
    int value;
    // byte 1 is (40 * first component) + second component
    value = dottedAtoi(str);
    bytes.push_back((40 * value) + dottedAtoi(str));
    // rest of bytes are base-128 representation of each component
    // high bit set for all but the last byte of each block
    while(*str)
    {
        value = dottedAtoi(str);
        bytes.push_back(value & 0x7f);
        value >>= 7;
        Bytes :: iterator it = bytes.end();
        --it;
        while (value != 0)
        {
            it = bytes.insert(it, (value & 0x7f) | 0x80);
            value >>= 7;
        }
    }
    return bytes;
}

Bytes createFlags(int flags)
{

    // DER-encoded bit string; we know a zero will need to be right-padded.
    Bytes bytes;
    flags <<= 1;
    bytes.push_back(1);
    bytes.push_back((char) (flags & 0x0ff));
    return bytes;
}

Bytes createHeader(const unsigned char type, const int length)
{
    Bytes header;
    header.push_back(type);
    if (length < 0x80)
    {
        header.push_back((char)(length & 0x7f));
    }
    else
    {
        int rest = length;
        header.push_back(2); // number of bytes for length
        header.push_back(0); // zero byte at the beginning
        header.push_back((char)(rest & 0xff));
        Bytes :: iterator it = header.end();
        --it;
        rest >>= 8;
        while(rest != 0)
        {
            it = header.insert(it, (char)(rest & 0xff));
            rest >>= 8;
            header[1]++;
        }
        header[1] |= 0x80;
    }
    return header;
}

unsigned int readLength(ByteListReader & in)
{
    char ch;
    ch = in.read();

    if ((ch & 0x80) == 0)
    {
        return ch;
    }

    unsigned int length = 0;
    for (char bytes2read = ch & 0x7f; bytes2read != 0; --bytes2read)
    {
        ch = in.read();
        length = (length << 8) | ch;
    }
    return length;
}

inline bool isSupportedMech(const char * mech)
{
    return (strcmp(mech, GSSOID_KERBEROS_MECHANISM) == 0
            || strcmp(mech, GSSOID_MS_KERBEROS_MECHANISM) == 0
            || strcmp(mech, GSSOID_NTLMSSP_MECHANISM) == 0);
}

}

bool spnegoCheckInitMsg(
    const char *spnegoPacket,
    const unsigned int size,
    bool & isMechanismPresent,
    const char * mechanism)
{
    isMechanismPresent = false;
    ByteListReader br(spnegoPacket, size);
    if (readTokenType(br) != NEG_TOKEN_INIT)
    {
        return false;
    }

    if(!isSupportedMech(mechanism))
    {
        // log error
        return 0;
    }
    std::string mech(mechanism);

    Bytes mechanismMaterial;
    Strings mechanisms;
    int flags;
    Bytes mic;
    if (! readInitiatorToken(br,mechanismMaterial,mechanisms, flags, mic))
    {
        return false;
    }
    for (int i = mechanisms.size(); i >= 0; --i)
    {
        if (mechanisms[i] == mech)
        {
            isMechanismPresent = true;
            return true;
        }
    }
    return false;
}

bool spnegoCheckNtlmInitMsg(
    const char *spnegoPacket,
    const unsigned int size,
    bool & isMechanismPresent)
{
    return spnegoCheckInitMsg(spnegoPacket, size, isMechanismPresent, GSSOID_NTLMSSP_MECHANISM);
}


// return value is the size of output buffer; zero == error
unsigned int spnegoWrapMaterial(
    const char *material, // auth material
    unsigned int msize,
    const char *mechanism, // one of auth mechanisms known to the lib
    char * output, // where to put the result (not yet base64 encoded)
    unsigned int max_output,
    spnego_auth_result rc // auth result
)
{
    Bytes mic;
    if(!isSupportedMech(mechanism))
    {
        // log error
        return 0;
    }
    std::string mech(mechanism);

    Bytes bytes(material, material + msize);
    Bytes spnego = createTargetMaterial(bytes, mech, rc, mic);
    unsigned int size = spnego.size();
    if (size > max_output)
    {
        // log error
        size = max_output;
    }
    for (int i = size-1; i >=0; --i)
    {
        output[i] = spnego[i];
    }
    return size;
}

// return value is the size of output buffer; zero == error
unsigned int spnegoUnwrapMaterial(
    const char * spnegoPacket,
    unsigned int ssize,
    const char * mechanism, // just to verify
    char * output, // where to put extracted auth blob
    unsigned int max_output, // max size of buffer
    spnego_auth_result & rc // auth result
)
{
    rc = ACCEPT_INCOMPLETE;

    if(!isSupportedMech(mechanism))
    {
        // log error
        return 0;
    }
    std::string mech(mechanism);

    ByteListReader br(spnegoPacket, ssize);
    if (readTokenType(br)
            != NEG_TOKEN_TARG)
    {
        // log error
        return 0;
    }

    Bytes mechanismMaterial;
    std::string realMechanism;
    Bytes mic;
    if (! readTargetToken(br, mechanismMaterial, realMechanism, rc, mic)
       )
    {
        // log error
        return 0;
    }

    if ( mech != realMechanism)
    {
        // log error
        return 0;
    }

    if (rc != ACCEPT_INCOMPLETE && rc != ACCEPT_COMPLETED)
    {
        // log error
        return 0;
    }

    if ( mic.size()
       )
    {
        // log warning, we do not support mics
        mic.resize(0);
    }

    size_t size = mechanismMaterial.size();

    if (size > max_output)
    {
        // log error
        size = max_output;
    }


    for (int i = (size-1) ; i >=0; --i)
    {
        output[i] = mechanismMaterial[i];
    }
    return size;
}

unsigned int spnegoUnwrapNtlmMaterial(
    const char * spnegoPacket,
    const unsigned int size,
    char * output, // where to put extracted ntlm blob
    unsigned int max_output, // max size of buffer
    spnego_auth_result & rc // auth result
)
{
    return spnegoUnwrapMaterial(spnegoPacket, size, GSSOID_NTLMSSP_MECHANISM, output, max_output, rc);
}

unsigned int spnegoWrapNtlmMaterial(
    const char *material, // auth material
    const unsigned int size,
    char * output, // where to put the result (not yet base64 encoded)
    unsigned int max_output,
    spnego_auth_result rc // auth result
)
{
    return spnegoWrapMaterial(material, size, GSSOID_NTLMSSP_MECHANISM, output, max_output, rc);
}

