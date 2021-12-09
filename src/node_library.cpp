//*****************************************************************************
// Copyright 2021 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#include "node_library.hpp"

namespace ovms {

bool NodeLibrary::isValid() const {
    return execute != nullptr &&
           getInputsInfo != nullptr &&
           getOutputsInfo != nullptr &&
           release != nullptr &&
           initialize != nullptr &&
           deinitialize != nullptr;
}

NodeLibraryExecutor::NodeLibraryExecutor(std::unique_ptr<NodeLibraryBase>&& ptr) :
    nodeLibrary(std::move(ptr)) {
    //    auto v2Ptr = dynamic_cast<NodeLibraryV2*>(nodeLibrary.get());
}

bool NodeLibraryBase::isValid() const {
    return true;
}

}  // namespace ovms
