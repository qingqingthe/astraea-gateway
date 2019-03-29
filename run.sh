received="./client/received/received.csv"
rt="/response_times.csv"
dir="./latencies"
csv="/sent.csv"
if [[ ! -f $dir$csv ]]; then
    echo "Creating $dir$csv file"
    touch $dir$csv
fi
echo -n "" > ./client/received/received.csv
echo -n "" > ./latencies/sent.csv
echo -n "" > ./gateway/data/throughput.csv
if [ ! -d $dir ]; then
    mkdir $dir
fi
it=5000
for (( i=0; i<$it; i++ ))
do
#        sleep $(echo "0.0003 * ($it - $i)" | bc)
        sleep 0.0055 
        ./send_request.sh $i &
##        if [[ $(($i % 10)) == 0 ]]; then
##            echo "$i"
##        fi
done
#trigger gateway timeout by waiting 10 secs
sleep 12
echo "done"
./send_request.sh -1
echo "done!"
while [ $(ls -l $dir | wc -l) -lt $(($it - 2)) ]
do
    echo "it is $it"
    echo "ls is $(ls -l $dir | wc -l)"
    sleep 1
done
sort_file_list=$(ls $dir | sort)
rm $dir$csv
for file in $sort_file_list; do
    if [[ "/"$file != $csv ]]; then
        echo $(cat -e $dir"/"$file) >> $dir$csv
        rm $dir"/"$file
    fi
done
sed -i 's/\$//g' $dir$csv
echo $(wc -l $received)
if [[ $(wc -l $received) -gt $it ]]; then
    sed -i '$ d' $received
fi
echo "waiting for processes to finish"
sleep 10
paste -d"," latencies/sent.csv client/received/received.csv > latencies/intervals.csv

