
# Got this code from bitcoin utilities
# https://github.com/grondilu/bitcoin-bash-tools/blob/master/bitcoin.sh

declare -a base58=(
      1 2 3 4 5 6 7 8 9
    A B C D E F G H   J K L M N   P Q R S T U V W X Y Z
    a b c d e f g h i j k   m n o p q r s t u v w x y z
)
unset dcr; for i in {0..57}; do dcr+="${i}s${base58[i]}"; done
declare ec_dc='
I16i7sb0sa[[_1*lm1-*lm%q]Std0>tlm%Lts#]s%[Smddl%x-lm/rl%xLms#]s~
483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
2 100^d14551231950B75FC4402DA1732FC9BEBF-so1000003D1-ddspsm*+sGi
[_1*l%x]s_[+l%x]s+[*l%x]s*[-l%x]s-[l%xsclmsd1su0sv0sr1st[q]SQ[lc
0=Qldlcl~xlcsdscsqlrlqlu*-ltlqlv*-lulvstsrsvsulXx]dSXxLXs#LQs#lr
l%x]sI[lpSm[+q]S0d0=0lpl~xsydsxd*3*lal+x2ly*lIx*l%xdsld*2lx*l-xd
lxrl-xlll*xlyl-xrlp*+Lms#L0s#]sD[lpSm[+q]S0[2;AlDxq]Sdd0=0rd0=0d
2:Alp~1:A0:Ad2:Blp~1:B0:B2;A2;B=d[0q]Sx2;A0;B1;Bl_xrlm*+=x0;A0;B
l-xlIxdsi1;A1;Bl-xl*xdsld*0;Al-x0;Bl-xd0;Arl-xlll*x1;Al-xrlp*+L0
s#Lds#Lxs#Lms#]sA[rs.0r[rl.lAxr]SP[q]sQ[d0!<Qd2%1=P2/l.lDxs.lLx]
dSLxs#LPs#LQs#]sM[lpd1+4/r|]sR
';

decodeBase58() {
    local line
    echo -n "$1" | sed -e's/^\(1*\).*/\1/' -e's/1/00/g' | tr -d '\n'
    dc -e "$dcr 16o0$(sed 's/./ 58*l&+/g' <<<$1)p" |
    while read line; do echo -n ${line/\\/}; done
}
encodeBase58() {
    local n
    echo -n "$1" | sed -e's/^\(\(00\)*\).*/\1/' -e's/00/1/g' | tr -d '\n'
    dc -e "16i ${1^^} [3A ~r d0<x]dsxx +f" |
    while read -r n; do echo -n "${base58[n]}"; done
}


if [ $# -lt 1 ]
then
  echo "need at least the networkd (mainnet or testnet)"
  echo "base on this: https://gist.github.com/t4sk/ac6f2d607c96156ca15f577290716fcc"
  echo "test here: http://gobittest.appspot.com/PrivateKey"
exit 1
fi

network=$1
if [ "$network" != "testnet" && "$network" != "mainnet" ]
then
echo "network needs to be mainnet or testnet"
exit 1
fi

if [ $# -eq 3 ]
then
  control_wallet_user=$2
  control_wallet_password=$3
fi

if [ "$network" = "mainnet" ]
then
  # ver=80 -> base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
  # needs to be in hex
  ver=80
else
  #ver=E9  -> base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,233);
  ver=E9
fi


dir=`pwd`
if [ $# -eq 3 ]
then
cli_args="-$network -debug -rpcuser=$control_wallet_user -rpcpassword=$control_wallet_password"
else 
cli_args="-$network -debug"
fi

cli="$dir/kore-cli $cli_args"
echo "Getting the account address : $cli"
address=`$cli getaccountaddress spork`
echo "Generate Private Key for the Address: $address"
privateKey=`$cli dumpprivkey $address`
echo "Private Key = $privateKey"
privateKey=`decodeBase58 $privateKey`
#echo "Private Key = $privateKey"
#privateKey=619c335025c7f4012e556c2a58b2506e30b8511b53ade95ea316fd8c3286feb9
privateKey=00e6742f82d5c78c076ee9c3620535e14dc605bd3a2b599edd29edaa4e0a789e1f

#  Generating key from here, works, however it gives problem when importing the 
#  generated WIF:  kore-cli importprivkey 92KuV1Mtf9jTttTrw1yawobsa9uCZGbfpambH8H1Y7KfdDxxc4d "test-priv-key"
#   openssl ecparam -genkey -name secp256k1 -out sporkkey.pem
#   openssl ec -in sporkkey.pem -text > sporkkey.hex

echo "Let's convert private key to WIF"
doubleSHA256=`echo $ver$privateKey -n | xxd -r -p | openssl dgst -sha256 -binary | openssl dgst -sha256 -c -hex`
echo "Double Sha256 result"
echo "$doubleSHA256"
IFS=': ' read -r -a array <<< "$doubleSHA256"
checksum="${array[1]}${array[2]}${array[3]}${array[4]}"
echo "Getting the first 4 bytes as checksum"
echo "CHECKSUM: $checksum"
result="$ver$privateKey$checksum"
echo "Append Checksum: $result"
echo `encodeBase58 $result`

