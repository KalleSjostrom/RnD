@echo off

ml64.exe /nologo /c /Fojump_fcontext.obj asm/jump_x86_64_ms_pe_masm.asm
ml64.exe /nologo /c /Fomake_fcontext.obj asm/make_x86_64_ms_pe_masm.asm
cl main.cpp jump_fcontext.obj make_fcontext.obj