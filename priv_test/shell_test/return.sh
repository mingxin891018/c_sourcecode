#/bin/bash 
tmp=$(ps | grep "PID")
echo ${tmp}

tcpdump -i eth10  -s0 -w ./ddd/cap
echo $?












