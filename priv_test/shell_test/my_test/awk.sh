#!/bin/bash
input_file="/home/zhaomingxin/small_system/tm_8v8/platform/on-project/build/build_id.mk"
hotel_version=$(awk -F:=  -v val="HOTEL_VERSION " '$1==val {print val $2}' "${input_file}" |awk '{print $2}')

echo $hotel_version







