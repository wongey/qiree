; ModuleID = 'dynamicbv'
source_filename = "dynamicbv"

; ModuleID = 'BernsteinVazirani'
source_filename = "bv_algorithm"

%Qubit = type opaque
%Result = type opaque

define void @main() #0 {
entry:
  ; Initialize qubits
  call void @__quantum__qis__h__body(%Qubit* null) ; apply Hadamard to query qubit
  call void @__quantum__qis__x__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ; set ancillary qubit
  call void @__quantum__qis__h__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ;
  

  ; Apply CNOT for bit '1'
  call void @__quantum__qis__cnot__body(%Qubit* null, %Qubit* inttoptr (i64 1 to %Qubit*)) ; kickback phase on q0
  call void @__quantum__qis__h__body(%Qubit* null) ; correcting eigenvalue
  
  ; Mid-circuit measurement 
  call void @__quantum__qis__mz__body(%Qubit* null, %Result* null) ; from this we get the first bit
  call i1 @__quantum__qis__read_result__body(%Result* null)
  call void @__quantum__qis__mz__body(%Qubit* inttoptr (i64 1 to %Qubit*), %Result* inttoptr (i64 1 to %Result*)) ; just to reset ancillary qubit
  call i1 @__quantum__qis__read_result__body(%Result* inttoptr (i64 1 to %Result*))  
  
  ; Output the results
  call void @__quantum__rt__array_record_output(i64 2, i8* null)
  call void @__quantum__rt__result_record_output(%Result* null, i8* null)
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 1 to %Result*), i8* null)

  ; Initialize qubits 
  call void @__quantum__qis__x__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ; set ancillary qubit
  call void @__quantum__qis__h__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ;
  call void @__quantum__qis__h__body(%Qubit* null) ; apply Hadamard to query qubit

  ; Apply Identiry for bit '0'
  ; Nothing

  ; Mid-circuit measurement 
  call void @__quantum__qis__mz__body(%Qubit* null, %Result* null) ; from this we get the first bit
  call i1 @__quantum__qis__read_result__body(%Result* null)
  call void @__quantum__qis__mz__body(%Qubit* inttoptr (i64 1 to %Qubit*), %Result* inttoptr (i64 1 to %Result*)) ; just to reset ancillary qubit
  call i1 @__quantum__qis__read_result__body(%Result* inttoptr (i64 1 to %Result*))  
  
  ; Output the results
  call void @__quantum__rt__array_record_output(i64 2, i8* null)
  call void @__quantum__rt__result_record_output(%Result* null, i8* null)
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 1 to %Result*), i8* null)

  ; Initialize qubits
  call void @__quantum__qis__x__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ; set ancillary qubit
  call void @__quantum__qis__h__body(%Qubit* inttoptr (i64 1 to %Qubit*)) ;
  call void @__quantum__qis__h__body(%Qubit* null) ; apply Hadamard to query qubit

  ; Apply CNOT for bit '1'
  call void @__quantum__qis__cnot__body(%Qubit* null, %Qubit* inttoptr (i64 1 to %Qubit*)) ; kickback phase on q0
  call void @__quantum__qis__h__body(%Qubit* null) ; correcting eigenvalue
  
  ; Mid-circuit measurement 
  call void @__quantum__qis__mz__body(%Qubit* null, %Result* null) ; from this we get the first bit
  call i1 @__quantum__qis__read_result__body(%Result* null)
  call void @__quantum__qis__mz__body(%Qubit* inttoptr (i64 1 to %Qubit*), %Result* inttoptr (i64 1 to %Result*)) ; just to reset ancillary qubit
  call i1 @__quantum__qis__read_result__body(%Result* inttoptr (i64 1 to %Result*))  
  
  ; Output the results
  call void @__quantum__rt__array_record_output(i64 2, i8* null)
  call void @__quantum__rt__result_record_output(%Result* null, i8* null)
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 1 to %Result*), i8* null)

  ret void
}

; Declaration of quantum operations
declare void @__quantum__qis__h__body(%Qubit*)
declare void @__quantum__qis__x__body(%Qubit*)
declare void @__quantum__qis__cnot__body(%Qubit*, %Qubit*)
declare void @__quantum__qis__mz__body(%Qubit*, %Result*)
declare i1 @__quantum__qis__read_result__body(%Result*)

; Quantum runtime functions for managing qubits and results
declare %Qubit* @__quantum__rt__qubit_allocate()
declare %Result* @__quantum__rt__result_allocate()
declare void @__quantum__rt__qubit_release(%Qubit*)
declare void @__quantum__rt__result_release(%Result*)
declare void @__quantum__rt__result_record_output(%Result*, i8*)
declare void @__quantum__rt__array_record_output(i64, i8*)



attributes #0 = { "entry_point" "num_required_qubits"="2" "num_required_results"="2" "output_labeling_schema" "qir_profiles"="custom" }
attributes #1 = { "irreversible" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"qir_major_version", i32 1}
!1 = !{i32 7, !"qir_minor_version", i32 0}
!2 = !{i32 1, !"dynamic_qubit_management", i1 false}
!3 = !{i32 1, !"dynamic_result_management", i1 false}

