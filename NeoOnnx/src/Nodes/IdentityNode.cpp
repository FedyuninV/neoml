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

#include "IdentityNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CIdentityNode::CIdentityNode( const onnx::NodeProto& identity, int opsetVersion ) :
	COpNode( identity, opsetVersion )
{
	// v1 - original
	CheckNeoOnnxSupport( OpsetVersion >= 1 && OpsetVersion <= MaxOpsetVersion, "opset version", *this );

	CheckOnnxProtocol( InputCount() == 1, "node must have 1 input", *this );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", *this );
}

void CIdentityNode::AddLayers( const CObjectArray<const CTensorBase>& inputs,
	CDnn& /* dnn */, CObjectArray<const CTensorBase>& outputs )
{
	outputs[0] = inputs[0];
}

void CIdentityNode::CalculateOutput( const CObjectArray<const CTensorBase>& inputs,
	IMathEngine& /* mathEngine */, CObjectArray<const CTensorBase>& outputs )
{
	outputs[0] = inputs[0];
}

} // namespace NeoOnnx
