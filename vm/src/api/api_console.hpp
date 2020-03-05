//
// Created by null on 18-8-30.
//

#ifndef BOXED_API_CONSOLE_HPP
#define BOXED_API_CONSOLE_HPP
#include "api_aware_base.hpp"
#include "../utils/null_terminated_ptr.hpp"
#include "Runtime/Intrinsics.h"
#include "Runtime/Runtime.h"
#include "IR/IR.h"
#include "IR/Types.h"



namespace vm_api{

	Intrinsics::Module& getIntrinsicModule_console(){
		static Intrinsics::Module module;
		return module;
	}

	/*
	static void printa(Runtime::ContextRuntimeData* contextRuntimeData,int a);

	static Intrinsics::Function printa_Intrinsic(
			getIntrinsicModule_console(),
			"printa",
            (void*)&printa,
			Intrinsics::inferIntrinsicFunctionType(&printa),
			IR::CallingConvention::intrinsic);

	static void printa(Runtime::ContextRuntimeData* contextRuntimeData,int a){

	}*/

    DEFINE_INTRINSIC_FUNCTION(console,"printb",void,printb,int b){

	}

}
#endif //BOXED_API_CONSOLE_HPP
