@echo off

ml64.exe /nologo /c /Fojump_fcontext.obj ../engine/utils/fibers/asm/jump_x86_64_ms_pe_masm.asm
ml64.exe /nologo /c /Fomake_fcontext.obj ../engine/utils/fibers/asm/make_x86_64_ms_pe_masm.asm
cl /GT -I ../include -I../ generator.cpp ../include/dirent.c jump_fcontext.obj make_fcontext.obj