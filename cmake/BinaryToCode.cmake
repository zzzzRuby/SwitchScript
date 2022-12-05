
set(BINARY_TO_CODE_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})
function(binary_to_code)
	if (APPLE)
		set(type apple)
		set(suffix .S)
	elseif (MSVC OR EMSCRIPTEN)
		set(type c)
		set(suffix .c)
	else()
		set(type gas)
		set(suffix .S)
	endif()

	cmake_parse_arguments("args" "EXPORT" "INPUT;OUTPUT;SYMBOL;GENERATED_SOURCES" "" ${ARGN})
	find_package(PythonInterp REQUIRED 3)
	set(script ${BINARY_TO_CODE_LIST_DIR}/BinaryToCode.py)
	add_custom_command(
		OUTPUT ${args_OUTPUT}${suffix} ${args_OUTPUT}.h
		COMMAND ${PYTHON_EXECUTABLE} ${script}
			--type ${type} 
			--binary ${args_INPUT}
			--code ${args_OUTPUT}${suffix}
			--symbol-name ${args_SYMBOL} 
			--header ${args_OUTPUT}.h
			$<$<BOOL:${args_EXPORT}>:--export>
		DEPENDS ${script} ${args_INPUT}) 

	set(${args_GENERATED_SOURCES} ${args_OUTPUT}${suffix} ${args_OUTPUT}.h PARENT_SCOPE)
endfunction()

function(binary_to_cpp)
	cmake_parse_arguments("args" "" "INPUT;OUTPUT;SYMBOL;NAMESPACE" "" ${ARGN})

	find_package(PythonInterp REQUIRED 3)
	set(script ${BINARY_TO_CODE_LIST_DIR}/BinaryToCpp.py)
	add_custom_command(OUTPUT ${args_OUTPUT}
		COMMAND ${Python3_EXECUTABLE} ${script}
			-i ${args_INPUT} -o ${args_OUTPUT} -s ${args_SYMBOL} -n ${args_NAMESPACE}
		DEPENDS ${script} ${args_INPUT})
endfunction()