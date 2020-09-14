#!/bin/bash
#
# Copyright (c) 2020 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

mkdir -p $HOME/resnet50/1
if [ ! -f "$HOME/resnet50/1/resnet_50_i8.xml" ]; then
    wget https://storage.googleapis.com/public-artifacts/intelai_public_models/resnet_50_i8/1/resnet_50_i8.xml --directory-prefix $HOME/resnet50/1/
fi
if [ ! -f "$HOME/resnet50/1/resnet_50_i8.bin" ]; then
    wget https://storage.googleapis.com/public-artifacts/intelai_public_models/resnet_50_i8/1/resnet_50_i8.bin --directory-prefix $HOME/resnet50/1/
fi