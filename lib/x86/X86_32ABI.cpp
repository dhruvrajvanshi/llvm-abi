#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Callee.hpp>
#include <llvm-abi/Caller.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypePromoter.hpp>

#include <llvm-abi/x86/X86_32Classifier.hpp>
#include <llvm-abi/x86/X86_32ABI.hpp>
#include <llvm-abi/x86/X86_32ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86_32ABI::X86_32ABI(llvm::Module* const module,
		               const llvm::Triple targetTriple)
		: llvmContext_(module->getContext()),
		targetTriple_(targetTriple),
		typeInfo_(llvmContext_) { }
		
		X86_32ABI::~X86_32ABI() { }
		
		std::string X86_32ABI::name() const {
			return "x86";
		}
		
		const ABITypeInfo& X86_32ABI::typeInfo() const {
			return typeInfo_;
		}
		
		llvm::CallingConv::ID X86_32ABI::getCallingConvention(const CallingConvention callingConvention) const {
			switch (callingConvention) {
				case CC_CDefault:
				case CC_CDecl:
				case CC_CppDefault:
					return llvm::CallingConv::C;
				case CC_StdCall:
					return llvm::CallingConv::X86_StdCall;
				case CC_FastCall:
					return llvm::CallingConv::X86_FastCall;
				case CC_ThisCall:
					return llvm::CallingConv::X86_ThisCall;
				case CC_Pascal:
					return llvm::CallingConv::X86_StdCall;
				case CC_VectorCall:
#if LLVMABI_LLVM_VERSION >= 306
					return llvm::CallingConv::X86_VectorCall;
#else
					throw std::runtime_error("VectorCall not supported by version of LLVM built against (need LLVM 3.6+.)");
#endif
				default:
					llvm_unreachable("Invalid calling convention for ABI.");
			}
		}
		
		llvm::FunctionType* X86_32ABI::getFunctionType(const FunctionType& functionType) const {
			X86_32Classifier classifier(typeInfo_,
			                            typeBuilder_,
			                            targetTriple_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                functionType.argumentTypes());
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(typeInfo(),
			                                                    argInfoArray);
			
			return llvm_abi::getFunctionType(llvmContext_,
			                                 typeInfo_,
			                                 functionType,
			                                 functionIRMapping);
		}
		
		llvm::AttributeList X86_32ABI::getAttributes(const FunctionType& functionType,
		                                             llvm::ArrayRef<Type> rawArgumentTypes,
		                                             const llvm::AttributeList existingAttributes) const {
			assert(rawArgumentTypes.size() >= functionType.argumentTypes().size());
			
			// Promote argument types (e.g. for varargs).
			TypePromoter typePromoter(typeInfo());
			const auto argumentTypes = typePromoter.promoteArgumentTypes(functionType,
			                                                             rawArgumentTypes);
			
			X86_32Classifier classifier(typeInfo_,
			                            typeBuilder_,
			                            targetTriple_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(typeInfo(),
			                                                    argInfoArray);
			
			return llvm_abi::getFunctionAttributes(llvmContext_,
			                                       typeInfo_,
			                                       functionIRMapping,
			                                       existingAttributes);
		}
		
		llvm::Value* X86_32ABI::createCall(Builder& builder,
		                                 const FunctionType& functionType,
		                                 std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
		                                 llvm::ArrayRef<TypedValue> rawArguments) const {
			TypePromoter typePromoter(typeInfo());
			
			// Promote any varargs arguments (that haven't already been
			// promoted). This changes char => int, float => double etc.
			const auto arguments = typePromoter.promoteArguments(builder,
			                                                     functionType,
			                                                     rawArguments);
			
			llvm::SmallVector<Type, 8> argumentTypes;
			for (const auto& value: arguments) {
				argumentTypes.push_back(value.type());
			}
			
			X86_32Classifier classifier(typeInfo_,
			                            typeBuilder_,
			                            targetTriple_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(typeInfo(),
			                                                    argInfoArray);
			
			Caller caller(typeInfo_,
			              functionType,
			              functionIRMapping,
			              builder);
			
			const auto encodedArguments = caller.encodeArguments(arguments);
			
			const auto returnValue = callBuilder(encodedArguments);
			
			return caller.decodeReturnValue(encodedArguments, returnValue);
		}
		
		static
		FunctionIRMapping computeIRMapping(const ABITypeInfo& typeInfo,
		                                   const TypeBuilder& typeBuilder,
		                                   const llvm::Triple targetTriple,
		                                   const FunctionType& functionType,
		                                   llvm::ArrayRef<Type> argumentTypes) {
			X86_32Classifier classifier(typeInfo,
			                            typeBuilder,
			                            targetTriple);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes);
			assert(argInfoArray.size() >= 1);
			
			return getFunctionIRMapping(typeInfo,
			                            argInfoArray);
		}
		
		class FunctionEncoder_x86: public FunctionEncoder {
		public:
			FunctionEncoder_x86(const ABITypeInfo& typeInfo,
			                    const TypeBuilder& typeBuilder,
			                    const llvm::Triple targetTriple,
			                    Builder& builder,
			                    const FunctionType& functionType,
			                    llvm::ArrayRef<llvm::Value*> pArguments)
			: builder_(builder),
			functionIRMapping_(computeIRMapping(typeInfo,
			                                    typeBuilder,
			                                    targetTriple,
			                                    functionType,
			                                    functionType.argumentTypes())),
			callee_(typeInfo,
			        functionType,
			        functionIRMapping_,
			        builder),
			encodedArguments_(pArguments.begin(), pArguments.end()),
			arguments_(callee_.decodeArguments(pArguments)) { }
			
			llvm::ArrayRef<llvm::Value*> arguments() const {
				return arguments_;
			}
			
			llvm::ReturnInst* returnValue(llvm::Value* const value) {
				const auto encodedReturnValue = callee_.encodeReturnValue(value,
				                                                          encodedArguments_);
				if (encodedReturnValue->getType()->isVoidTy()) {
					return builder_.getBuilder().CreateRetVoid();
				} else {
					return builder_.getBuilder().CreateRet(encodedReturnValue);
				}
			}
			
			llvm::Value* returnValuePointer() const {
				// TODO!
				return nullptr;
			}
			
		private:
			Builder& builder_;
			FunctionIRMapping functionIRMapping_;
			Callee callee_;
			llvm::SmallVector<llvm::Value*, 8> encodedArguments_;
			llvm::SmallVector<llvm::Value*, 8> arguments_;
			
		};
		
		std::unique_ptr<FunctionEncoder>
		X86_32ABI::createFunctionEncoder(Builder& builder,
		                               const FunctionType& functionType,
		                               llvm::ArrayRef<llvm::Value*> arguments) const {
			return std::unique_ptr<FunctionEncoder>(new FunctionEncoder_x86(typeInfo_,
			                                                                typeBuilder_,
			                                                                targetTriple_,
			                                                                builder,
			                                                                functionType,
			                                                                arguments));
		}
		
	}
	
}

