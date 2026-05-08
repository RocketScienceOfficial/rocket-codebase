.PHONY: obc obc_sitl obc_sitl_freerun obc_sitl_run radio_module radio_module_sitl radio_module_sitl_run gcs gcs_flash gcs_sitl gcs_sitl_run test audit clean
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
		-D SITL_FREERUN=OFF
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
		-D SITL_FREERUN=ON
	ninja -C build/obc_sitl

obc_sitl_run:
	cd build/obc_sitl && ./firmware


radio_module:
	cmake \
		-S platform/pico \
		-B build/radio_module \
		-G Ninja \
		-D BOARD_NAME=radio_module \
		-D RUNNER_PROFILE=radio_module_flight \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL)
	ninja -C build/radio_module

radio_module_sitl:
	cmake \
		-S platform/host \
		-B build/radio_module_sitl \
		-G Ninja \
		-D BOARD_NAME=radio_module \
		-D RUNNER_PROFILE=radio_module_sitl \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL) \
		-D SITL_FREERUN=ON
	ninja -C build/radio_module_sitl

radio_module_sitl_run:
	cd build/radio_module_sitl && ./firmware


gcs:
	idf.py \
		-C platform/esp32 \
		-B build/gcs \
		-D BOARD_NAME=gcs \
		-D RUNNER_PROFILE=gcs_flight \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL) \
		build

PORT ?= /dev/ttyUSB0

gcs_flash:
	idf.py \
		-C platform/esp32 \
		-B build/gcs \
		-p ${PORT} \
		flash

OLED_TEST_MODE ?= OFF

gcs_sitl:
	cmake \
		-S platform/host \
		-B build/gcs_sitl \
		-G Ninja \
		-D BOARD_NAME=gcs \
		-D RUNNER_PROFILE=gcs_sitl \
		-D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-D LOG_LEVEL=$(LOG_LEVEL) \
		-D OLED_TEST_MODE=$(OLED_TEST_MODE) \
		-D SITL_FREERUN=ON
	ninja -C build/gcs_sitl

gcs_sitl_run:
	cd build/gcs_sitl && ./firmware


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