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

#include "LeakyReluNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CLeakyReluNode::CLeakyReluNode( const onnx::NodeProto& leakyRelu, int opsetVersion, IMathEngine& /*mathEngine*/ ) :
	CNode( leakyRelu, opsetVersion ),
	alpha( attributes.GetOptionalFloat( "alpha", 0.01f ) )
{
	// v1 - original ver.
	// v6 - removed legacy optimization attribute
	CheckNeoOnnxSupport( opsetVersion >= 1 && opsetVersion <= MaxOpsetVersion, "opset version", leakyRelu );

	CheckOnnxProtocol( input.Size() == 1, "node must have 1 input", leakyRelu );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", leakyRelu );
}

void CLeakyReluNode::CalcOutputShape()
{
	InputTensor( 0 ).Shape.CopyTo( output[0].Shape );
}

void CLeakyReluNode::CalcOutputData()
{
	CheckNeoOnnxSupport( InputTensor( 0 ).Data == nullptr, "output pre-calculation", onnxNode );
	// The output[0].Data was already set to nullptr in default constructor.
}

void CLeakyReluNode::MarkTensorDims()
{
	if( !InputTensor( 0 ).Dim.IsEmpty() ) {
		CheckNeoOnnxInternal( output[0].SetTensorDim( InputTensor( 0 ).Dim ),
			"marking output dimensions failed", onnxNode );
	}

	if( !output[0].Dim.IsEmpty() ) {
		CheckNeoOnnxInternal( InputTensor( 0 ).SetTensorDim( output[0].Dim ),
			"marking input dimensions failed", onnxNode );
	}
}

void CLeakyReluNode::AddLayers( CDnn& dnn )
{
	CPtr<CLeakyReLULayer> leakyRelu = new CLeakyReLULayer( dnn.GetMathEngine() );
	leakyRelu->SetName( "NeoMLLayer" + Str( dnn.GetLayerCount() ) );

	leakyRelu->Connect( 0, InputLayer( 0 ), InputLayerIndex( 0 ) );
	leakyRelu->SetAlpha( alpha );
	
	dnn.AddLayer( *leakyRelu );

	neoMLInputInfo.Add( CNeoMLInputInfo( leakyRelu, 0 ) );
}

} // namespace NeoOnnx
