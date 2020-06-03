.PHONY: watch backup build
watch:
	./watch
backup:
	./backup

build:
	meson build/
	cd build/ && ninja
