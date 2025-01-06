cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Nordic)

file(GLOB_RECURSE LIBCORRECT_FILES_C "libcorrect/*.c")
file(GLOB_RECURSE LIBCORRECT_FILES_H "libcorrect/*.h")


target_sources(app PRIVATE 
                            src/main.c 
                            src/peripheral-controllers/signals/spim_controller.h src/peripheral-controllers/signals/spim_controller.c 
                            src/peripheral-controllers/signals/gpiote_controller.h src/peripheral-controllers/signals/gpiote_controller.c 
                            src/peripheral-controllers/timing/timer_controller.h src/peripheral-controllers/timing/timer_controller.c 
                            src/peripheral-controllers/timing/dppi_controller.h src/peripheral-controllers/timing/dppi_controller.c 
                            src/common.h src/common.c 
                            src/peripheral-controllers/interfacing/gpio_controller.h src/peripheral-controllers/interfacing/gpio_controller.c 
                            src/buffer.h src/buffer.c 
                            src/manchester.h src/manchester.c
                            src/peripheral-controllers/input-detection/adc_controller.h src/peripheral-controllers/input-detection/adc_controller.c
                            src/peripheral-controllers/signals/spis_controller.h src/peripheral-controllers/signals/spis_controller.c
                            src/peripheral-controllers/interfacing/uarte_controller.h src/peripheral-controllers/interfacing/uarte_controller.c
                            src/peripheral-controllers/input-detection/lpcomp_controller.h src/peripheral-controllers/input-detection/lpcomp_controller.c
                            ${LIBCORRECT_FILES_C}
                            ${LIBCORRECT_FILES_H}
                )
