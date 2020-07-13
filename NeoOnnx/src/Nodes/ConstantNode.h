/* Copyright © 2017-2020 ABBYY Production LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------------------------------------------------------*/

#pragma once

#include "../Node.h"

// Forward declaration(s).
namespace onnx {
class NodeProto;
} // namespace onnx

namespace NeoOnnx {

class CConstantNode : public COpNode {
public:
	CConstantNode( const onnx::NodeProto& constant, int opsetVersion, IMathEngine& mathEngine );

	// CNode methods' realizations.
	void CalcOutputShape() override;
	void CalcOutputData() override;
	void MarkTensorDims() override {}
	void AddLayers( CDnn& ) override {}

private:
	CPtr<CDnnBlob> value; // constant's value
};

} // namespace NeoOnnx
