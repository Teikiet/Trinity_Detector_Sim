#!/bin/sh
# Copyright (c) Contributors to the Apptainer project, established as
#   Apptainer a Series of LF Projects LLC.
#   For website terms of use, trademark policy, privacy policy and other
#   project policies see https://lfprojects.org/policies
# Copyright (c) 2018-2021, Sylabs Inc. All rights reserved.
# This software is licensed under a 3-clause BSD license. Please consult
# https://github.com/apptainer/apptainer/blob/main/LICENSE.md regarding your
# rights to use or distribute this software.

# Custom environment shell code should follow

    export ROBROOT=/root/TrinitySims/ROBAST/ROBAST-3.2.0
	export LD_LIBRARY_PATH=$ROBROOT:/usr/local/lib:/usr/lib/oracle/21/client64/lib:/exact/dict:$LD_LIBRARY_PATH
	export ROOT_INCLUDE_PATH=$ROBROOT/include:$ROOT_INCLUDE_PATH
	export PATH=$PATH:/root/TrinitySims/CARE/VBF-0.3.4-c-17/vbfTools
	export LD_LIBRARY_PATH=/root/TrinitySims/CARE/lib:$LD_LIBRARY_PATH



