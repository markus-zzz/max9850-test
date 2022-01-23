#!/bin/bash

set -e -x
yosys ulx3s-top.ys
nextpnr-ecp5 \
	--json max9850-test.json \
	--textcfg max9850-test.config \
	--lpf ulx3s.lpf \
	--25k

ecppack --idcode 0x21111043 max9850-test.config max9850-test.bit
