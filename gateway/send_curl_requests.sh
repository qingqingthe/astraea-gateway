max=100
read_=false
write=false

echo "start parsing"
# parse read_ and write flags
while [ "$1" != "" ]; do
    case $1 in
        -m | -max) shift
                   max=$1
                   ;;
        -r )       read_=true
                   ;;
        -w )       write=true
                   ;;
        * )        exit 1
    esac
    shift
done

echo "max $max read_ $read_ write $write"

if [ $read_ == true ] && [ $write == true ]
then
    echo "send read_ and write requests"
    curl -X POST "127.0.0.1:12346/send_read_and_write_requests?max=$max"
fi
if [ $read_ == true ] && [ $write == false ]
then
    echo "send read_ requests"
    curl -X POST "127.0.0.1:12346/send_read_requests?max=$max"
#   curl -X POST "127.0.0.1:12346/read_request_from_client?max=$max"

fi
if [ $read_ == false ] && [ $write == true ]
then
    echo "send write requests"
    curl -X POST "127.0.0.1:12346/send_write_requests?max=$max"
fi
