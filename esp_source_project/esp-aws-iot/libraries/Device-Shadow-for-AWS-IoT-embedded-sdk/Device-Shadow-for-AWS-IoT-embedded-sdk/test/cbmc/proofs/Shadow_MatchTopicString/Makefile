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
HARNESS_FILE=Shadow_MatchTopicString_harness
PROOF_UID = Shadow_MatchTopicString

# The topic length is bounded to reduce the proof run time. This length bounds
# the time to look for the thing and shadow names in the topic string. It adds
# no value to the proof to input the largest possible topic string accepted by
# AWS (64KB). We allow for a the longest named shadow topic, but with only
# reasonable allowance for typical Thing and Shadow names.
# $aws/things/thingName/shadow/name/shadowName/update/documents
TOPIC_STRING_LENGTH_MAX=65

DEFINES += -DTOPIC_STRING_LENGTH_MAX=$(TOPIC_STRING_LENGTH_MAX)
INCLUDES +=

# The maximum length of the message type ( /update/documents ) is 17.
UNWINDSET += strncmp.0:18

# Allow for the longest possible Shadow name
UNWINDSET += __CPROVER_file_local_shadow_c_validateName.0:64

# The number of message types is 8.
UNWINDSET += __CPROVER_file_local_shadow_c_extractShadowMessageType.0:9


PROOF_SOURCES += $(PROOFDIR)/$(HARNESS_FILE).c
PROJECT_SOURCES += $(SRCDIR)/source/shadow.c

include ../Makefile.common
