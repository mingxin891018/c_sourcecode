#/bin/bash
test=$(ifconfig  | awk -F ' ' '$1=="eth0" {print $5}')
name=$(ifconfig  | awk -F ' ' '$1=="eth0" {print $5}' | awk '{print $1}')
name=$(ifconfig  | awk -F ' ' '$1=="eth0" {print $1,$3,$5}' | awk '{print $2}')

echo test=${test}
echo name=${name}


