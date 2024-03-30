TAG=jcreed/libtwelf

docker:
	docker build . -t $(TAG)
bash:
	docker run --rm -it $(TAG)

dist/twelf.a: Dockerfile
	mkdir -p dist
	docker create --name tempcont "$(TAG):latest"
	docker cp "tempcont:/build/twelf/bin/twelf-server.a" dist/twelf.a
	docker cp "tempcont:/build/twelf/bin/twelf_server.h" dist/twelf.h
	touch dist/twelf.a
	touch dist/twelf.h
	docker rm tempcont
