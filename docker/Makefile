app.bin: Dockerfile app/*.c app/*.r app/CMakeLists.txt
	docker build . -t "jcreed/retro68:latest"
	docker create --name tempcont "jcreed/retro68:latest"
	docker cp "tempcont:/root/app/build/Logic.bin" app.bin
	touch app.bin
	docker rm tempcont

docker:
	docker build . -t jcreed/retro68
bash:
	docker run --rm -it jcreed/retro68
bashv:
	docker run --rm -it -v "/home/jcreed/pgit/Retro68:/root" jcreed/retro68


serve:
	python3 -m http.server
