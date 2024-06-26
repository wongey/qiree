; ModuleID = 'multiple-functions'
source_filename = "multiple-functions"

%Qubit = type opaque
%String = type opaque

@0 = internal constant [3 x i8] c"()\00"

declare %Qubit* @__quantum__rt__qubit_allocate()

define internal void @SimpleLoop__Main__body() {
entry:
  call void @SimpleLoop__MiniCircuit__body(%Qubit* null, %Qubit* nonnull inttoptr (i64 1 to %Qubit*))
  call void @SimpleLoop__MiniCircuit__body(%Qubit* null, %Qubit* nonnull inttoptr (i64 1 to %Qubit*))
  ret void
}

define internal void @SimpleLoop__MiniCircuit__body(%Qubit* %q1, %Qubit* %q2) {
entry:
  call void @Intrinsic__Function__H__body(%Qubit* %q1)
  call void @Intrinsic__Function__H__body(%Qubit* %q2)
  call void @Intrinsic__Function__CNOT__body(%Qubit* %q1, %Qubit* %q2)
  ret void
}

declare void @__quantum__rt__qubit_release(%Qubit*)

define internal void @Intrinsic__Function__H__body(%Qubit* %qubit) {
entry:
  call void @__quantum__qis__h__body(%Qubit* %qubit)
  ret void
}

define internal void @Intrinsic__Function__CNOT__body(%Qubit* %control, %Qubit* %target) {
entry:
  call void @__quantum__qis__cnot__body(%Qubit* %control, %Qubit* %target)
  ret void
}

declare void @__quantum__qis__cnot__body(%Qubit*, %Qubit*)

declare void @__quantum__qis__h__body(%Qubit*)

define void @SimpleLoop__Main() #1 {
entry:
  call void @SimpleLoop__Main__body()
  ret void
}

declare %String* @__quantum__rt__string_create(i8*)

declare void @__quantum__rt__message(%String*)

declare void @__quantum__rt__string_update_reference_count(%String*, i32)

attributes #0 = { "InteropFriendly" }
attributes #1 = { "EntryPoint" }

