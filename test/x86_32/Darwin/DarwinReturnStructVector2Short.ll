; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {<2 x short>} ()

declare i32 @callee()

define i32 @caller() {
  %coerce1 = alloca { <2 x i16> }, align 4
  %coerce = alloca { <2 x i16> }, align 4
  %1 = call i32 @callee()
  %coerce.dive = getelementptr { <2 x i16> }* %coerce, i32 0, i32 0
  %2 = bitcast <2 x i16>* %coerce.dive to i32*
  store i32 %1, i32* %2, align 1
  %3 = load { <2 x i16> }* %coerce, align 4
  store { <2 x i16> } %3, { <2 x i16> }* %coerce1, align 4
  %coerce.dive2 = getelementptr { <2 x i16> }* %coerce1, i32 0, i32 0
  %4 = bitcast <2 x i16>* %coerce.dive2 to i32*
  %5 = load i32* %4, align 1
  ret i32 %5
}
