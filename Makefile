ifndef SNAP_ROOT
$(info You have specified a wrong $$SNAP_ROOT.)
$(error Please make sure that $$SNAP_ROOT is set up correctly.)
endif

ifndef METAL_ROOT
$(info You have specified a wrong $$METAL_ROOT.)
$(error Please make sure that $$METAL_ROOT is set up correctly.)
endif

export IMAGE_JSON        = $(PWD)/image.json
export IMAGE_TARGET      = $(PWD)/image_compiled.json
export METAL_TARGET     ?= SNAP_WEBPACK_SIM

snap_targets = hw_project model image 

$(IMAGE_JSON):
	@echo '{ "target": "$(METAL_TARGET)", "stream_bytes": 64, "operators": { "filter": { "source": "./." } } }' > $@

$(snap_targets): $(IMAGE_JSON)
	@make -C $(SNAP_ROOT) -s $@

sim:
	@$(METAL_ROOT)/targets/$(METAL_TARGET)/sim

SOLUTION_NAME ?= hls_operator_filter_sln
SOLUTION_DIR ?= hls_operator_filter_sln

FPGACHIP ?= xcku035-ffva1156-2-e
HLS_ACTION_CLOCK ?= 4

HLS_CFLAGS = -std=c++11 -I$(METAL_ROOT)/src/metal_fpga/hw/hls/include

srcs += hls_operator_filter.cpp

WRAPPER ?= $(shell jq -r .main operator.json)

include $(SNAP_ROOT)/actions/hls.mk
include $(METAL_ROOT)/src/metal_fpga/hw/hls/hls.mk
