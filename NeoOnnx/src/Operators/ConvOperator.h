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

#include "../Operator.h"

namespace NeoOnnx {

// Conv operator
class CConvOperator : public CLayerOperator {
public:
	CConvOperator( const onnx::NodeProto& conv, int opsetVersion );

	// CLayerOperator methods
	void AddLayers( const CObjectArray<const CTensorBase>& inputs,
		CDnn& dnn, CObjectArray<const CTensorBase>& outputs ) override;

	// COperator methods
	void UserInputMask( CUserInputMask& mask ) const override { mask |= 0; }

private:
	// groups count
	const int group;
	// padding mode
	const CString autoPad;
	// kernel strides
	CFastArray<int, 8> strides;
	// convolution paddings
	CFastArray<int, 8> pads;
	// convolution dilations
	CFastArray<int, 8> dilations;
	// convolution dilations
	CFastArray<int, 8> outputShape;

	void add2dConvLayer( const CObjectArray<const CTensorBase>& inputs,
		CDnn& dnn, CObjectArray<const CTensorBase>& outputs );
	void add3dConvLayer( const CObjectArray<const CTensorBase>& inputs,
		CDnn& dnn, CObjectArray<const CTensorBase>& outputs );
};

} // namespace NeoOnnx
