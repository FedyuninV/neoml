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

#include "../common.h"
#pragma hdrstop

#include "ConstantOfShapeNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CConstantOfShapeNode::CConstantOfShapeNode( int nodeIndex, const onnx::NodeProto& constantOfShape, int opsetVersion ) :
	COpNode( nodeIndex, constantOfShape, opsetVersion )
{
	// This op was introduced in version 9
	CheckOnnxProtocol( opsetVersion >= 9, "wrong opset version", constantOfShape );
	CheckNeoOnnxSupport( opsetVersion <= MaxOpsetVersion, "opset version", constantOfShape );

	CheckOnnxProtocol( InputCount() == 1, "node must have 1 input", constantOfShape );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", constantOfShape );
}

void CConstantOfShapeNode::CalcOutputTensors( CGraphTensors& tensors, IMathEngine& mathEngine )
{
	CheckNeoOnnxSupport( InputTensor( tensors, 0 ).Data != nullptr, "non-constant input tensor", onnxNode );
	CheckNeoOnnxSupport( InputTensor( tensors, 0 ).Data->GetDataType() == CT_Int, "non-integer input tensor", onnxNode );

	OutputTensor( tensors, 0 ).Shape.SetSize( InputTensor( tensors, 0 ).Data->GetDataSize() );
	InputTensor( tensors, 0 ).Data->CopyTo( OutputTensor( tensors, 0 ).Shape.GetPtr() );

	CPtr<CDnnBlob> value = CDnnBlob::CreateVector( mathEngine, CT_Float, 1 );
	value->Clear();
	attributes.GetOptionalTensor( "value", *value );

	CBlobDesc outputBlobDesc( value->GetDataType() );
	for( int dimIndex = 0; dimIndex < OutputTensor( tensors, 0 ).Shape.Size(); ++dimIndex ) {
		outputBlobDesc.SetDimSize( dimIndex, OutputTensor( tensors, 0 ).Shape[dimIndex] );
	}

	OutputTensor( tensors, 0 ).Data = CDnnBlob::CreateBlob( mathEngine, value->GetDataType(), outputBlobDesc );
	if( OutputTensor( tensors, 0 ).Data->GetDataType() == CT_Float ) {
		OutputTensor( tensors, 0 ).Data->Fill( value->GetData().GetValue() );
	} else {
		OutputTensor( tensors, 0 ).Data->Fill<int>( value->GetData<int>().GetValue() );
	}
}

} // namespace NeoOnnx
