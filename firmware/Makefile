.PHONY: obc obc_sitl obc_sitl_freerun run_obc_sitl test audit clean
.SILENT:

BUILD_TYPE ?= Release
LOG_LEVEL  ?= OFF

obc:
	cmake \
		-S platform/pico \
		-B build/obc \
		-G Ninja \
		-D BOARD_NAME=obc \
		-D RUNNER_PROFILE=obc_flight \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL)
	ninja -C build/obc

obc_sitl:
	cmake \
		-S platform/host \
		-B build/obc_sitl \
		-G Ninja \
		-D BOARD_NAME=obc \
		-D RUNNER_PROFILE=obc_sitl \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL) \
		-D SITL_FREERUN=OFF \
		-D BUILD_TESTS=OFF
	ninja -C build/obc_sitl

obc_sitl_freerun:
	cmake \
		-S platform/host \
		-B build/obc_sitl \
		-G Ninja \
		-D BOARD_NAME=obc \
		-D RUNNER_PROFILE=obc_sitl \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL) \
		-D SITL_FREERUN=ON \
		-D BUILD_TESTS=OFF
	ninja -C build/obc_sitl

run_obc_sitl:
	cd build/obc_sitl && ./firmware

test:
	cmake \
		-S platform/host \
		-B build/test \
		-G Ninja \
		-D BOARD_NAME=obc \
		-D RUNNER_PROFILE=obc_sitl \
		-D CMAKE_BUILD_TYPE=Debug \
		-D LOG_LEVEL=OFF \
		-D SITL_FREERUN=OFF \
		-D BUILD_TESTS=ON
	ninja -C build/test
	ctest --test-dir build/test --output-on-failure

audit:
	python tools/audit.py

clean:
	rm -rf build/