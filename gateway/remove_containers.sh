docker kill gateway
docker kill logger
docker kill astraea
docker kill database
docker kill client

docker rm gateway
docker rm logger
docker rm astraea
docker rm database
docker rm client

docker network rm my_network