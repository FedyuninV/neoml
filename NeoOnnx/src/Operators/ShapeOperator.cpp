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

#include "ShapeOperator.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CShapeOperator::CShapeOperator( const onnx::NodeProto& shape, int opsetVersion ) :
	COperator( shape, opsetVersion )
{
	// This operator doesn't have multiple versions
	CheckNeoOnnxSupport( OpsetVersion >= 1 && OpsetVersion <= MaxOpsetVersion, "opset version", *this );

	CheckOnnxProtocol( InputCount() == 1, "operator must have 1 input", *this );
	CheckOnnxProtocol( OutputCount() == 1, "operator must have 1 output", *this );
}

void CShapeOperator::GetOutputTensors( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const
{
	NeoAssert( inputs[0] != nullptr );
	const CTensorShape& inputShape = inputs[0]->Shape();
	CTensorLayout outputLayout( 1 );
	CBlobDesc outputBlobDesc( CT_Int );
	outputBlobDesc.SetDimSize( outputLayout[0], inputShape.Size() );
	CPtr<CDnnBlob> outputBlob = CDnnBlob::CreateBlob( dnn.GetMathEngine(), CT_Int, outputBlobDesc );
	outputBlob->CopyFrom( inputShape.GetPtr() );
	outputs[0] = new CDataTensor( { inputShape.Size() }, outputLayout, *outputBlob );
}

} // namespace NeoOnnx