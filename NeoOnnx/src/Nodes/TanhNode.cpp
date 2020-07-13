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

#include "TanhNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CTanhNode::CTanhNode( const onnx::NodeProto& tanh, int opsetVersion, IMathEngine& /*mathEngine*/ ) :
	COpNode( tanh, opsetVersion )
{
	// The differences between versions are in supported data types and legacy optimization attributes
	CheckNeoOnnxSupport( opsetVersion >= 1 && opsetVersion <= MaxOpsetVersion, "opset version", tanh );

	CheckOnnxProtocol( input.Size() == 1, "node must have 1 input", tanh );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", tanh );
}

void CTanhNode::CalcOutputShape()
{
	InputTensor( 0 ).Shape.CopyTo( output[0].Shape );
}

void CTanhNode::CalcOutputData()
{
	CheckNeoOnnxSupport( InputTensor( 0 ).Data == nullptr, "output pre-calculation", onnxNode );
	// The output[0].Data was already set to nullptr in default constructor.
}

void CTanhNode::MarkTensorDims()
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

void CTanhNode::AddLayers( CDnn& dnn )
{
	CPtr<CTanhLayer> tanh = new CTanhLayer( dnn.GetMathEngine() );
	tanh->SetName( "NeoMLLayer" + Str( dnn.GetLayerCount() ) );

	tanh->Connect( 0, InputLayer( 0 ), InputLayerIndex( 0 ) );
	
	dnn.AddLayer( *tanh );

	neoMLInputInfo.Add( CNeoMLInputInfo( tanh, 0 ) );
}

} // namespace NeoOnnx
