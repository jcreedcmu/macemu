app.bin: Dockerfile app/*.c app/*.r app/CMakeLists.txt app/icon.r
	docker build . -t "jcreed/retro68:latest"
	docker run --rm -v "./app:/root/app" -v "./../libtwelf/dist:/root/libtwelf" "jcreed/retro68:latest" /root/build.sh
	cp app/build/Twelf.bin app.bin
	touch app.bin
# node ../scripts/set-bits.js app.bin

app/icon.r: ../scripts/*.js ../assets/*.png
	convert  ../assets/twelf-icon.png -compress none /tmp/twelf-icon.ppm
	convert  ../assets/twelf-icon-small.png -compress none /tmp/twelf-icon-small.ppm
	convert  ../assets/twelf-doc.png -compress none /tmp/twelf-doc.ppm
	convert  ../assets/twelf-doc-small.png -compress none /tmp/twelf-doc-small.ppm
	node ../scripts/make-icon-resources.js >app/icon.r

docker:
	docker build . -t jcreed/retro68
bash:
	docker run --rm -v "./app:/root/app" -v "../libtwelf/dist:/root/libtwelf" -it jcreed/retro68

# local server that I can access from http://10.0.2.2:8000 on the client to
# download app.bin and put it through stuffit
serve:
	python3 ../server/server.py
