app.bin: Dockerfile app/*.c app/*.r app/CMakeLists.txt
	docker build . -t "jcreed/retro68:latest"
	docker create --name tempcont "jcreed/retro68:latest"
	docker cp "tempcont:/root/app/build/Twelf.bin" app.bin
	touch app.bin
	docker rm tempcont

docker:
	docker build . -t jcreed/retro68
bash:
	docker run --rm -it jcreed/retro68
bashv:
	docker run --rm -it -v "/home/jcreed/pgit/Retro68:/root" jcreed/retro68

# local server that I can access from http://10.0.2.2:8000 on the client to
# download app.bin and put it through stuffit
serve:
	python3 -m http.server
