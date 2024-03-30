TAG=jcreed/libtwelf

docker:
	docker build . -t $(TAG)
bash:
	docker run --rm -it $(TAG)

dist/libtwelf.a dist/libtwelf.h dist/libgmp.a: Dockerfile
	mkdir -p dist
	docker create --name tempcont "$(TAG):latest"
	docker cp "tempcont:/build/gmp-m68k-INSTALL/lib/libgmp.a" dist/libgmp.a
	docker cp "tempcont:/build/twelf/bin/twelf-server.a" dist/libtwelf.a
	docker cp "tempcont:/build/twelf/bin/twelf_server.h" dist/libtwelf.h
	touch dist/libgmp.a
	touch dist/libtwelf.a
	touch dist/libtwelf.h
	docker rm tempcont
