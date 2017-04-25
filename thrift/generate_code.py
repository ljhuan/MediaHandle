#!/usr/bin/python
#coding = utf-8

import os
import sys
thrift = ""
if sys != "win32":
    thrift = "thrift"

print thrift
os.system("%s --gen cpp --gen py media_server.thrift" % thrift)
