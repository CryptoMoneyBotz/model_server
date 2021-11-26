//*****************************************************************************
// Copyright 2020 Intel Corporation
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

#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#pragma GCC diagnostic pop

#include "../deserialization.hpp"
#include "ovtestutils.hpp"
#include "test_utils.hpp"

#include <gmock/gmock-generated-function-mockers.h>

using tensorflow::TensorProto;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;

using InferenceEngine::IInferRequest;
using InferenceEngine::PreProcessInfo;
using InferenceEngine::ResponseDesc;

using namespace ovms;
using namespace InferenceEngine;

using testing::_;
using testing::NiceMock;
using testing::Throw;

const std::vector<InferenceEngine::Precision> SUPPORTED_INPUT_PRECISIONS{
    // InferenceEngine::Precision::UNDEFINED,
    // InferenceEngine::Precision::MIXED,
    InferenceEngine::Precision::FP32,
    InferenceEngine::Precision::FP16,
    // InferenceEngine::Precision::Q78,
    InferenceEngine::Precision::I16,
    InferenceEngine::Precision::U8,
    InferenceEngine::Precision::I8,
    InferenceEngine::Precision::U16,
    InferenceEngine::Precision::I32,
    // InferenceEngine::Precision::I64,
    // InferenceEngine::Precision::BIN,
    // InferenceEngine::Precision::BOOL
    // //InferenceEngine::Precision::CUSTOM)
};

const std::vector<ovms::Precision> SUPPORTED_INPUT_PRECISIONS_2{
    // ovms::Precision::UNSPECIFIED,
    // ovms::Precision::MIXED,
    ovms::Precision::FP32,
    ovms::Precision::FP16,
    // InferenceEngine::Precision::Q78,
    ovms::Precision::I16,
    ovms::Precision::U8,
    ovms::Precision::I8,
    ovms::Precision::U16,
    ovms::Precision::I32,
    // ovms::Precision::I64,
    // ovms::Precision::BIN,
    // ovms::Precision::BOOL
    // ovms::Precision::CUSTOM)
};

const std::vector<InferenceEngine::Precision> UNSUPPORTED_INPUT_PRECISIONS{
    InferenceEngine::Precision::UNSPECIFIED,
    InferenceEngine::Precision::MIXED,
    // InferenceEngine::Precision::FP32,
    // InferenceEngine::Precision::FP16,
    InferenceEngine::Precision::Q78,
    // InferenceEngine::Precision::I16,
    // InferenceEngine::Precision::U8,
    // InferenceEngine::Precision::I8,
    // InferenceEngine::Precision::U16,
    // InferenceEngine::Precision::I32,
    InferenceEngine::Precision::I64,
    InferenceEngine::Precision::BIN,
    InferenceEngine::Precision::BOOL
    // InferenceEngine::Precision::CUSTOM)
};

const std::vector<ovms::Precision> UNSUPPORTED_INPUT_PRECISIONS_2{
   ovms::Precision::UNDEFINED,
   ovms::Precision::MIXED,
    // ovms::Precision::FP32,
    // ovms::Precision::FP16,
    ovms::Precision::Q78,
    // ovms::Precision::I16,
    // ovms::Precision::U8,
    // ovms::Precision::I8,
    // ovms::Precision::U16,
    // ovms::Precision::I32,
    ovms::Precision::I64,
    ovms::Precision::BIN,
    ovms::Precision::BOOL
    // ovms::Precision::CUSTOM)
};

class TensorflowGRPCPredict : public ::testing::TestWithParam<InferenceEngine::Precision> {
protected:
    void SetUp() override {
        InferenceEngine::Precision precision = InferenceEngine::Precision::FP32;
        InferenceEngine::TensorDesc tensorDesc_prec_1_3_1_1_NHWC = {
            precision,
            {1, 3, 1, 1},
            InferenceEngine::Layout::NHWC};

        tensorMap[tensorName] = std::make_shared<ovms::TensorInfo>(
            tensorName,
            tensorDesc_prec_1_3_1_1_NHWC.getPrecision(),
            tensorDesc_prec_1_3_1_1_NHWC.getDims(),
            tensorDesc_prec_1_3_1_1_NHWC.getLayout());
        SetUpTensorProto(fromInferenceEnginePrecision(precision));
    }
    void SetUpTensorProto(tensorflow::DataType dataType) {
        tensorProto.set_dtype(dataType);
        auto tensorShape = tensorProto.mutable_tensor_shape();
        tensorShape->Clear();
        tensorShape->add_dim()->set_size(1);
        tensorShape->add_dim()->set_size(3);
        tensorShape->add_dim()->set_size(1);
        tensorShape->add_dim()->set_size(1);
        *(tensorProto.mutable_tensor_content()) = std::string(1 * 3 * 1 * 1, '1');
    }
    TensorProto tensorProto;
    const char* tensorName = "Input_PRECISION_1_3_1_1_NHWC";
    ovms::tensor_map_t tensorMap;
    bool isPipeline = false;
};

class TensorflowGRPCPredict_2 : public ::testing::TestWithParam<ovms::Precision> {
protected:
    void SetUp() override {
        auto precision = ovms::Precision::FP32;
        tensorMap[tensorName] = std::make_shared<ovms::TensorInfo>(
            tensorName,
            precision,
            shape_t{1, 3, 1, 1},
            InferenceEngine::Layout::NHWC);
        SetUpTensorProto(TensorInfo::getPrecisionAsDataType(precision));
    }
    void SetUpTensorProto(tensorflow::DataType dataType) {
        tensorProto.set_dtype(dataType);
        auto tensorShape = tensorProto.mutable_tensor_shape();
        tensorShape->Clear();
        tensorShape->add_dim()->set_size(1);
        tensorShape->add_dim()->set_size(3);
        tensorShape->add_dim()->set_size(1);
        tensorShape->add_dim()->set_size(1);
        *(tensorProto.mutable_tensor_content()) = std::string(1 * 3 * 1 * 1, '1');
    }
    TensorProto tensorProto;
    const char* tensorName = "Input_PRECISION_1_3_1_1_NHWC";
    ovms::tensor_map_t tensorMap;
    bool isPipeline = false;
};

class DeserializeTFTensorProto_2 : public TensorflowGRPCPredict_2 {};
class DeserializeTFTensorProtoNegative_2 : public TensorflowGRPCPredict_2 {};

class GRPCPredictRequest : public TensorflowGRPCPredict {
public:
    void SetUp() {
        TensorflowGRPCPredict::SetUp();
        (*request.mutable_inputs())[tensorName] = tensorProto;
    }
    void TearDown() {
        request.mutable_inputs()->clear();
    }

public:
    PredictRequest request;
};

class GRPCPredictRequest_2 : public TensorflowGRPCPredict_2 {
public:
    void SetUp() {
        TensorflowGRPCPredict_2::SetUp();
        (*request.mutable_inputs())[tensorName] = tensorProto;
    }
    void TearDown() {
        request.mutable_inputs()->clear();
    }

public:
    PredictRequest request;
};

class GRPCPredictRequestNegative : public GRPCPredictRequest {};

TEST_P(GRPCPredictRequestNegative, ShouldReturnDeserializationErrorForPrecision) {
    InferenceEngine::Precision testedPrecision = GetParam();
    tensorMap[tensorName]->setPrecision(testedPrecision);
    InferenceEngine::InferRequest inferRequest;
    InputSink<InferRequest&> inputSink(inferRequest);
    auto status = deserializePredictRequest<ConcreteTensorProtoDeserializator>(request, tensorMap, inputSink, isPipeline);
    EXPECT_EQ(status, ovms::StatusCode::OV_UNSUPPORTED_DESERIALIZATION_PRECISION)
        << "Unsupported OVMS precision:"
        << testedPrecision
        << " should return error";
}

TEST_P(GRPCPredictRequestNegative, ShouldReturnDeserializationErrorForSetBlobException) {
    InferenceEngine::Precision testedPrecision = GetParam();
    tensorMap[tensorName]->setPrecision(testedPrecision);
    InferenceEngine::InferRequest inferRequest;
    InputSink<InferRequest&> inputSink(inferRequest);
    auto status = deserializePredictRequest<ConcreteTensorProtoDeserializator>(request, tensorMap, inputSink, isPipeline);
    EXPECT_EQ(status, ovms::StatusCode::OV_UNSUPPORTED_DESERIALIZATION_PRECISION) << status.string();
}

class MockTensorProtoDeserializatorThrowingInferenceEngine {
public:
    MOCK_METHOD(InferenceEngine::Blob::Ptr,
        deserializeTensorProto,
        (const tensorflow::TensorProto&,
            const std::shared_ptr<ovms::TensorInfo>&, bool));
};

// Enables static method mock
class MockTensorProtoDeserializator {
public:
    static MockTensorProtoDeserializatorThrowingInferenceEngine* mock;
    static InferenceEngine::Blob::Ptr deserializeTensorProto(
        const tensorflow::TensorProto& requestInput,
        const std::shared_ptr<ovms::TensorInfo>& tensorInfo, bool isPipeline) {
        return mock->deserializeTensorProto(requestInput, tensorInfo, isPipeline);
    }
};

MockTensorProtoDeserializatorThrowingInferenceEngine* MockTensorProtoDeserializator::mock = nullptr;

TEST_F(GRPCPredictRequestNegative, ShouldReturnDeserializationErrorForSetBlobException2) {
    InferenceEngine::TensorDesc tensorDesc(InferenceEngine::Precision::FP32, shape_t{1, 10}, InferenceEngine::Layout::NC);
    std::shared_ptr<ovms::TensorInfo> tensorInfo = std::make_shared<ovms::TensorInfo>(
        DUMMY_MODEL_INPUT_NAME,
        tensorDesc.getPrecision(),
        tensorDesc.getDims(),
        tensorDesc.getLayout());
    tensorMap[tensorName] = tensorInfo;
    InferenceEngine::Core ieCore;
    InferenceEngine::CNNNetwork network = ieCore.ReadNetwork(std::filesystem::current_path().u8string() + "/src/test/dummy/1/dummy.xml");
    InferenceEngine::ExecutableNetwork execNetwork = ieCore.LoadNetwork(network, "CPU");
    InferenceEngine::InferRequest inferRequest = execNetwork.CreateInferRequest();
    std::shared_ptr<NiceMock<MockBlob>> mockBlobPtr = std::make_shared<NiceMock<MockBlob>>(tensorDesc);
    inferRequest.SetBlob("b", mockBlobPtr);
    MockTensorProtoDeserializatorThrowingInferenceEngine mockTPobject;
    MockTensorProtoDeserializator::mock = &mockTPobject;
    EXPECT_CALL(mockTPobject, deserializeTensorProto(_, _, _))
        .Times(1)
        .WillRepeatedly(
            Throw(InferenceEngine::GeneralError("")));
    InputSink<InferRequest&> inputSink(inferRequest);
    Status status;
    status = deserializePredictRequest<MockTensorProtoDeserializator>(
        request, tensorMap, inputSink, isPipeline);
    EXPECT_EQ(status, ovms::StatusCode::OV_INTERNAL_DESERIALIZATION_ERROR) << status.string();
}

// TEST_F(GRPCPredictRequest, ShouldSuccessForSupportedPrecision) {
//     InferenceEngine::TensorDesc tensorDesc(InferenceEngine::Precision::FP32, shape_t{1, 10}, InferenceEngine::Layout::NC);
//     std::shared_ptr<ovms::TensorInfo> tensorInfo = std::make_shared<ovms::TensorInfo>(
//         DUMMY_MODEL_INPUT_NAME,
//         tensorDesc.getPrecision(),
//         tensorDesc.getDims(),
//         tensorDesc.getLayout());
//     tensorMap[tensorName] = tensorInfo;
//     InferenceEngine::Core ieCore;
//     InferenceEngine::CNNNetwork network = ieCore.ReadNetwork(std::filesystem::current_path().u8string() + "/src/test/dummy/1/dummy.xml");
//     InferenceEngine::ExecutableNetwork execNetwork = ieCore.LoadNetwork(network, "CPU");
//     InferenceEngine::InferRequest inferRequest = execNetwork.CreateInferRequest();
//     std::shared_ptr<NiceMock<MockBlob>> mockBlobPtr = std::make_shared<NiceMock<MockBlob>>(tensorDesc);
//     inferRequest.SetBlob("b", mockBlobPtr);
//     InputSink<InferRequest&> inputSink(inferRequest);
//     auto status = deserializePredictRequest<ConcreteTensorProtoDeserializator>(request, tensorMap, inputSink, isPipeline);
//     EXPECT_TRUE(status.ok());
// }

TEST_F(GRPCPredictRequest_2, ShouldSuccessForSupportedPrecision) {
    auto tensorInfo = std::make_shared<ovms::TensorInfo>(
        DUMMY_MODEL_INPUT_NAME,
        ovms::Precision::FP32,
        ovms::shape_t{1, 10},
        InferenceEngine::Layout::NC);
    ov::runtime::Core ieCore;
    std::shared_ptr<ov::Function> network = ieCore.read_model(std::filesystem::current_path().u8string() + "/src/test/dummy/1/dummy.xml");
    ov::runtime::ExecutableNetwork execNetwork = ieCore.compile_model(network, "CPU");
    ov::runtime::InferRequest inferRequest = execNetwork.create_infer_request();
    NiceMock<MockBlob_2> mockedTensor(tensorInfo);
    inferRequest.set_tensor(DUMMY_MODEL_INPUT_NAME, mockedTensor);
    InputSink_2<ov::runtime::InferRequest&> inputSink(inferRequest);
    // auto status = deserializePredictRequest<ConcreteTensorProtoDeserializator>(request, tensorMap, inputSink, isPipeline);
    // EXPECT_TRUE(status.ok());
}

TEST_P(DeserializeTFTensorProtoNegative_2, ShouldReturnNullptrForPrecision) {
    ovms::Precision testedPrecision = GetParam();
    tensorMap[tensorName]->setPrecision(testedPrecision);
    ov::runtime::Tensor tensor = deserializeTensorProto_2<ConcreteTensorProtoDeserializator_2>(tensorProto, tensorMap[tensorName], isPipeline);
    EXPECT_FALSE((bool)tensor) << "Unsupported OVMS precision:"
                                << toString(testedPrecision)
                                << " should return nullptr";
}

TEST_P(DeserializeTFTensorProto_2, ShouldReturnValidBlob) {
    ovms::Precision testedPrecision = GetParam();
    SetUpTensorProto(TensorInfo::getPrecisionAsDataType(testedPrecision));
    tensorMap[tensorName]->setPrecision(testedPrecision);
    ov::runtime::Tensor tensor = deserializeTensorProto_2<ConcreteTensorProtoDeserializator_2>(tensorProto, tensorMap[tensorName], isPipeline);
    EXPECT_TRUE((bool)tensor) << "Supported OVMS precision:"
                                << toString(testedPrecision)
                                << " should return valid blob ptr";
}

INSTANTIATE_TEST_SUITE_P(
    TestDeserialize,
    GRPCPredictRequestNegative,
    ::testing::ValuesIn(UNSUPPORTED_INPUT_PRECISIONS),
    ::testing::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    TestDeserialize,
    GRPCPredictRequest_2,
    ::testing::ValuesIn(SUPPORTED_INPUT_PRECISIONS_2),
    [](const ::testing::TestParamInfo<GRPCPredictRequest_2::ParamType>& info) {
      return toString(info.param);
    });

INSTANTIATE_TEST_SUITE_P(
    Test,
    DeserializeTFTensorProtoNegative_2,
    ::testing::ValuesIn(UNSUPPORTED_INPUT_PRECISIONS_2),
    [](const ::testing::TestParamInfo<DeserializeTFTensorProtoNegative_2::ParamType>& info) {
      return toString(info.param);
    });

INSTANTIATE_TEST_SUITE_P(
    Test,
    DeserializeTFTensorProto_2,
    ::testing::ValuesIn(SUPPORTED_INPUT_PRECISIONS_2),
    [](const ::testing::TestParamInfo<DeserializeTFTensorProto_2::ParamType>& info) {
      return toString(info.param);
    });
