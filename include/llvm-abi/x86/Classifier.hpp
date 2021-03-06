#ifndef LLVMABI_X86_64_CLASSIFIER_HPP
#define LLVMABI_X86_64_CLASSIFIER_HPP

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		class Classification;
		
		class Classifier {
		public:
			Classifier(const ABITypeInfo& typeInfo);
			
			Classification classify(const Type type,
			                        bool isNamedArg);
			
			ArgInfo classifyType(Type type,
			                     bool isArgument,
			                     unsigned freeIntRegs,
			                     unsigned &neededInt,
			                     unsigned &neededSse,
			                     bool isNamedArg);
			
			ArgInfo classifyReturnType(Type type);
			
			llvm::SmallVector<ArgInfo, 8>
			classifyFunctionType(const FunctionType& functionType,
			                     llvm::ArrayRef<Type> argumentTypes);
			
		private:
			const ABITypeInfo& typeInfo_;
			
		};
		
	}
	
}

#endif
