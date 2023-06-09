#
# Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

HARNESS_ENTRY=harness
HARNESS_FILE=MQTT_SerializeConnect_harness
PROOF_UID=MQTT_SerializeConnect

DEFINES +=
INCLUDES +=

# This function does not coincide with the call graph of MQTT_Serialize, but is
# found by CBMC during processing in logs/MQTT_Connect_harness3.txt. We remove
# the function body to improve coverage accuracy.
REMOVE_FUNCTION_BODY += MQTT_GetIncomingPacketTypeAndLength

# The encodeRemainingLength loop is unwound 5 times because encodeRemainingLength()
# divides a size_t variable by 128 until it reaches zero to stop the loop.
# log128(SIZE_MAX) = 4.571...
UNWINDSET += __CPROVER_file_local_core_mqtt_serializer_c_encodeRemainingLength.0:5

PROOF_SOURCES += $(PROOFDIR)/$(HARNESS_FILE).c
PROOF_SOURCES += $(SRCDIR)/test/cbmc/sources/mqtt_cbmc_state.c
PROJECT_SOURCES += $(SRCDIR)/source/core_mqtt_serializer.c

include ../Makefile.common
