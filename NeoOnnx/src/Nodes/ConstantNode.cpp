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

#include "ConstantNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CConstantNode::CConstantNode( const onnx::NodeProto& constant, int opsetVersion ) :
	COpNode( constant, opsetVersion )
{
	// v1 - original
	// v9 - supported new data types
	// v11 - added "sparse_value" attribute
	// v12 - added new attributes: "value_*"
	CheckNeoOnnxSupport( OpsetVersion >= 1 && OpsetVersion <= MaxOpsetVersion, "opset version", *this );

	CheckOnnxProtocol( InputCount() == 0, "node must have no inputs", *this );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", *this );

	if( OpsetVersion >= 11 ) {
		// NeoOnnx supports only "value" attribute
		CheckNeoOnnxSupport( Attributes.Has( "value" ), "non-trivial value attribute", *this );
	}
}

void CConstantNode::AddLayers( const CObjectArray<const CTensorBase>& /* inputs */,
	CObjectArray<const CTensorBase>& /* outputs */, CDnn& /* dnn */ )
{
	NeoAssert( false );
}

void CConstantNode::CalculateOutput( const CObjectArray<const CTensorBase>& /* inputs */,
	CObjectArray<const CTensorBase>& outputs, IMathEngine& mathEngine )
{
	outputs[0] = Attributes.GetRequiredTensor( "value", mathEngine );
}

} // namespace NeoOnnx
