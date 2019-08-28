// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The KoreCore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "script.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

namespace {
inline std::string ValueString(const std::vector<unsigned char>& vch)
{
    if (vch.size() <= 4)
        return strprintf("%d", CScriptNum(vch, false).getint());
    else
        return HexStr(vch);
}
} // anon namespace

using namespace std;

const char* GetOpName(opcodetype opcode)
{
    switch (opcode)
    {
    // push value
    case OP_0                      : return "0";
    case OP_PUSHDATA1              : return "OP_PUSHDATA1";
    case OP_PUSHDATA2              : return "OP_PUSHDATA2";
    case OP_PUSHDATA4              : return "OP_PUSHDATA4";
    case OP_1NEGATE                : return "-1";
    case OP_RESERVED               : return "OP_RESERVED";
    case OP_1                      : return "1";
    case OP_2                      : return "2";
    case OP_3                      : return "3";
    case OP_4                      : return "4";
    case OP_5                      : return "5";
    case OP_6                      : return "6";
    case OP_7                      : return "7";
    case OP_8                      : return "8";
    case OP_9                      : return "9";
    case OP_10                     : return "10";
    case OP_11                     : return "11";
    case OP_12                     : return "12";
    case OP_13                     : return "13";
    case OP_14                     : return "14";
    case OP_15                     : return "15";
    case OP_16                     : return "16";

    // control
    case OP_NOP                    : return "OP_NOP";
    case OP_VER                    : return "OP_VER";
    case OP_IF                     : return "OP_IF";
    case OP_NOTIF                  : return "OP_NOTIF";
    case OP_VERIF                  : return "OP_VERIF";
    case OP_VERNOTIF               : return "OP_VERNOTIF";
    case OP_ELSE                   : return "OP_ELSE";
    case OP_ENDIF                  : return "OP_ENDIF";
    case OP_VERIFY                 : return "OP_VERIFY";
    case OP_RETURN                 : return "OP_RETURN";

    // stack ops
    case OP_TOALTSTACK             : return "OP_TOALTSTACK";
    case OP_FROMALTSTACK           : return "OP_FROMALTSTACK";
    case OP_2DROP                  : return "OP_2DROP";
    case OP_2DUP                   : return "OP_2DUP";
    case OP_3DUP                   : return "OP_3DUP";
    case OP_2OVER                  : return "OP_2OVER";
    case OP_2ROT                   : return "OP_2ROT";
    case OP_2SWAP                  : return "OP_2SWAP";
    case OP_IFDUP                  : return "OP_IFDUP";
    case OP_DEPTH                  : return "OP_DEPTH";
    case OP_DROP                   : return "OP_DROP";
    case OP_DUP                    : return "OP_DUP";
    case OP_NIP                    : return "OP_NIP";
    case OP_OVER                   : return "OP_OVER";
    case OP_PICK                   : return "OP_PICK";
    case OP_ROLL                   : return "OP_ROLL";
    case OP_ROT                    : return "OP_ROT";
    case OP_SWAP                   : return "OP_SWAP";
    case OP_TUCK                   : return "OP_TUCK";

    // splice ops
    case OP_CAT                    : return "OP_CAT";
    case OP_SUBSTR                 : return "OP_SUBSTR";
    case OP_LEFT                   : return "OP_LEFT";
    case OP_RIGHT                  : return "OP_RIGHT";
    case OP_SIZE                   : return "OP_SIZE";

    // bit logic
    case OP_INVERT                 : return "OP_INVERT";
    case OP_AND                    : return "OP_AND";
    case OP_OR                     : return "OP_OR";
    case OP_XOR                    : return "OP_XOR";
    case OP_EQUAL                  : return "OP_EQUAL";
    case OP_EQUALVERIFY            : return "OP_EQUALVERIFY";
    case OP_RESERVED1              : return "OP_RESERVED1";
    case OP_RESERVED2              : return "OP_RESERVED2";

    // numeric
    case OP_1ADD                   : return "OP_1ADD";
    case OP_1SUB                   : return "OP_1SUB";
    case OP_2MUL                   : return "OP_2MUL";
    case OP_2DIV                   : return "OP_2DIV";
    case OP_NEGATE                 : return "OP_NEGATE";
    case OP_ABS                    : return "OP_ABS";
    case OP_NOT                    : return "OP_NOT";
    case OP_0NOTEQUAL              : return "OP_0NOTEQUAL";
    case OP_ADD                    : return "OP_ADD";
    case OP_SUB                    : return "OP_SUB";
    case OP_MUL                    : return "OP_MUL";
    case OP_DIV                    : return "OP_DIV";
    case OP_MOD                    : return "OP_MOD";
    case OP_LSHIFT                 : return "OP_LSHIFT";
    case OP_RSHIFT                 : return "OP_RSHIFT";
    case OP_BOOLAND                : return "OP_BOOLAND";
    case OP_BOOLOR                 : return "OP_BOOLOR";
    case OP_NUMEQUAL               : return "OP_NUMEQUAL";
    case OP_NUMEQUALVERIFY         : return "OP_NUMEQUALVERIFY";
    case OP_NUMNOTEQUAL            : return "OP_NUMNOTEQUAL";
    case OP_LESSTHAN               : return "OP_LESSTHAN";
    case OP_GREATERTHAN            : return "OP_GREATERTHAN";
    case OP_LESSTHANOREQUAL        : return "OP_LESSTHANOREQUAL";
    case OP_GREATERTHANOREQUAL     : return "OP_GREATERTHANOREQUAL";
    case OP_MIN                    : return "OP_MIN";
    case OP_MAX                    : return "OP_MAX";
    case OP_WITHIN                 : return "OP_WITHIN";

    // crypto
    case OP_RIPEMD160              : return "OP_RIPEMD160";
    case OP_SHA1                   : return "OP_SHA1";
    case OP_SHA256                 : return "OP_SHA256";
    case OP_HASH160                : return "OP_HASH160";
    case OP_HASH256                : return "OP_HASH256";
    case OP_CODESEPARATOR          : return "OP_CODESEPARATOR";
    case OP_CHECKSIG               : return "OP_CHECKSIG";
    case OP_CHECKSIGVERIFY         : return "OP_CHECKSIGVERIFY";
    case OP_CHECKMULTISIG          : return "OP_CHECKMULTISIG";
    case OP_CHECKMULTISIGVERIFY    : return "OP_CHECKMULTISIGVERIFY";

    // expanson
    case OP_NOP1                   : return "OP_NOP1";
    case OP_CHECKLOCKTIMEVERIFY    : return "OP_CHECKLOCKTIMEVERIFY";
    case OP_CHECKSEQUENCEVERIFY    : return "OP_CHECKSEQUENCEVERIFY";
    case OP_NOP4                   : return "OP_NOP4";
    case OP_NOP5                   : return "OP_NOP5";
    case OP_NOP6                   : return "OP_NOP6";
    case OP_NOP7                   : return "OP_NOP7";
    case OP_NOP8                   : return "OP_NOP8";
    case OP_NOP9                   : return "OP_NOP9";
    case OP_NOP10                  : return "OP_NOP10";

    case OP_INVALIDOPCODE          : return "OP_INVALIDOPCODE";

    // Note:
    //  The template matching params OP_SMALLINTEGER/etc are defined in opcodetype enum
    //  as kind of implementation hack, they are *NOT* real opcodes.  If found in real
    //  Script, just let the default: case deal with them.

    default:
        return "OP_UNKNOWN";
    }
}

/**
 * Used for swap dump
 */
bool GetOpFromName(string str, opcodetype& opcode)
{
    if (str == "0") opcode = OP_0;
    else if (str == "OP_PUSHDATA1") opcode = OP_PUSHDATA1;
    else if (str == "OP_PUSHDATA2") opcode = OP_PUSHDATA2;
    else if (str == "OP_PUSHDATA4") opcode = OP_PUSHDATA4;
    else if (str == "-1") opcode = OP_1NEGATE;
    else if (str == "OP_RESERVED") opcode = OP_RESERVED;
    else if (str == "1") opcode = OP_1;
    else if (str == "2") opcode = OP_2;
    else if (str == "3") opcode = OP_3;
    else if (str == "4") opcode = OP_4;
    else if (str == "5") opcode = OP_5;
    else if (str == "6") opcode = OP_6;
    else if (str == "7") opcode = OP_7;
    else if (str == "8") opcode = OP_8;
    else if (str == "9") opcode = OP_9;
    else if (str == "10") opcode = OP_10;
    else if (str == "11") opcode = OP_11;
    else if (str == "12") opcode = OP_12;
    else if (str == "13") opcode = OP_13;
    else if (str == "14") opcode = OP_14;
    else if (str == "15") opcode = OP_15;
    else if (str == "16") opcode = OP_16;
    else if (str == "OP_NOP") opcode = OP_NOP;
    else if (str == "OP_VER") opcode = OP_VER;
    else if (str == "OP_IF") opcode = OP_IF;
    else if (str == "OP_NOTIF") opcode = OP_NOTIF;
    else if (str == "OP_VERIF") opcode = OP_VERIF;
    else if (str == "OP_VERNOTIF") opcode = OP_VERNOTIF;
    else if (str == "OP_ELSE") opcode = OP_ELSE;
    else if (str == "OP_ENDIF") opcode = OP_ENDIF;
    else if (str == "OP_VERIFY") opcode = OP_VERIFY;
    else if (str == "OP_RETURN") opcode = OP_RETURN;
    else if (str == "OP_TOALTSTACK") opcode = OP_TOALTSTACK;
    else if (str == "OP_FROMALTSTACK") opcode = OP_FROMALTSTACK;
    else if (str == "OP_2DROP") opcode = OP_2DROP;
    else if (str == "OP_2DUP") opcode = OP_2DUP;
    else if (str == "OP_3DUP") opcode = OP_3DUP;
    else if (str == "OP_2OVER") opcode = OP_2OVER;
    else if (str == "OP_2ROT") opcode = OP_2ROT;
    else if (str == "OP_2SWAP") opcode = OP_2SWAP;
    else if (str == "OP_IFDUP") opcode = OP_IFDUP;
    else if (str == "OP_DEPTH") opcode = OP_DEPTH;
    else if (str == "OP_DROP") opcode = OP_DROP;
    else if (str == "OP_DUP") opcode = OP_DUP;
    else if (str == "OP_NIP") opcode = OP_NIP;
    else if (str == "OP_OVER") opcode = OP_OVER;
    else if (str == "OP_PICK") opcode = OP_PICK;
    else if (str == "OP_ROLL") opcode = OP_ROLL;
    else if (str == "OP_ROT") opcode = OP_ROT;
    else if (str == "OP_SWAP") opcode = OP_SWAP;
    else if (str == "OP_TUCK") opcode = OP_TUCK;
    else if (str == "OP_CAT") opcode = OP_CAT;
    else if (str == "OP_SUBSTR") opcode = OP_SUBSTR;
    else if (str == "OP_LEFT") opcode = OP_LEFT;
    else if (str == "OP_RIGHT") opcode = OP_RIGHT;
    else if (str == "OP_SIZE") opcode = OP_SIZE;
    else if (str == "OP_INVERT") opcode = OP_INVERT;
    else if (str == "OP_AND") opcode = OP_AND;
    else if (str == "OP_OR") opcode = OP_OR;
    else if (str == "OP_XOR") opcode = OP_XOR;
    else if (str == "OP_EQUAL") opcode = OP_EQUAL;
    else if (str == "OP_EQUALVERIFY") opcode = OP_EQUALVERIFY;
    else if (str == "OP_RESERVED1") opcode = OP_RESERVED1;
    else if (str == "OP_RESERVED2") opcode = OP_RESERVED2;
    else if (str == "OP_1ADD") opcode = OP_1ADD;
    else if (str == "OP_1SUB") opcode = OP_1SUB;
    else if (str == "OP_2MUL") opcode = OP_2MUL;
    else if (str == "OP_2DIV") opcode = OP_2DIV;
    else if (str == "OP_NEGATE") opcode = OP_NEGATE;
    else if (str == "OP_ABS") opcode = OP_ABS;
    else if (str == "OP_NOT") opcode = OP_NOT;
    else if (str == "OP_0NOTEQUAL") opcode = OP_0NOTEQUAL;
    else if (str == "OP_ADD") opcode = OP_ADD;
    else if (str == "OP_SUB") opcode = OP_SUB;
    else if (str == "OP_MUL") opcode = OP_MUL;
    else if (str == "OP_DIV") opcode = OP_DIV;
    else if (str == "OP_MOD") opcode = OP_MOD;
    else if (str == "OP_LSHIFT") opcode = OP_LSHIFT;
    else if (str == "OP_RSHIFT") opcode = OP_RSHIFT;
    else if (str == "OP_BOOLAND") opcode = OP_BOOLAND;
    else if (str == "OP_BOOLOR") opcode = OP_BOOLOR;
    else if (str == "OP_NUMEQUAL") opcode = OP_NUMEQUAL;
    else if (str == "OP_NUMEQUALVERIFY") opcode = OP_NUMEQUALVERIFY;
    else if (str == "OP_NUMNOTEQUAL") opcode = OP_NUMNOTEQUAL;
    else if (str == "OP_LESSTHAN") opcode = OP_LESSTHAN;
    else if (str == "OP_GREATERTHAN") opcode = OP_GREATERTHAN;
    else if (str == "OP_LESSTHANOREQUAL") opcode = OP_LESSTHANOREQUAL;
    else if (str == "OP_GREATERTHANOREQUAL") opcode = OP_GREATERTHANOREQUAL;
    else if (str == "OP_MIN") opcode = OP_MIN;
    else if (str == "OP_MAX") opcode = OP_MAX;
    else if (str == "OP_WITHIN") opcode = OP_WITHIN;
    else if (str == "OP_RIPEMD160") opcode = OP_RIPEMD160;
    else if (str == "OP_SHA1") opcode = OP_SHA1;
    else if (str == "OP_SHA256") opcode = OP_SHA256;
    else if (str == "OP_HASH160") opcode = OP_HASH160;
    else if (str == "OP_HASH256") opcode = OP_HASH256;
    else if (str == "OP_CODESEPARATOR") opcode = OP_CODESEPARATOR;
    else if (str == "OP_CHECKSIG") opcode = OP_CHECKSIG;
    else if (str == "OP_CHECKSIGVERIFY") opcode = OP_CHECKSIGVERIFY;
    else if (str == "OP_CHECKMULTISIG") opcode = OP_CHECKMULTISIG;
    else if (str == "OP_CHECKMULTISIGVERIFY") opcode = OP_CHECKMULTISIGVERIFY;
    else if (str == "OP_NOP1") opcode = OP_NOP1;
    else if (str == "OP_CHECKLOCKTIMEVERIFY") opcode = OP_CHECKLOCKTIMEVERIFY;
    else if (str == "OP_NOP2") opcode = OP_CHECKLOCKTIMEVERIFY;
    else if (str == "OP_CHECKSEQUENCEVERIFY") opcode = OP_CHECKSEQUENCEVERIFY;
    else if (str == "OP_NOP3") opcode = OP_CHECKSEQUENCEVERIFY;
    else if (str == "OP_NOP4") opcode = OP_NOP4;
    else if (str == "OP_NOP5") opcode = OP_NOP5;
    else if (str == "OP_NOP6") opcode = OP_NOP6;
    else if (str == "OP_NOP7") opcode = OP_NOP7;
    else if (str == "OP_NOP8") opcode = OP_NOP8;
    else if (str == "OP_NOP9") opcode = OP_NOP9;
    else if (str == "OP_NOP10") opcode = OP_NOP10;
    else {
        opcode = OP_INVALIDOPCODE;
        return false;
    }

    return true;
}

unsigned int CScript::GetSigOpCount(bool fAccurate) const
{
    unsigned int n = 0;
    const_iterator pc = begin();
    opcodetype lastOpcode = OP_INVALIDOPCODE;
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            break;
        if (opcode == OP_CHECKSIG || opcode == OP_CHECKSIGVERIFY)
            n++;
        else if (opcode == OP_CHECKMULTISIG || opcode == OP_CHECKMULTISIGVERIFY)
        {
            if (fAccurate && lastOpcode >= OP_1 && lastOpcode <= OP_16)
                n += DecodeOP_N(lastOpcode);
            else
                n += MAX_PUBKEYS_PER_MULTISIG;
        }
        lastOpcode = opcode;
    }
    return n;
}

unsigned int CScript::GetSigOpCount(const CScript& scriptSig) const
{
    if (!IsPayToScriptHash())
        return GetSigOpCount(true);

    // This is a pay-to-script-hash scriptPubKey;
    // get the last item that the scriptSig
    // pushes onto the stack:
    const_iterator pc = scriptSig.begin();
    vector<unsigned char> data;
    while (pc < scriptSig.end())
    {
        opcodetype opcode;
        if (!scriptSig.GetOp(pc, opcode, data))
            return 0;
        if (opcode > OP_16)
            return 0;
    }

    /// ... and return its opcount:
    CScript subscript(data.begin(), data.end());
    return subscript.GetSigOpCount(true);
}

bool CScript::IsNormalPaymentScript() const
{
    if(this->size() != 25) return false;

    std::string str;
    opcodetype opcode;
    const_iterator pc = begin();
    int i = 0;
    while (pc < end())
    {
        GetOp(pc, opcode);

        if(     i == 0 && opcode != OP_DUP) return false;
        else if(i == 1 && opcode != OP_HASH160) return false;
        else if(i == 3 && opcode != OP_EQUALVERIFY) return false;
        else if(i == 4 && opcode != OP_CHECKSIG) return false;
        else if(i == 5) return false;

        i++;
    }

    return true;
}

bool CScript::IsPayToScriptHash() const
{
    // Extra-fast test for pay-to-script-hash CScripts:
    return (this->size() == 23 &&
            (*this)[0] == OP_HASH160 &&
            (*this)[1] == 0x14 &&
            (*this)[22] == OP_EQUAL);
}

bool CScript::IsStakeLockScript() const
{
    static CScript stakeLockNumber = CScript() << Params().GetStakeLockSequenceNumber();

    if (this->size() < 41 || this->size() > 73)
        return false;

    if (!((*this)[0] == stakeLockNumber[0] && // 0x03 &&
          (*this)[1] == stakeLockNumber[1] && // 0x01 &&
          (*this)[2] == stakeLockNumber[2] && // 0x00 &&
          (*this)[3] == 0x40 &&
          (*this)[4] == 0xB2 &&
          (*this)[5] == 0x75))
        return false;
    
    int scriptSize = (*this)[6] + 7;

    return (*this)[scriptSize] == 0xAC;
}

bool CScript::IsPushOnly(const_iterator pc) const
{
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            return false;
        // Note that IsPushOnly() *does* consider OP_RESERVED to be a
        // push-type opcode, however execution of OP_RESERVED fails, so
        // it's not relevant to P2SH/BIP62 as the scriptSig would fail prior to
        // the P2SH special validation code being executed.
        if (opcode > OP_16)
            return false;
    }
    return true;
}

bool CScript::IsPushOnly() const
{
    return this->IsPushOnly(begin());
}

std::string CScript::ToString() const
{
    std::string str;
    opcodetype opcode;
    std::vector<unsigned char> vch;
    const_iterator pc = begin();
    while (pc < end())
    {
        if (!str.empty())
            str += " ";
        if (!GetOp(pc, opcode, vch))
        {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4)
            str += ValueString(vch);
        else
            str += GetOpName(opcode);
    }
    return str;
}