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

// Graph input node
class CGraphInput : public CNode {
public:
	explicit CGraphInput( const onnx::ValueInfoProto& input );

	// CNode methods' realizations
	bool CanCalculateOutput( const CObjectArray<const CTensorBase>& inputs ) const override;
	void AddLayers( const CObjectArray<const CTensorBase>& inputs,
		CDnn& dnn, CObjectArray<const CTensorBase>& outputs ) override;
	void CalculateOutput( const CObjectArray<const CTensorBase>& inputs,
		IMathEngine& mathEngine, CObjectArray<const CTensorBase>& outputs ) override;

private:
	const onnx::ValueInfoProto& valueInfo; // Input information from onnx
};

} // namespace NeoOnnx
