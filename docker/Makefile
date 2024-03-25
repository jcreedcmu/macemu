app.bin: Dockerfile app/*.c app/*.r app/CMakeLists.txt app/icon.r
	docker build . -t "jcreed/retro68:latest"
	docker create --name tempcont "jcreed/retro68:latest"
	docker cp "tempcont:/root/app/build/Twelf.bin" app.bin
	touch app.bin
	docker rm tempcont

app/icon.r: ../scripts/*.js ../assets/twelf-icon.png
	convert  ../assets/twelf-icon.png -compress none /tmp/twelf-icon.ppm
	convert  ../assets/twelf-icon-small.png -compress none /tmp/twelf-icon-small.ppm
	node ../scripts/make-icon-resources.js >app/icon.r

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
