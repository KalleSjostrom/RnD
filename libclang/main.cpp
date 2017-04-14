#ifdef _MSC_VER
	#pragma comment(lib, "libclang.lib")
#endif
#include "clang-c/Index.h"

#include <stdio.h>
#define LOG(fmt, ...) { fprintf(stderr, fmt "\n", __VA_ARGS__); }
#define LOG_LINE(fmt, ...) { fprintf(stderr, fmt, __VA_ARGS__); }

#define LOG2(msg) { fprintf(stderr, msg "\n"); }
#define LOG_LINE2(msg) { fprintf(stderr, msg); }

#include "main.h"

void output_diagnostic(CXTranslationUnit TU) {
	for (int i = 0, n = clang_getNumDiagnostics(TU); i < n; ++i) {
		CXDiagnostic diag = clang_getDiagnostic(TU, i);
		CXString string = clang_formatDiagnostic(
			diag, clang_defaultDiagnosticDisplayOptions());
		LOG("%s\n", clang_getCString(string));
		clang_disposeString(string);
	}
}

// #include "trans.cpp"

#if _MSC_VER
	#define __attribute__(x)
#endif

CXChildVisitResult print_visitor(CXCursor cursor, CXCursor parent,
	                              CXClientData data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);
	if (clang_Location_isFromMainFile(location) == 0)
		return CXChildVisit_Continue;

	// CXCursor_FunctionDecl
	// if (cursor.kind != CXCursor_StructDecl)
	// if (!clang_isAttribute(cursor.kind))
		// return CXChildVisit_Continue;

	// CXTranslationUnit TU = (CXTranslationUnit)data;
	// CXToken *tokens;
	// unsigned num_tokens;
	// clang_tokenize(TU, range, &tokens, &num_tokens);

	unsigned line, column;
	clang_getSpellingLocation(location, 0, &line, &column, 0);

	const char *kind_str = get_cursor_kind_string(cursor.kind);

	// CXString name = clang_getCursorDisplayName(cursor);
	CXString name = clang_getCursorSpelling(cursor);
	const char *name_str = clang_getCString(name);

	LOG_LINE("%d: [%s] %s", line, kind_str, name_str);

	clang_disposeString(name);

	CXString brief_comment = clang_Cursor_getBriefCommentText(cursor);
	const char* brief_comment_str = clang_getCString(brief_comment);
	if (brief_comment_str != NULL && brief_comment_str[0] != '\0')
		LOG_LINE(" %s", brief_comment_str);
	clang_disposeString(brief_comment);

	// CXString raw_comment = clang_Cursor_getRawCommentText(cursor);
	// const char *raw_comment_str = clang_getCString(raw_comment);
	// if (raw_comment_str != NULL && raw_comment_str[0] != '\0')
	// 	LOG_LINE(" %s", raw_comment_str);
	// clang_disposeString(raw_comment);

	LOG_LINE2("\n");


	// CXChildVisit_Break
	return CXChildVisit_Recurse;
}

int main(int argc, char** argv) {
	(void)argc; (void)argv;

	CXIndex index = clang_createIndex(0, 0);
	if (!index) {
		LOG2("Failed to create index");
		return 0;
	}

	// const char *source_filename = "test.cpp";
	// CXTranslationUnit TU = clang_createTranslationUnitFromSourceFile(
	// 	index, source_filename,
	// 	0, NULL, 0, NULL);

	CXTranslationUnit TU = clang_parseTranslationUnit(
		index, 0, argv, argc, 0, 0, CXTranslationUnit_None);

	if (!TU) {
		LOG2("Failed to create translation unit");
		clang_disposeIndex(index);
		return 0;
	}

	{
		CXCursor cursor = clang_getTranslationUnitCursor(TU);

		if (clang_visitChildren(cursor, print_visitor, (CXClientData)TU) != 0) {
			LOG2("Ended prematurely");
		}
	}

	clang_disposeTranslationUnit(TU);
	clang_disposeIndex(index);

	return 0;
}
