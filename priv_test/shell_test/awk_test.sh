#/bin/bash
test=$(ifconfig  | awk -F ' ' '$1=="eth0" {print $5}')
name=$(ifconfig  | awk -F ' ' '$1=="eth0" {print $5}' | awk '{print $1}')

echo test=${test}
echo name=${name}


