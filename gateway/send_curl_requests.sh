max=100
read=false
write=false

echo "start parsing"
# parse read and write flags
while [ "$1" != "" ]; do
    case $1 in
        -m | -max) shift
                   max=$1
                   ;;
        -r )       read=true
                   ;;
        -w )       write=true
                   ;;
        * )        exit 1
    esac
    shift
done

echo "max $max read $read write $write"

if [ $read == true ] && [ $write == true ]
then
    echo "send read and write requests"
    curl -X POST "127.0.0.1:12346/send_read_and_write_requests?max=$max"
fi
if [ $read == true ] && [ $write == false ]
then
    echo "send read requests"
    curl -X POST "127.0.0.1:12346/send_read_requests?max=$max"
fi
if [ $read == false ] && [ $write == true ]
then
    echo "send write requests"
    curl -X POST "127.0.0.1:12346/send_write_requests?max=$max"
fi

sleep 10s
curl -X POST "127.0.0.1:12346/write_results"