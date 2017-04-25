#!/bin/bash

protoc --cpp_out=./ --python_out=./ taskinfo.proto
