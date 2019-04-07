@echo off

pushd "../compiler/bin/"
dir
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_1_A"
popd

pushd "../engine/bin/"
start engine.exe ../../reloader_test/out reloader_test
popd

SET /P variable=Press Enter for CASE_1_B
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_1_B"
popd

SET /P variable=Press Enter for CASE_1_C
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_1_C"
popd

SET /P variable=Press Enter for CASE_1_D
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_1_D"
popd

SET /P variable=Press Enter for CASE_2_A
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_2_A"
popd

SET /P variable=Press Enter for CASE_2_B
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_2_B"
popd

SET /P variable=Press Enter for CASE_2_C
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_2_C"
popd

SET /P variable=Press Enter for CASE_2_D
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_2_D"
popd

SET /P variable=Press Enter for CASE_3_A
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_3_A"
popd

SET /P variable=Press Enter for CASE_3_B
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_3_B"
popd

SET /P variable=Press Enter for CASE_3_C
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_3_C"
popd

SET /P variable=Press Enter for CASE_3_D
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_3_D"
popd

SET /P variable=Press Enter for CASE_4_A
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_4_A"
popd

SET /P variable=Press Enter for CASE_4_B
pushd "../compiler/bin/"
compiler.exe --project ../../reloader_test/reloader_test.project --command_line "-D CASE_4_B"
popd



