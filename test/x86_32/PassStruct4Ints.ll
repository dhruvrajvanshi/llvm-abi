; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int, int })

declare void @callee(i32, i32, i32, i32)

define void @caller(i32, i32, i32, i32) {
  %expand.source.arg = alloca { i32, i32, i32, i32 }, align 4
  %expand.dest.arg = alloca { i32, i32, i32, i32 }, align 4
  %5 = getelementptr { i32, i32, i32, i32 }* %expand.dest.arg, i32 0, i32 0
  store i32 %0, i32* %5, align 4
  %6 = getelementptr { i32, i32, i32, i32 }* %expand.dest.arg, i32 0, i32 1
  store i32 %1, i32* %6, align 4
  %7 = getelementptr { i32, i32, i32, i32 }* %expand.dest.arg, i32 0, i32 2
  store i32 %2, i32* %7, align 4
  %8 = getelementptr { i32, i32, i32, i32 }* %expand.dest.arg, i32 0, i32 3
  store i32 %3, i32* %8, align 4
  %9 = load { i32, i32, i32, i32 }* %expand.dest.arg, align 4
  store { i32, i32, i32, i32 } %9, { i32, i32, i32, i32 }* %expand.source.arg, align 4
  %10 = getelementptr { i32, i32, i32, i32 }* %expand.source.arg, i32 0, i32 0
  %11 = load i32* %10, align 4
  %12 = getelementptr { i32, i32, i32, i32 }* %expand.source.arg, i32 0, i32 1
  %13 = load i32* %12, align 4
  %14 = getelementptr { i32, i32, i32, i32 }* %expand.source.arg, i32 0, i32 2
  %15 = load i32* %14, align 4
  %16 = getelementptr { i32, i32, i32, i32 }* %expand.source.arg, i32 0, i32 3
  %17 = load i32* %16, align 4
  call void @callee(i32 %11, i32 %13, i32 %15, i32 %17)
  ret void
}
