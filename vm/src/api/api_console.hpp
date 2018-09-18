//
// Created by null on 18-8-30.
//

#ifndef BOXED_API_CONSOLE_HPP
#define BOXED_API_CONSOLE_HPP
#include "api_aware_base.hpp"
#include "../utils/null_terminated_ptr.hpp"
#include "WAVM/Include/Runtime/Intrinsics.h"
#include "WAVM/Include/Runtime/Runtime.h"
#include "WAVM/Include/IR/IR.h"



namespace vm_api{


    #define KKK_INTRINSIC_FUNCTION(module, nameString, Result, cName, ...)                          \
	static Result cName(Runtime::ContextRuntimeData* contextRuntimeData, ##__VA_ARGS__);           \
	static Intrinsics::Function cName##Intrinsic(getIntrinsicModule_##module(),                    \
												 nameString,                                       \
												 (void*)&cName,                                    \
												 Intrinsics::inferIntrinsicFunctionType(&cName),   \
												 Runtime::CallingConvention::intrinsic);           \
	static Result cName(Runtime::ContextRuntimeData* contextRuntimeData, ##__VA_ARGS__)



	Intrinsics::Module& getIntrinsicModule_console(){
		static Intrinsics::Module module;
		return module;
	}

	static void printa(Runtime::ContextRuntimeData* contextRuntimeData,int a);

	static Intrinsics::Function printa_Intrinsic(
			getIntrinsicModule_console(),
			"printa",
			printa,
			Intrinsics::inferIntrinsicFunctionType((void*)printa),
			Runtime::CallingConvention::intrinsic);

	static void printa(Runtime::ContextRuntimeData* contextRuntimeData,int a){

	}

}
#endif //BOXED_API_CONSOLE_HPP
