# Copyright 2021 Max Planck Institute for Software Systems, and
# National University of Singapore
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include mk/subdir_pre.mk

dir_corundum := $(abspath $(d))
verilator_dir_corundum := $(d)obj_dir
verilog_interface_name := mqnic_core_axi
verilator_interface_name := V$(verilog_interface_name)
verilator_src_corundum := $(verilator_dir_corundum)/$(verilator_interface_name).cpp
verilator_bin_corundum := $(verilator_dir_corundum)/$(verilator_interface_name)
adapter_main := corundum_simbricks_adapter
corundum_simbricks_adapter_src := $(addprefix $(d),$(adapter_main).cpp)
OBJS := $(corundum_simbricks_adapter_src:.cpp=.o)
$(OBJS): CPPFLAGS := $(CPPFLAGS) -I$(d)include/
corundum_simbricks_adapter_bin := $(d)$(adapter_main)

$(verilator_src_corundum): # $(vsrcs_corundum)
	$(VERILATOR) $(VFLAGS) -GPTP_TS_ENABLE=0 -GSTAT_ENABLE=0 -GSTAT_DMA_ENABLE=0 \
		-GSTAT_AXI_ENABLE=0 -GAPP_ENABLE=0 -GAPP_CTRL_ENABLE=0 -GAPP_DMA_ENABLE=0 \
		-GAPP_AXIS_DIRECT_ENABLE=0 -GAPP_AXIS_SYNC_ENABLE=0 -GAPP_AXIS_IF_ENABLE=0 \
		-GAPP_STAT_ENABLE=0 --cc -O3 \
	    -CFLAGS "-I$(abspath $(lib_dir)) -iquote $(abspath $(base_dir)) -O3 -g -Wall -Wno-maybe-uninitialized" \
	    --Mdir $(verilator_dir_corundum) \
		--top-module $(verilog_interface_name) \
	    -y $(dir_corundum)/fpga/common/rtl \
		-y $(dir_corundum)/fpga/common/lib/axis/rtl \
		-y $(dir_corundum)/fpga/common/lib/eth/rtl \
		-y $(dir_corundum)/fpga/common/lib/pcie/rtl \
	    -y $(dir_corundum)/fpga/lib/axi/rtl \
	    -y $(dir_corundum)/fpga/lib/eth/lib/axis/rtl/ \
	    -y $(dir_corundum)/fpga/lib/pcie/rtl \
	    $(dir_corundum)/fpga/common/rtl/$(verilog_interface_name).v \
	    $(dir_corundum)/fpga/common/rtl/mqnic_tx_scheduler_block_rr.v \
		--exe $(abspath $(corundum_simbricks_adapter_src)) $(abspath $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base))

$(verilator_bin_corundum): $(verilator_src_corundum) $(corundum_simbricks_adapter_src) \
    $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base)
	$(MAKE) -C $(verilator_dir_corundum) -f $(verilator_interface_name).mk

$(corundum_simbricks_adapter_bin): $(verilator_bin_corundum)
	cp $< $@

$(d)ready: $(corundum_simbricks_adapter_bin)
	touch $@

CLEAN := $(corundum_simbricks_adapter_bin) $(verilator_dir_corundum) $(OBJS) $(d)ready

include mk/subdir_post.mk
