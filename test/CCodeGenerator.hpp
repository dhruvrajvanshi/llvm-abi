#ifndef LLVM_ABI_TEST_CCODEGENERATOR_HPP
#define LLVM_ABI_TEST_CCODEGENERATOR_HPP

#include <sstream>
#include <string>

namespace llvm_abi {
	
	class ABITypeInfo;
	struct TestFunctionType;
	class Type;
	
	/**
	 * \brief C code generator.
	 * 
	 * This class generates C code for a pair of callee and caller functions
	 * for ABI function types. This allows function types to be passed to
	 * a C compiler (Clang) and the output to be compared against the output
	 * of the LLVM-ABI library.
	 */
	class CCodeGenerator {
	public:
		CCodeGenerator(const ABITypeInfo& typeInfo);
		
		std::string generatedSourceCode() const;
		
		std::string emitType(const Type& type);
		
		size_t emitFunctionTypes(const TestFunctionType& testFunctionType);
		
		void emitCalleeFunction(const TestFunctionType& testFunctionType,
		                        size_t functionId);
		
		void emitCallerFunction(const TestFunctionType& testFunctionType,
		                        size_t functionId);
		
		void emitCalleeAndCallerFunctions(const TestFunctionType& functionType);
		
	private:
		std::ostringstream sourceCodeStream_;
		const ABITypeInfo& typeInfo_;
		size_t arrayId_;
		size_t functionId_;
		size_t structId_;
		size_t unionId_;
		size_t vectorId_;
		
	};
	
}

#endif