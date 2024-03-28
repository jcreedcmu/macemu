LaunchAPPLServer.bin: Dockerfile
	docker rm tempcont
	docker build . -t "jcreed/launchappl:latest"
	docker run --name tempcont "jcreed/launchappl:latest" /root/build.sh
	docker cp "tempcont:/root/app/build/Server/LaunchAPPLServer.bin" LaunchAPPLServer.bin
	touch LaunchAPPLServer.bin
	docker rm tempcont

bash:
	docker run --rm -it jcreed/launchappl

build: Dockerfile
	docker build . -t "jcreed/launchappl:latest"
