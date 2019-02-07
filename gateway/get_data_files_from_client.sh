# script to compute summary of client data for graphs

# get data from client container
docker cp client:/data/ ./

# create file to write to
summary_filename="./data/summary.csv";
touch $summary_filename
> "data/summary.csv"
echo "requests;r;w;rw" >> "data/summary.csv"

# iterate through all files
for filename in ./data/*; do

    # do not parse summary file
    if [ $filename == $summary_filename ];
    then
        continue
    fi

    # set read and write flags
    echo $filename
    read=false
    write=false
    if [[ $filename == *"_r_"* ]]; then read=true; fi
    if [[ $filename == *"_w_"* ]]; then write=true; fi

    total=0
    count=0

    # iterate through all lines
    while IFS= read -r line; do
       # skip missing responses
       if [[ $line == "missing response"* ]];
       then
        echo "found missing response"
        continue
       fi

       # sum durations
       total=$(echo $total+$(cut -d';' -f2 <<<"$line") | bc )
       ((count++))
    done < $filename

    # compute average
    if [ $count > 0 ]; then
        average=$(echo "scale=2; $total / $count" | bc)
        echo $average

        # get number of requests from filename
        max=$(echo $filename | cut -d'_' -f2)

        # search in summary for requestNumber
        lineNumber=0
        newLine=""
        while IFS= read -r line; do
            if [ $(echo $line | cut -d';' -f1) == $max ]; then
                # found line in which the value must be inserted (depends on read and write flag)
                newLine=$(echo $line | cut -d';' -f1)
                oldR=$(echo $line | cut -d';' -f2)
                oldW=$(echo $line | cut -d';' -f3)
                oldRW=$(echo $line | cut -d';' -f4)
                echo "$reads $writes $readWrites"
                echo "$oldR $oldW $oldRW"
                if ($read) && !($write); then newLine="$newLine;$average;$oldW;$oldRW"; fi
                if !($read) && ($write); then newLine="$newLine;$oldR;$average;$oldRW"; fi
                if ($read) && ($write) then newLine="$newLine;$oldR;$oldW;$average"; fi

                # write new line to file
                echo "write $newLine to lineNumber $lineNumber"
                echo $line
                sed -i '' "s|$line|$newLine|g" $summary_filename
                break
            fi
            ((lineNumber++))
        done < $summary_filename

        # if $newLine is empty append a new one
        echo "newLine: $newLine"
        if [ -z $newLine ]; then
            newLine="$max"
            if ($read) && !($write); then newLine="$newLine;$average;0;0"; fi
            if !($read) && ($write); then newLine="$newLine;0;$writes;0"; fi
            if ($read) && ($write); then newLine="$newLine;0;0;$average"; fi
            echo "$newLine" >> "data/summary.csv"
            echo "add $newLine"
        fi
    else
        echo "file was empty"
    fi

    cat $summary_filename
done