Twelf.bin: Dockerfile twelf-app/*.c twelf-app/*.r twelf-app/Makefile twelf-app/icon.r
	docker build . -t "jcreed/retro68:latest"
	docker run --rm -v "./twelf-app:/root/app" "jcreed/retro68:latest" /root/build.sh
	cp twelf-app/Twelf.bin Twelf.bin
	touch Twelf.bin

twelf-app/icon.r: ../scripts/*.js ../assets/*.png
	convert  ../assets/twelf-icon.png -compress none /tmp/twelf-icon.ppm
	convert  ../assets/twelf-icon-small.png -compress none /tmp/twelf-icon-small.ppm
	convert  ../assets/twelf-doc.png -compress none /tmp/twelf-doc.ppm
	convert  ../assets/twelf-doc-small.png -compress none /tmp/twelf-doc-small.ppm
	node ../scripts/make-icon-resources.js >twelf-app/icon.r

TESample.bin: Dockerfile tesample-app/*.c tesample-app/*.r tesample-app/Makefile
	docker build . -t "jcreed/retro68:latest"
	docker run --rm -v "./tesample-app:/root/app" "jcreed/retro68:latest" /root/build.sh
	cp tesample-app/TESample.bin TESample.bin
	touch TESample.bin

docker:
	docker build . -t jcreed/retro68

bash:
	docker run --rm -v "./tesample-app:/root/app" -it jcreed/retro68

# local server that I can access from http://10.0.2.2:8000 on the client to
# download Twelf.bin and put it through stuffit
serve:
	python3 ../server/server.py
