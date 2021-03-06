#pragma once

enum TokenType {
	TokenType_Unknown,
	TokenType_String,
	TokenType_Number,
	TokenType_NumberLiteral,
	TokenType_Identifier,
	TokenType_Namespace,
	TokenType_CommandMarker,
	TokenType_CommandMarkerBlock,
	TokenType_Line,
	TokenType_IDStringMarker,
	TokenType_ID32StringMarker,
	TokenType_ID64StringMarker,
};

enum TokenizerFlags {
	TokenizerFlags_Verbatim = 1<<0, // If set, will not do any actual conversions between strings and numbers.
	TokenizerFlags_KeepLineComments = 1<<1,
};

///////// PARSING /////////
struct Tokenizer {
	char *at;
	char *debug_start;
	u32 debug_size;
	u32 flags;
};

// Only for debugging purposes
struct ParserContext {
	Tokenizer *tokenizer;
	char *filepath;
	char *source;
};
static unsigned global_error = 0;
static __declspec(thread) ParserContext *global_last_parser_context;

struct Token {
	i32 type;
	union {
		struct {
			f32 number;
		};
		struct {
			char *text;
			i32 length;
		};
		struct {
			String string;
		};
	};
};

__forceinline char *type_tostring(int token_type, char *type_buffer) {
	switch (token_type) {
		case TokenType_Unknown            : { return "Unknown";            } break;
		case TokenType_String             : { return "String";             } break;
		case TokenType_Number             : { return "Number";             } break;
		case TokenType_NumberLiteral      : { return "NumberLiteral";      } break;
		case TokenType_Identifier         : { return "Identifier";         } break;
		case TokenType_Namespace          : { return "Namespace";          } break;
		case TokenType_CommandMarker      : { return "CommandMarker";      } break;
		case TokenType_CommandMarkerBlock : { return "CommandMarkerBlock"; } break;
		case TokenType_Line               : { return "Line";               } break;
		case TokenType_IDStringMarker     : { return "IDStringMarker";     } break;
		case TokenType_ID32StringMarker   : { return "ID32StringMarker";   } break;
		case TokenType_ID64StringMarker   : { return "ID64StringMarker";   } break;
		default: {
			type_buffer[0] = (char)token_type;
			type_buffer[1] = '\0';
			return type_buffer;
		}
	}
}
__forceinline char *token_tostring(Token &token) {
	static char error_buffer[256];
	if (token.type == TokenType_Number) {
		snprintf(error_buffer, ARRAY_COUNT(error_buffer), "%g", (double)token.number);
	} else {
		snprintf(error_buffer, ARRAY_COUNT(error_buffer), "%.*s", (int)token.string.length, token.string.text);
	}
	return error_buffer;
}

__forceinline bool is_equal(Token a, Token b) {
	return a.string == b.string;
}

__forceinline void set_parser_context(ParserContext &context, Tokenizer *tokenizer, char *filepath) {
	context.tokenizer = tokenizer;
	context.filepath = filepath;
	context.source = tokenizer->at;

	global_last_parser_context = &context;
}

__forceinline Tokenizer make_tokenizer(char *at, size_t debug_size, u32 flags) {
	Tokenizer t = {};
	t.flags = flags;
	t.at = at;
	t.debug_start = at;
	t.debug_size = (u32)debug_size;
	return t;
}

__forceinline Token make_token(char *text, i32 length, char type) {
	Token t = {};
	t.type = type;
	t.length = length;
	t.text = text;
	return t;
}

__forceinline bool is_eos(char c) { return c == '\0'; }
__forceinline bool is_line_end(char c) { return c == '\n' || c == '\r'; }
__forceinline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
__forceinline bool is_numeric(char c) { return (c >= '0' && c <= '9'); }
__forceinline bool is_alpha_numeric(char c) { return is_alpha(c) || is_numeric(c); }
__forceinline bool is_number(char c) { return is_numeric(c) || c == 'x' || c == 'b' || c == '.'; } // 0.0 0.0f 0.0L 0x00 0b00
__forceinline bool is_whitespace(char c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || is_line_end(c)); }

__forceinline bool is_line_comment(char *at) { return (at[0] == '/' && at[1] == '/' && at[2] != '!') || (at[0] == '-' && at[1] == '-'); }
__forceinline bool is_block_comment_opening(char *at) { return at[0] == '/' && at[1] == '*' && at[2] != '!'; }
__forceinline bool is_block_comment_closing(char *at) { return at[0] == '*' && at[1] == '/'; }
__forceinline bool is_eos(char *at) { return at[0] == '\0'; }
__forceinline bool is_line_end(char *at) { return at[0] == '\n' || at[0] == '\r'; }
__forceinline bool is_whitespace(char *at) { return is_whitespace(at[0]); }

__forceinline bool is_line_comment(Tokenizer *tok) { return is_line_comment(tok->at); }
__forceinline bool is_block_comment_opening(Tokenizer *tok) { return is_block_comment_opening(tok->at); }
__forceinline bool is_block_comment_closing(Tokenizer *tok) { return is_block_comment_closing(tok->at); }
__forceinline bool is_eos(Tokenizer *tok) { return is_eos(tok->at); }
__forceinline bool is_line_end(Tokenizer *tok) { return is_line_end(tok->at); }
__forceinline bool is_whitespace(Tokenizer *tok) { return is_whitespace(tok->at); }

__forceinline bool try_get_id_marker(Tokenizer *tok, Token &token) {
	bool found = false;
	if (tok->at[0] == 'D') {
		if (tok->at[1] == '(') {
			found = true;
			token.type = TokenType_IDStringMarker;
			token.length = 3;
			tok->at += 2;
		} else if (tok->at[1] == '3' && tok->at[2] == '2' && tok->at[3] == '(') {
			found = true;
			token.type = TokenType_ID32StringMarker;
			token.length = 5;
			tok->at += 4;
		} else if (tok->at[1] == '6' && tok->at[2] == '4' && tok->at[3] == '(') {
			found = true;
			token.type = TokenType_ID64StringMarker;
			token.length = 5;
			tok->at += 4;
		}
	}
	return found;
}

__forceinline void consume_line(Tokenizer *tok) {
	while (!is_eos(tok) && !is_line_end(tok))
		tok->at++;
}

__forceinline bool try_consume_block_comment_opening(Tokenizer *tok) {
	bool is_opening = is_block_comment_opening(tok);
	if (is_opening)
		tok->at+=2;
	return is_opening;
}

__forceinline bool try_consume_block_comment_closing(Tokenizer *tok) {
	bool is_closing = is_block_comment_closing(tok);
	if (is_closing)
		tok->at+=2;
	return is_closing;
}

void consume_whitespace(Tokenizer *tok) {
	while (true) {
		if (is_whitespace(tok)) {
			tok->at++;
		} else if (!(tok->flags & TokenizerFlags_KeepLineComments) && is_line_comment(tok)) {
			tok->at += 2;
			consume_line(tok);
		} else if (try_consume_block_comment_opening(tok)) {
			while (!is_eos(tok) && !try_consume_block_comment_closing(tok))
				tok->at++;
		} else {
			break;
		}
	}
}

void trim_whitespaces(String &string) {
	// trim from the beginning
	while (is_whitespace(string.text[0])) {
		string.text++;
		string.length--;
	}

	// trim from the end
	while (is_whitespace(string.text[string.length-1])) {
		string.length--;
	}
}

void trim_non_characters_from_token(Token &token) {
	// trim from the beginning
	while (!is_alpha(token.text[0])) {
		token.text++;
		token.length--;
	}

	// trim from the end
	while (!is_alpha(token.text[token.length-1])) {
		token.length--;
	}
}

bool try_parse_number(char *at, f32 &result, int sign = 1, int integer = 0, bool do_fraction = false) {
	if (do_fraction) {
		int divisor = 1;
		int fraction = 0;
		while (is_numeric(at[0])) {
			char c = at[0];
			fraction = fraction*10 + (c - '0');
			divisor *= 10;
			at++;
		}
		result = sign * (integer + (fraction/(f32)divisor));
		return true;
	} else if (at[0] == '-') {
		sign = -1;
		at++;
		return try_parse_number(at, result, sign, integer, true);
	} else if (at[0] == '.') {
		at++;
		return try_parse_number(at, result, sign, integer, true);
	} else {
		while (is_numeric(at[0])) {
			char c = at[0];
			integer = integer*10 + (c - '0');
			at++;
		}
		if (at[0] == '.') {
			at++;
			return try_parse_number(at, result, sign, integer, true);
		} else {
			result = sign * integer;
			return true;
		}
	}
}

f32 parse_number(Tokenizer *tok, int sign, int integer, bool do_fraction) {
	if (do_fraction) {
		int divisor = 1;
		int fraction = 0;
		while (is_numeric(tok->at[0])) {
			char c = tok->at[0];
			fraction = fraction*10 + (c - '0');
			divisor *= 10;
			tok->at++;
		}

		f32 value = sign * (integer + (fraction/(f32)divisor));
		if (tok->at[0] == 'e') {
			tok->at++;
			divisor = 1;
			int exponent = 0;
            int exponent_sign = 1;
            if (tok->at[0] == '-') {
                exponent_sign = -1;
                tok->at++;
            }
			while (is_numeric(tok->at[0])) {
				char c = tok->at[0];
				exponent = exponent*10 + (c - '0');
				tok->at++;
			}
			value = value * powf(10, exponent * exponent_sign);
		}

		return value;
	} else if (tok->at[0] == '.') {
		tok->at++;
		return parse_number(tok, sign, integer, true);
	} else {
		while (is_numeric(tok->at[0])) {
			char c = tok->at[0];
			integer = integer*10 + (c - '0');
			tok->at++;
		}
		if (tok->at[0] == '.') {
			tok->at++;
			return parse_number(tok, sign, integer, true);
		} else {
			return sign * integer;
		}
	}
}

Token next_token(Tokenizer *tok) {
	if (tok->debug_start) {
		i64 debug_current_size = tok->at - tok->debug_start;
		ASSERT(debug_current_size <= tok->debug_size, "Tokenizer out of bounds!");
	}

	consume_whitespace(tok);

	char c = tok->at[0];
	Token token = make_token(tok->at, 1, c);
	tok->at++;
	if (c == '"') {
		token.type = TokenType_String;
		token.text = tok->at;
		while (!is_eos(tok) && tok->at[0] != '"') {
			if (tok->at[0] == '\\' && tok->at[1])
				tok->at++;
			tok->at++;
		}
		token.length = (i32)(tok->at - token.text);
		if (tok->at[0] == '"')
			tok->at++;
	} else if (c == '/' && tok->at[0] == '/' && tok->at[1] == '!') {
		token.type = TokenType_CommandMarker;
		token.length = 3;
		tok->at += 2;
	} else if (c == '/' && tok->at[0] == '*' && tok->at[1] == '!') {
		token.type = TokenType_CommandMarkerBlock;
		token.length = 3;
		tok->at += 2;
	} else if (c == 'I' && try_get_id_marker(tok, token)) {
	} else if (c == ':' && tok->at[0] == ':') {
		token.type = TokenType_Namespace;
		token.length = 2;
		tok->at++;
	} else if (is_alpha(c) || c == '_') {
		while (is_alpha_numeric(tok->at[0]) || tok->at[0] == '_')
			tok->at++;
		token.type = TokenType_Identifier;
		token.length = (i32)(tok->at - token.text);
	} else {
		if (tok->flags & TokenizerFlags_Verbatim) {
			if (is_numeric(c)) {
				while (is_numeric(tok->at[0]))
					tok->at++;
				token.type = TokenType_NumberLiteral;
				token.length = (i32)(tok->at - token.text);
			}
		} else {
			f32 number = 0;
			bool number_found = false;
			if (is_numeric(c)) {
				number = parse_number(tok, 1, c-'0', false);
				number_found = true;
			} else if (c == '-') {
				if (is_numeric(tok->at[0])) {
					number = parse_number(tok, -1, 0, false);
					number_found = true;
				} else if (tok->at[0] == '.' && is_numeric(tok->at[1])) {
					tok->at++;
					number = parse_number(tok, -1, 0, true);
					number_found = true;
				}
			} else if (c == '+') {
				if (is_numeric(tok->at[0])) {
					number = parse_number(tok, 1, 0, false);
					number_found = true;
				} else if (tok->at[0] == '.' && is_numeric(tok->at[1])) {
					tok->at++;
					number = parse_number(tok, 1, 0, true);
					number_found = true;
				}
			} else if (c == '.' && is_numeric(tok->at[0])) {
				number = parse_number(tok, 1, 0, true);
				number_found = true;
			}
			if (number_found) {
				token.type = TokenType_Number;
				token.number = number;
			}
		}
	}

	return token;
}

void get_current_position(ParserContext &context, int *row, int *column) {
	char *start = context.source;
	char *stop = context.tokenizer->at;
	i64 diff = stop - start;

	char *at = start;
	*row = 1;
	*column = 1;
	for (unsigned i = 0; i < diff; ++i) {
		if (*at == '\n') {
			(*row)++;
			*column = 0;
		} else {
			(*column)++;
		}
		at++;
	}
}

int get_indentation(Tokenizer *tok) {
	int indentation = 0;

	while (true) {
		if (tok->at[0] == '\t') {
			break;
		} else if (is_whitespace(tok)) {
			tok->at++;
		} else if (is_line_comment(tok)) {
			tok->at += 2;
			consume_line(tok);
		} else if (try_consume_block_comment_opening(tok)) {
			while (!is_eos(tok) && !try_consume_block_comment_closing(tok))
				tok->at++;
		} else {
			break;
		}
	}

	char *at = tok->at;
	while (at[0] == '\t') {
		indentation++;
		at++;
	}

	if (is_eos(at))
		return -1;

	if (is_whitespace(at) || is_line_comment(at) || is_block_comment_opening(at)) {
		tok->at = at;
		return get_indentation(tok);
	}

	return indentation;
}

bool is_next_a_newline(Tokenizer *tok) {
	while (true) {
		if (tok->at[0] == '\n') {
			return true;
		} else if (is_whitespace(tok)) {
			tok->at++;
		} else if (is_line_comment(tok)) {
			tok->at += 2;
			consume_line(tok);
		} else {
			break;
		}
	}
	return (tok->at[0] == '\n' || tok->at[0] == '\0');
}

Token peek_next_token(Tokenizer *tok) {
	char *at = tok->at;
	Token t = next_token(tok);
	tok->at = at;
	return t;
}

Token next_line(Tokenizer *tok) {
	consume_whitespace(tok);

	char *line = tok->at;

	while (1) {
		char c = tok->at[0];
		if (is_eos(c) || is_line_end(c) || is_block_comment_closing(tok)) {
			break;
		}
		tok->at++;
	}

	Token token = make_token(line, (i32)(tok->at - line), TokenType_Line);
	return token;
}
Token peek_next_line(Tokenizer *tok) {
	char *at = tok->at;
	Token t = next_line(tok);
	tok->at = at;
	return t;
}

bool is_there_more_on_current_line(Tokenizer *tok) {
	char *at = tok->at;

	while (true) {
		if (is_line_end(at))
			return false;
		if (!is_whitespace(at))
			return true;

		at++;
	}
}

Token all_until(Tokenizer *tok, char end) {
	consume_whitespace(tok);

	char *line = tok->at;

	while (1) {
		char c = tok->at[0];
		if (is_eos(c)) {
			return make_token(line, 1, '\0');
		} else if (c == end) {
			break;
		}
		tok->at++;
	}

	Token token = make_token(line, (i32)(tok->at - line), TokenType_String);
	return token;
}

#define TOKENIZE(text) (make_token((char *)text, sizeof(text)-1, TokenType_String))

#define ASSERT_TOKEN(tok, token, expected_token) { \
	{ \
		bool equal = is_equal(token, expected_token); \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(equal, "Unexpected token! (expected='%s', found='%s')", expected_token.string.text, token_tostring(token)); \
	} \
}
#define ASSERT_NEXT_TOKEN(tok, expected_token) { \
	{ \
		Token next_token = next_token(&tok); \
		bool equal = is_equal(next_token, expected_token); \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(equal, "Unexpected token! (expected='%s', found='%s')", expected_token.string.text, token_tostring(next_token)); \
	} \
}
#define ASSERT_TOKEN_TYPE(token, expected_type) { \
	{ \
		char a[2]; \
		char b[2]; \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(token.type == expected_type, "Unexpected token! (expected='%s', found token='%s', found type='%s')", type_tostring(expected_type, a), token_tostring(token), type_tostring(token.type, b)) \
	} \
}

#define PARSER_ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		static char buf[1024]; \
		snprintf(buf, ARRAY_COUNT(buf), (format), ##__VA_ARGS__); \
		int row, column; \
		get_current_position(*global_last_parser_context, &row, &column); \
		fprintf(stderr, "%s(%d) : error : %s\n", global_last_parser_context->filepath, row, buf); \
		fflush(stderr); \
		ASSERT(arg, format, ##__VA_ARGS__); \
	} \
}

static char _type_buffer_a[2];
static char _type_buffer_b[2];
__forceinline void assert_next_token_type(Tokenizer *tok, i32 expected_type) {
	Token token = next_token(tok);
	global_last_parser_context->tokenizer = tok;
	PARSER_ASSERT(token.type == expected_type, "Unexpected token! (expected='%s', found token='%s', found type='%s')", type_tostring(expected_type, _type_buffer_a), token_tostring(token), type_tostring(token.type, _type_buffer_b));
}

#define ASSERT_NEW_LINE(tok) { \
	{ \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(is_next_a_newline(&tok), "Expected a newline"); \
	} \
}


#define PARSER_ERROR(arg, format, ...) { \
	if (!(arg)) { \
		static char buf[1024]; \
		snprintf(buf, ARRAY_COUNT(buf), (format), __VA_ARGS__); \
		char filepath_buffer[1024]; \
		GetFullPathName(global_last_parser_context->filepath, 1024, filepath_buffer, 0); \
		int row, column; \
		get_current_position(*global_last_parser_context, &row, &column); \
		fprintf(stderr, "%s(%d) : error : %s\n", filepath_buffer, row, buf); \
		fflush(stderr); \
		global_error = -1; \
	} \
}
