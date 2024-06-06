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
#bin_corundum := $(d)corundum_verilator
verilator_dir_corundum := $(d)obj_dir
verilator_src_corundum := $(verilator_dir_corundum)/Vinterface.cpp
#verilator_bin_corundum := $(verilator_dir_corundum)/Vinterface

# TODO
# vsrcs_corundum := $(wildcard $(d)rtl/*.v $(d)lib/*/rtl/*.v \
#     $(d)lib/*/lib/*/rtl/*.v)
#srcs_corundum := $(addprefix $(d),corundum_verilator.cc dma.cc mem.cc)
#OBJS := $(srcs_corundum:.cc=.o)


#$(OBJS): CPPFLAGS := $(CPPFLAGS) -I$(d)include/



PTP_TS_ENABLE = 1,
TX_CPL_ENABLE = PTP_TS_ENABLE,
TX_CHECKSUM_ENABLE = 1,
RX_HASH_ENABLE = 1,
RX_CHECKSUM_ENABLE = 1,
PFC_ENABLE = 1,
LFC_ENABLE = PFC_ENABLE,

APP_DMA_ENABLE = 1,
APP_AXIS_DIRECT_ENABLE = 1,
APP_AXIS_SYNC_ENABLE = 1,
APP_AXIS_IF_ENABLE = 1,
APP_STAT_ENABLE = 1,

$(verilator_src_corundum): # $(vsrcs_corundum)
	$(VERILATOR) $(VFLAGS) -GPTP_TS_ENABLE=0 -GSTAT_ENABLE=0 -GSTAT_DMA_ENABLE=0 -GSTAT_AXI_ENABLE=0 -GAPP_CTRL_ENABLE=0 --cc -O3 \
	    -CFLAGS "-I$(abspath $(lib_dir)) -iquote $(abspath $(base_dir)) -O3 -g -Wall -Wno-maybe-uninitialized" \
	    --Mdir $(verilator_dir_corundum) \
		--top-module mqnic_core_axi \
	    -y $(dir_corundum)/fpga/common/rtl \
		-y $(dir_corundum)/fpga/common/lib/axis/rtl \
		-y $(dir_corundum)/fpga/common/lib/eth/rtl \
		-y $(dir_corundum)/fpga/common/lib/pcie/rtl \
	    -y $(dir_corundum)/fpga/lib/axi/rtl \
	    -y $(dir_corundum)/fpga/lib/eth/lib/axis/rtl/ \
	    -y $(dir_corundum)/fpga/lib/pcie/rtl \
	    $(dir_corundum)/fpga/common/rtl/mqnic_core_axi.v \
	    $(dir_corundum)/fpga/common/rtl/mqnic_tx_scheduler_block_rr.v 
#		 --exe $(abspath $(srcs_corundum)) \
#	      $(abspath $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base))

#$(verilator_bin_corundum): $(verilator_src_corundum) $(srcs_corundum) \
#    $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base)
#	$(MAKE) -C $(verilator_dir_corundum) -f Vinterface.mk

#$(bin_corundum): $(verilator_bin_corundum)
#	cp $< $@

#CLEAN := $(bin_corundum) $(verilator_dir_corundum) $(OBJS)
CLEAN := $(verilator_dir_corundum)

ifeq ($(ENABLE_VERILATOR),y)
#ALL := $(bin_corundum)
ALL := $(verilator_src_corundum)
endif
include mk/subdir_post.mk
