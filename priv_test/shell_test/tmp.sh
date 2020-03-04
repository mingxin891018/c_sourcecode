#/bin/bash
test=$(df | grep "tmp" | awk -F ' ' '$1=="tmpfs" {print $6}')
name=$(ifconfig | awk -F ' ' '$1=="eth0" {print $5}' | awk '{print $1}')
name=$(ifconfig | awk -F ' ' '$1=="eth0" {print $1,$3,$5}' | awk '{print $2}')
mountdir=$(df | grep "tmp" | awk -F ' ' '{print $6}')

echo test=${test}
echo name=${name}
echo mountdir=${mountdir}


