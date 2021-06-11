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
#pragma once

#include <memory>

#include "status.hpp"
#include "tensorinfo.hpp"

namespace ovms {
Status convertStringValToBlob(const tensorflow::TensorProto& src, InferenceEngine::Blob::Ptr& blob, const std::shared_ptr<TensorInfo>& tensorInfo);
Status convertBlobToStringVal(tensorflow::TensorProto& dst, const std::shared_ptr<TensorInfo>& tensorInfo, const InferenceEngine::Blob::Ptr& blob);
}  // namespace ovms
