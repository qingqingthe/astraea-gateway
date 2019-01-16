# build docker images
docker build -t silicon_base -f Dockerfile_base .
docker build -t silicon_gateway -f Dockerfile_gateway .
docker build -t silicon_logger -f Dockerfile_logger .
docker build -t silicon_astraea -f Dockerfile_astraea .
docker build -t silicon_database -f Dockerfile_database .
docker build -t silicon_client -f Dockerfile_client .

# create network
docker network create my_network

# run every image in a new tab and connect to network
ttab -t "gateway" 'docker run --name gateway -p 12345:12345 --net my_network silicon_gateway'
ttab -t "astraea" 'docker run --name astraea --net my_network silicon_astraea'
ttab -t "database" 'docker run --name database --net my_network silicon_database'
ttab -t "logger" 'docker run --name logger --net my_network silicon_logger'
ttab -t "client" 'docker run --name client -p 12346:12346 --net my_network silicon_client'

# wait for containers to run
sleep 10s

# send example read request to gateway
curl "127.0.0.1:12346/hello_world"
#curl -X POST "127.0.0.1:12346/send_read_requests"
