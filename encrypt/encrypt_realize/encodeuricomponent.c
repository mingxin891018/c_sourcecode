#include <stdio.h>
#include <string.h>

#define BURSIZE 2048

int hex2dec(char c)
{
    if ('0' <= c && c <= '9') 
    {
        return c - '0';
    } 
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    } 
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    } 
    else 
    {
        return -1;
    }
}

char dec2hex(short int c)
{
    if (0 <= c && c <= 9) 
    {
        return c + '0';
    } 
    else if (10 <= c && c <= 15) 
    {
        return c + 'A' - 10;
    } 
    else 
    {
        return -1;
    }
}


//编码一个url
void urlencode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (    ('0' <= c && c <= '9') ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') || 
                c == '/' || c == '.') 
        {
            res[res_len++] = c;
        } 
        else 
        {
            int j = (short int)c;
            if (j < 0)
                j += 256;
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            res[res_len++] = '%';
            res[res_len++] = dec2hex(i1);
            res[res_len++] = dec2hex(i0);
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}

// 解码url
void urldecode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (c != '%') 
        {
            res[res_len++] = c;
        }
        else 
        {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}

int main(int argc, char *argv[])
{
    char url[4096] = "{\"VER\":\"01\",\"CTEI\":\"CT1234561234567\",\"MAC\":\"24:e2:71:f4:d7:b0\",\"IP\":\"192.168.0.1\",\"UPLINKMAC\":\"01:02:03:04:05:06\",\"LINK\":\"1\",\"FWVER\":\"1.1\",\"DATE\":\"2018-07-01 12:05:30\",\"UID\":\"3HKB01232422LVM\",\"SN\":\"1183379001578\",\"REFRESHSTATE\":\"1\"}";
    urlencode(url); //编码后
    printf("http://'测试/@mike  ----> %s\n", url);

    char buf[100] = "http%3A//%27%E6%B5%8B%E8%AF%95/%40mike";
    urldecode(buf); //解码后
    printf("http%%3A//%%27%%E6%%B5%%8B%%E8%%AF%%95/%%40mike  ----> %s\n", buf);

    return 0;
}