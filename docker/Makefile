docker:
	docker build . -t jcreed/retro68
bash:
	docker run --rm -it jcreed/retro68
bashv:
	docker run --rm -it -v "/home/jcreed/pgit/Retro68:/root" jcreed/retro68

app.bin: Dockerfile
	docker build . -t "jcreed/retro68:latest"
	docker create --name tempcont "jcreed/retro68:latest"
	docker cp "tempcont:/root/app/build/Dialog.bin" app.bin
	touch app.bin
	docker rm tempcont

serve:
	python3 -m http.server
