#!/bin/sh

if [ $# -lt 5 ] ; then
    echo "./go_main_release_vs_gnugo_runner.sh begin end inputfile result_file stderr_file (blackappendparam) (whiteappendparam) (blackappendname) (whiteappendname) (boardsize)"
    exit 1
fi

begin=${1}
end=${2}
inputfile=${3}
result_file=${4}
stderr_file=${5}
blackappendprefix="-r "
whiteappendprefix="-r "
if [ $# -ge 6 ] ; then
    blackappendprefix=${6}
fi
if [ $# -ge 7 ] ; then
    whiteappendprefix=${7}
fi
blackappendname=""
whiteappendname=""
if [ $# -ge 8 ] ; then
    blackappendname="--appendblackname ${8}"
fi
if [ $# -ge 9 ] ; then
    whiteappendname="--appendwhitename ${9}"
fi
boardsize="9"
if [ $# -ge 10 ] ; then
    boardsize=${10}
fi

# executer=./go_main_release
# if [ $# -ge 7 ] ; then
#     executer=${7}
# fi

echo "begin:${begin} end:${end}"
echo "result_file:${result_file}"
echo "stderr_file:${stderr_file}"
echo "blackappend:${blackappendprefix}"
echo "whiteappend:${whiteappendprefix}"
echo "blackappend:${blackappendname}"
echo "whiteappend:${whiteappendname}"
echo "boardsize:${boardsize}"


i=${begin}
while [ $i -le ${end} ] ; do
    echo "RandomSeed,${i}" >> ${result_file}
    python eval_tools/computer_battle.py --appendblackparam "${blackappendprefix} ${i}" --appendwhiteparam "${whiteappendprefix} ${i}" --file ${result_file} ${blackappendname} ${whiteappendname} --boardsize ${boardsize} < $inputfile
    i=`expr $i + 1`
done > ${stderr_file} 2>&1

#index=10; expand=50; i=`expr ${index} \* 100 - 99`; end=`expr ${i} + 99`; echo $i $end; while [ $i -le $end ] ; do python eval_tools/computer_battle.py --appendblackparam "-r $i" --appendwhiteparam "-r $i" --file tmp_result/softmax_select_by_winrate_c0.3_expand${expand}_pl5k_vs_gnugo_${index} < eval_tools/for_tuning/softmax_select_by_winrate_c_0.3_pl5k_expand50_withbook.txt; i=`expr $i + 1`; done >tmp/softmax_select_by_winrate_c0.3_expand${expand}_pl5k_vs_gnugo_${index} 2>&1

# index=1; c=0.3; alpha=0.00000001; lambda=0.00000001; i=`expr ${index} \* 50 - 49`; end=`expr ${i} + 49`; echo begin ${i}; echo end ${end}; while [ $i -le ${end} ] ; do ./go_main_release -g 1 --blackparamfile parameter_files/for_tuning/softmax_fullfeatures_c${c}_alpha${alpha}_lambda${lambda}_pl5k_withbook.txt --whiteparamfile parameter_files/for_tuning/softmax_fullfeatures_c0.3_pl5k_withbook.txt -r ${i}; i=`expr ${i} + 1`; done > tmp_result/dynamic_c${c}_alpha${alpha}_lambda${lambda}_vs_softmax_c0.3_pl5k_withbook_${index} 2>tmp/dynamic_c${c}_alpha${alpha}_lambda${lambda}_vs_softmax_c0.3_pl5k_withbook_${index}