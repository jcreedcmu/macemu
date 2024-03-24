.PHONY: ../docker/app/icon.r
../docker/app/icon.r:
	convert  ../assets/twelf-icon.png -compress none /tmp/twelf-icon.ppm
	node make-icon-resources.js >../docker/app/icon.r
