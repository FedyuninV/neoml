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

// Softmax operator graph node
class CSoftmaxNode : public CLayerOpNode {
public:
	CSoftmaxNode( const onnx::NodeProto& softmax, int opsetVersion );

	// CNode methods
	void AddLayers( const CObjectArray<const CTensorBase>& inputs,
		CObjectArray<const CTensorBase>& outputs, CDnn& dnn ) override;

	// COpNode methods
	void UserInputMask( CUserInputMask& mask ) const override
		{ mask.Add( true ); mask.Add( false, InputCount() - 1 ); }

private:
	int axis; // First axis to be softmaxed

	CTensorLayout getCompatibleLayout( int dimCount, int axis, const CTensorLayout& inputLayout ) const;
};

} // namespace NeoOnnx