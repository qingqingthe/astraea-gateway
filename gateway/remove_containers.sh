docker kill gateway
docker kill astraea
docker kill database
docker kill logger
docker kill client

docker rm gateway
docker rm astraea
docker rm database
docker rm logger
docker rm client

docker network rm my_network