# clear data folder
rm -r data
mkdir data

# send some requests
./send_curl_requests.sh -r -m 10
./send_curl_requests.sh -r -m 20
./send_curl_requests.sh -r -m 30
./send_curl_requests.sh -r -m 40
./send_curl_requests.sh -r -m 50
./send_curl_requests.sh -r -m 60
./send_curl_requests.sh -r -m 70
./send_curl_requests.sh -r -m 80
./send_curl_requests.sh -r -m 90
./send_curl_requests.sh -w -m 100
./send_curl_requests.sh -w -m 20
./send_curl_requests.sh -w -m 30
./send_curl_requests.sh -w -m 40
./send_curl_requests.sh -w -m 50
./send_curl_requests.sh -w -m 60
./send_curl_requests.sh -w -m 50
./send_curl_requests.sh -w -m 70
./send_curl_requests.sh -w -m 80
./send_curl_requests.sh -w -m 90
./send_curl_requests.sh -w -m 100
./send_curl_requests.sh -r -w -m 10
./send_curl_requests.sh -r -w -m 20
./send_curl_requests.sh -r -w -m 30
./send_curl_requests.sh -r -w -m 40
./send_curl_requests.sh -r -w -m 50
./send_curl_requests.sh -r -w -m 60
./send_curl_requests.sh -r -w -m 70
./send_curl_requests.sh -r -w -m 80
./send_curl_requests.sh -r -w -m 90
./send_curl_requests.sh -r -w -m 100

# fetch data from client + compute summary
./get_data_files_from_client.sh