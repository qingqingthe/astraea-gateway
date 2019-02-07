-----------------------
	Gateway
-----------------------

In this folder you find a couple of classes, dockerfiles and scripts.
In the following I will explain how to use the scripts.

-- run_docker_container.sh
this will build, run and connect the 5 docker container (gateway, astraea, database, logger and client) with each other win the docker network my_network.

-- remove_containers.sh
this will kill and remove all 5 containers and the network.

-- send_curl_requests.sh
with this script you may send requests to the gateway via the client. The client also receives the responses.
You can call the script with some params: -r for reading requests, -w for writing requests, both for both kind of requests simultaneously and -m or -max with a number for the maximum number of requests.