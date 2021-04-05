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

namespace NeoOnnx {

// Gemm operation node
class CGemmNode : public CLayerOpNode {
public:
	CGemmNode( const onnx::NodeProto& node, int opsetVersion );

	// CNode methods
	void AddLayers( const CObjectArray<const CTensorBase>& inputs,
		CObjectArray<const CTensorBase>& outputs, CDnn& dnn ) override;

	// COpNode methods
	void UserInputMask( CUserInputMask& mask ) const override
		{ mask.Add( true ); mask.Add( false, InputCount() - 1 ); }

private:
	// In onnx Gemm is implemented like
	//     Y = alpha * A' * B' + beta * C
	// where
	//     A' is equal to A if transA == 0 or transpose(A) otherwise
	//     B' is equal to B if transB == 0 or transpose(B) otherwise
	//     C is optional matrix

	// Values from formula above
	const float alpha;
	const float beta;
	const int transA;
	const int transB;
};

} // namespace NeoOnnx
