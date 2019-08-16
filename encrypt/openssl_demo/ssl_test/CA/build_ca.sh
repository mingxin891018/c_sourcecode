#!/bin/bash
rm -rf sign.crt  ssl.csr  ssl.key
openssl genrsa -out ssl.key 2048
openssl req -new -key ssl.key -subj "/CN=test/OU=t1/O=sunniwell/L=sz/ST=gd/C=cn" -out ssl.csr
openssl x509 -req -in ssl.csr -extensions v3_ca -signkey ssl.key -out sign.crt -days 365


#-subj '/C=US/ST=California/L=Mountain View/O=Android/OU=Android/CN=Android/emailAddress=android@android.com'
#查看证书信息
openssl x509 -in sign.crt -noout -serial -subject
