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

#include "common.h"
#pragma hdrstop

#include "Node.h"

#include "Nodes/AbsNode.h"
#include "Nodes/AddNode.h"
#include "Nodes/AveragePoolNode.h"
#include "Nodes/BatchNormalizationNode.h"
#include "Nodes/ClipNode.h"
#include "Nodes/ConcatNode.h"
#include "Nodes/ConstantNode.h"
#include "Nodes/ConstantOfShapeNode.h"
#include "Nodes/ConvNode.h"
#include "Nodes/EluNode.h"
#include "Nodes/FlattenNode.h"
#include "Nodes/GatherNode.h"
#include "Nodes/GemmNode.h"
#include "Nodes/GlobalAveragePoolNode.h"
#include "Nodes/LeakyReluNode.h"
#include "Nodes/LstmNode.h"
#include "Nodes/MaxPoolNode.h"
#include "Nodes/ReduceMeanNode.h"
#include "Nodes/ReluNode.h"
#include "Nodes/ReshapeNode.h"
#include "Nodes/ShapeNode.h"
#include "Nodes/SigmoidNode.h"
#include "Nodes/SliceNode.h"
#include "Nodes/SqueezeNode.h"
#include "Nodes/TanhNode.h"
#include "Nodes/UnsqueezeNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

#include <string>

namespace NeoOnnx {

static CMap<CString, TCreateOpNodeFunction>& getRegisteredNodes()
{
	static CMap<CString, TCreateOpNodeFunction> registeredNodes;
	return registeredNodes;
}

void RegisterNode( const char* opName, TCreateOpNodeFunction function )
{
	NeoAssert( !getRegisteredNodes().Has( opName ) );
	getRegisteredNodes().Add( opName, function );
}

namespace {

// Register all nodes
REGISTER_OP_NODE( CAbsNode, "Abs" )
REGISTER_OP_NODE( CAddNode, "Add" )
REGISTER_OP_NODE( CAveragePoolNode, "AveragePool" )
REGISTER_OP_NODE( CBatchNormalizationNode, "BatchNormalization" )
REGISTER_OP_NODE( CClipNode, "Clip" )
REGISTER_OP_NODE( CConcatNode, "Concat" )
REGISTER_OP_NODE( CConstantNode, "Constant" )
REGISTER_OP_NODE( CConstantOfShapeNode, "ConstantOfShape" )
REGISTER_OP_NODE( CConvNode, "Conv" )
REGISTER_OP_NODE( CEluNode, "Elu" )
REGISTER_OP_NODE( CFlattenNode, "Flatten" )
REGISTER_OP_NODE( CGatherNode, "Gather" )
REGISTER_OP_NODE( CGemmNode, "Gemm" )
REGISTER_OP_NODE( CGlobalAveragePoolNode, "GlobalAveragePool" )
REGISTER_OP_NODE( CLeakyReluNode, "LeakyRelu" )
REGISTER_OP_NODE( CLstmNode, "LSTM" )
REGISTER_OP_NODE( CMaxPoolNode, "MaxPool" )
REGISTER_OP_NODE( CReduceMeanNode, "ReduceMean" )
REGISTER_OP_NODE( CReluNode, "Relu" )
REGISTER_OP_NODE( CReshapeNode, "Reshape" )
REGISTER_OP_NODE( CShapeNode, "Shape" )
REGISTER_OP_NODE( CSigmoidNode, "Sigmoid" )
REGISTER_OP_NODE( CSliceNode, "Slice" )
REGISTER_OP_NODE( CSqueezeNode, "Squeeze" )
REGISTER_OP_NODE( CTanhNode, "Tanh" )
REGISTER_OP_NODE( CUnsqueezeNode, "Unsqueeze" )

} // namespace

//---------------------------------------------------------------------------------------------------------------------

CNode::CNode( int inputCount, int outputCount )
{
	input.SetSize( inputCount );
	output.SetSize( outputCount );
}

int CNode::OutputCount() const
{
	return output.Size();
}

const CTensor& CNode::InputTensor( int index ) const
{
	CheckNeoOnnxInternal( index >= 0 && index < input.Size(), "attempt to access non-existing input" );
	CheckNeoOnnxInternal( input[index].InputNode != nullptr, "attempt to acces empty input" );
	return input[index].InputNode->output[input[index].OutputIndex];
}

CTensor& CNode::InputTensor( int index )
{
	CheckNeoOnnxInternal( index >= 0 && index < input.Size(), "attempt to access non-existing input" );
	CheckNeoOnnxInternal( input[index].InputNode != nullptr, "attempt to acces empty input" );
	return input[index].InputNode->output[input[index].OutputIndex];
}

void CNode::SetInput( int index, const CNode::CInputInfo& inputInfo )
{
	CheckNeoOnnxInternal( index >= 0 && index < input.Size(), "attempt to set non-existing input" );
	CheckNeoOnnxInternal( input[index].InputNode == nullptr && input[index].OutputIndex == NotFound,
		"attempt to set already-defined input" );
	CheckNeoOnnxInternal( inputInfo.InputNode != nullptr, "attempt to set an input to nullptr" );
	CheckNeoOnnxInternal( inputInfo.OutputIndex >= 0, "attempt to set an input with incorrect index" );
	input[index] = inputInfo;
}

const CNode::CNeoMLInputInfo& CNode::InputInfo( int index ) const
{
	CheckNeoOnnxInternal( index >= 0 && index < input.Size(), "attempt to access non-existing input" );
	CheckNeoOnnxInternal( input[index].InputNode != nullptr, "attempt to acces empty input" );
	const CNode& inputNode = *input[index].InputNode;
	const int inputNodeOutputIndex = input[index].OutputIndex;

	NeoAssert( inputNode.neoMLInputInfo.Size() == inputNode.output.Size() );
	NeoAssert( inputNodeOutputIndex >= 0 && inputNodeOutputIndex < inputNode.output.Size() );
	NeoAssert( inputNode.neoMLInputInfo[inputNodeOutputIndex].Layer != nullptr );
	return inputNode.neoMLInputInfo[inputNodeOutputIndex];
}

const CBaseLayer& CNode::InputLayer( int index ) const
{
	return *InputInfo( index ).Layer;
}

int CNode::InputLayerIndex( int index ) const
{
	return InputInfo( index ).OutputIndex;
}

//---------------------------------------------------------------------------------------------------------------------

COpNode::COpNode( const onnx::NodeProto& _onnxNode, int _opsetVersion ) :
	CNode( _onnxNode.input_size(), _onnxNode.output_size() ),
	opsetVersion( _opsetVersion ),
	attributes( _onnxNode ),
	onnxNode( _onnxNode )
{
	input.SetSize( onnxNode.input_size() );
	output.SetSize( _onnxNode.output_size() );
}

COpNode* COpNode::CreateOpNode( const onnx::NodeProto& onnxNode, int opsetVersion, IMathEngine& mathEngine )
{
	TMapPosition pos = getRegisteredNodes().GetFirstPosition( onnxNode.op_type() );
	CheckNeoOnnxSupport( pos != NotFound, CString( "operator " ) + onnxNode.op_type().c_str() );
	return getRegisteredNodes().GetValue( pos )( onnxNode, opsetVersion, mathEngine );
}


} // namespace NeoOnnx
