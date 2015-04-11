#!/bin/sh

if [ $# -lt 6 ] ; then
    echo "./go_main_release_dynamic_runner.sh begin end blackparamfile whiteparamfile stdout stderr (executer=./go_main_release) (boardsize=9)"
    exit 1
fi

begin=${1}
end=${2}
blackparam=${3}
whiteparam=${4}
stdout_file=${5}
stderr_file=${6}
executer=./go_main_release
if [ $# -ge 7 ] ; then
    executer=${7}
fi
boardsize=9
if [ $# -ge 8 ] ; then
    boardsize=${8}
fi

echo "begin:${begin} end:${end}"
echo "black:${blackparam} white:${whiteparam}"
echo "stdout_file:${stdout_file}"
echo "stderr_file:${stderr_file}"
echo "executer:${executer}"

i=${begin}
while [ $i -le ${end} ] ; do
    ${executer} -g 1 --blackparamfile ${blackparam} --whiteparamfile ${whiteparam} -r ${i} -b ${boardsize}
    i=`expr $i + 1`
done > ${stdout_file} 2>${stderr_file}

# index=1; c=0.3; alpha=0.00000001; lambda=0.00000001; i=`expr ${index} \* 50 - 49`; end=`expr ${i} + 49`; echo begin ${i}; echo end ${end}; while [ $i -le ${end} ] ; do ./go_main_release -g 1 --blackparamfile parameter_files/for_tuning/softmax_fullfeatures_c${c}_alpha${alpha}_lambda${lambda}_pl5k_withbook.txt --whiteparamfile parameter_files/for_tuning/softmax_fullfeatures_c0.3_pl5k_withbook.txt -r ${i}; i=`expr ${i} + 1`; done > tmp_result/dynamic_c${c}_alpha${alpha}_lambda${lambda}_vs_softmax_c0.3_pl5k_withbook_${index} 2>tmp/dynamic_c${c}_alpha${alpha}_lambda${lambda}_vs_softmax_c0.3_pl5k_withbook_${index}