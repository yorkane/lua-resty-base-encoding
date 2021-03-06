/**
 * \file modp_b64w.c
 * <PRE>
 * MODP_B64 - High performance base64 encoder/decoder
 * https://github.com/client9/stringencoders
 *
 * Copyright &copy; 2005-2016 Nick Galbreath
 * Modified by spacewander
 * All rights reserved.
 * Released under MIT license. See LICENSE for details.
 * </PRE>
 */

/*
 * If you are ripping this out of the library, comment out the next
 * line and uncomment the next lines as approrpiate
 */
//#include "config.h"

/* public header */
#include "modp_stdint.h"
#include "math.h"
#include <stdio.h>
#include "xxhash.h"
size_t modp_b64w_encode(char *dest, unsigned char *str, size_t len); //refer to modp_b64w.c

static char base64_table[65] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
                                'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
                                'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', '-', '_', '='};

static const int b64map[131] = {
    /*0=*/99,
    /*1=*/ 99, /*2=*/ 99, /*3=*/ 99, /*4=*/ 99, /*5=*/ 99, /*6=*/ 99, /*7=*/ 99, /*8=*/ 99, /*9=	*/ 99, /*10=*/99,
    /*11=*/ 99, /*12=*/ 99, /*13=*/99, /*14=*/ 99, /*15=*/ 99, /*16=*/ 99, /*17=*/ 99, /*18=*/ 99, /*19=*/ 99, /*20=*/ 99,
    /*21=*/ 99, /*22=*/ 99, /*23=*/ 99, /*24=*/ 99, /*25=*/ 99, /*26=*/ 99, /*27=*/ 99, /*28=*/ 99, /*29=*/ 99, /*30=*/ 99,
    /*31=*/ 99, /*32= */ 99, /*33=!*/ 99, /*34="*/ 99, /*35=#*/ 99, /*36=$*/ 99, /*37=%*/ 99, /*38=&*/ 99, /*39='*/ 99, /*40=(*/ 99,
    /*41=)*/ 99, /*42=**/ 99, /*43=+*/ 99, /*44=,*/ 99, /*45=-*/ 62, /*46=.*/ 99, /*47=/*/ 99, /*48=0*/ 52, /*49=1*/ 53, /*50=2*/ 54,
    /*51=3*/ 55, /*52=4*/ 56, /*53=5*/ 57, /*54=6*/ 58, /*55=7*/ 59, /*56=8*/ 60, /*57=9*/ 61, /*58=:*/ 99, /*59=;*/ 99, /*60=<*/ 99,
    /*61==*/99, /*62=>*/ 99, /*63=?*/ 99, /*64=@*/ 99, /*65=A*/ 0, /*66=B*/ 1, /*67=C*/ 2, /*68=D*/ 3, /*69=E*/ 4, /*70=F*/ 5,
    /*71=G*/ 6, /*72=H*/ 7, /*73=I*/ 8, /*74=J*/ 9, /*75=K*/ 10, /*76=L*/ 11, /*77=M*/ 12, /*78=N*/ 13, /*79=O*/ 14, /*80=P*/ 15,
    /*81=Q*/ 16, /*82=R*/ 17, /*83=S*/ 18, /*84=T*/ 19, /*85=U*/ 20, /*86=V*/ 21, /*87=W*/ 22, /*88=X*/ 23, /*89=Y*/ 24, /*90=Z*/ 25,
    /*91=[*/ 99, /*92=\*/ 99, /*93=]*/ 99, /*94=^*/ 99, /*95=_*/ 63, /*96=`*/ 99, /*97=a*/ 26, /*98=b*/ 27, /*99=c*/ 28, /*100=d*/ 29,
    /*101=e*/ 30, /*102=f*/ 31, /*103=g*/ 32, /*104=h*/ 33, /*105=i*/ 34, /*106=j*/ 35, /*107=k*/ 36, /*108=l*/ 37, /*109=m*/ 38, /*110=n*/ 39,
    /*111=o*/ 40, /*112=p*/ 41, /*113=q*/ 42, /*114=r*/ 43, /*115=s*/ 44, /*116=t*/ 45, /*117=u*/ 46, /*118=v*/ 47, /*119=w*/ 48, /*120=x*/ 49,
    /*121=y*/ 50, /*122=z*/ 51, /*123={*/ 99, /*124=|*/ 99, /*125=}*/ 99, /*126=~*/ 99, /*127=*/ 99, /*128=€*/ 99, /*129=*/ 99, /*130=‚*/ 99};

static long pow64[12] = {1, 64, 4096, 262144, 16777216, 1073741824, 68719476736, 4398046511104, 281474976710656, 18014398509481984, 1152921504606846976};

unsigned long base64_long(const char *dst, int len)
{
    int i;
    int k;
    unsigned long num = 0;
    // printf("[%s] C=%d\n", dst, dst[3]);
    // for (i = 1; i < 127; i++)
    // {
    //     printf("%d %c \n", i, i); //print ASCLL table
    // }
    // printf("%d %d %d %d \n", dst[0], dst[1], dst[2], dst[3]);
    // printf("%c %c %c %c \n", dst[0], dst[1], dst[2], dst[3]);
    for (i = len; i >= 0; i--)
    {
        k = dst[i];
        int n = b64map[k];
        // printf("Start with n:   %d, index = %d\n", n, k);
        if (n < 99)
        {
            num = num + (n * pow64[len - i]);
            // printf("[%d]    %c=%d=%d	Pow=%ld\t\ttotal %ld\n", i, k, n, base64_table[n - 1], pow64[len - i], num);
        }
    }
    // printf("Total = %ld \n", num);
    return num;
}

unsigned int base64_int(const char *dst, int len)
{
    int i;
    int k;
    unsigned int num = 0;
    for (i = len; i >= 0; i--)
    {
        k = dst[i];
        unsigned int n = b64map[k];
        if (n < 99)
        {
            num = num + (n * pow64[len - i]);
        }
    }
    return num;
}

size_t int_base64(char *dst, unsigned int Value10)
{
    int size = 0;
    while (Value10 > 0)
    {
        int v = Value10 % 64;
        dst[size] = base64_table[v];
        Value10 /= 64;
        size++;
    }
    int i = 0;
    int temp = 0;
    for (i = 0; i < (size / 2); i++)
    {
        temp = dst[i];
        dst[i] = dst[size - i - 1];
        dst[size - i - 1] = temp;
    }
    return (size_t)size;
}

size_t long_base64(char *dst, long Value10)
{
    int size = 0;
    while (Value10 > 0)
    {
        long v = fmod(Value10, 64);
        // printf("%ld mod(64) = %ld \n", Value10, v);
        dst[size] = base64_table[v];
        Value10 /= 64;
        size++;
    }
    // int i = 0;
    // int temp = 0;
    // for (i = 0; i < (size / 2); i++)
    // {
    //     temp = dst[i];
    //     dst[i] = dst[size - i - 1];
    //     dst[size - i - 1] = temp;
    // }
    return (size_t)size;
}

unsigned int xxhash32(const char *str, size_t length, unsigned int const seed)
{
    unsigned int const hash = XXH32(str, length, seed);
    return hash;
}

unsigned long long xxhash64(const char *str, size_t length, unsigned long long const seed)
{
    unsigned long long const hash = XXH64(str, length, seed);
    return hash;
}

unsigned long djb2_hash(const char *str)
{
    unsigned long hash = 5381;
    int c = 0;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}


size_t xxhash64_b64(char *dest, const char *str, size_t length, unsigned long long const seed)
{
    unsigned long long lhash = xxhash64(str, length, seed);
    unsigned char bytes[8];
    bytes[0] = (lhash >> 56) & 0xFF;
    bytes[1] = (lhash >> 48) & 0xFF;
    bytes[2] = (lhash >> 40) & 0xFF;
    bytes[3] = (lhash >> 32) & 0xFF;
    bytes[4] = (lhash >> 24) & 0xFF;
    bytes[5] = (lhash >> 16) & 0xFF;
    bytes[6] = (lhash >> 8) & 0xFF;
    bytes[7] = lhash & 0xFF;
    return modp_b64w_encode(dest, bytes, 8);
}

size_t xxhash32_b64(char *dest, const char *str, size_t length, unsigned int const seed)
{
    unsigned int lhash = xxhash32(str, length, seed);
    unsigned char bytes[4];
    bytes[0] = (lhash >> 24) & 0xFF;
    bytes[1] = (lhash >> 16) & 0xFF;
    bytes[2] = (lhash >> 8) & 0xFF;
    bytes[3] = lhash & 0xFF;
    return modp_b64w_encode(dest, bytes, 4);
}

unsigned int get_unsigned_int(const char *buffer, int offset, int length)
{
    if (length > 3)
        return (((buffer[offset + 3]) & 0x000000FF) | ((buffer[offset + 2] << 8) & 0x0000FF00) | ((buffer[offset + 1] << 16) & 0x00FF0000) | ((buffer[offset] << 24) & 0xFF000000));
    if (length == 3)
        return (((buffer[offset + 2]) & 0x000000FF) | ((buffer[offset + 1] << 8) & 0x0000FF00) | ((buffer[offset] << 16) & 0x00FF0000));
    if (length == 2)
        return (((buffer[offset + 1]) & 0x000000FF) | ((buffer[offset] << 8) & 0x0000FF00));
    return ((buffer[offset]) & 0x000000FF);
}

unsigned int get_unsigned_int_from(int a, int b, int c, int d)
{
    return (
        (a & 0x000000FF) |
        ((b << 8) & 0x0000FF00) |
        ((c << 16) & 0x00FF0000) |
        ((d << 24) & 0xFF000000));
}

size_t get_bytes_from_unsigned_int(char *dest, unsigned int const num, int len)
{
    if (len == 4)
    {
        dest[0] = (num >> 24) & 0xFF;
        dest[1] = (num >> 16) & 0xFF;
        dest[2] = (num >> 8) & 0xFF;
        dest[3] = num & 0xFF;
    }
    else if (len == 3)
    {
        dest[0] = (num >> 16) & 0xFF;
        dest[1] = (num >> 8) & 0xFF;
        dest[2] = num & 0xFF;
    }
    else if (len == 2)
    {
        dest[0] = (num >> 8) & 0xFF;
        dest[1] = num & 0xFF;
    } else
    {
         dest[0] = num & 0xFF;
    }
    
    return len;
}