#include "common.h"
#include "file_utils.inl"
#include "memory_arena.cpp"
#include "string_utils.inl"

#define TokenType_Unknown 1
#define TokenType_String 2
#define TokenType_Number 3
#define TokenType_Identifier 4
#define TokenType_Namespace 5
#define TokenType_CommandMarker 6
#define TokenType_Line 7
#define TokenType_Tag 8

namespace parser {
	///////// PARSING /////////
	struct Parameter {
		String scope;
		String type;
		String variable_name;
	};
	struct Function {
		Parameter parameters[32];
		int parameter_count;
		String function_name;
	};

	struct Tokenizer {
		char *at;
		char *debug_start;
		size_t debug_size;
	};

	struct Token {
		char type;
		union {
			struct {
				float number;
			};
			struct {
				u32 length;
				char *text;
			};
			struct {
				String string;
			};
		};
	};

	inline bool is_equal(Token a, Token b) {
		return string_is_equal(a.string, b.string);
	}

	inline Tokenizer make_tokenizer(char *at, size_t debug_size) {
		Tokenizer t = {};
		t.at = at;
		t.debug_start = at;
		t.debug_size = debug_size;
		return t;
	}

	inline Token make_token(char *text, int length, char type) {
		Token t = {};
		t.type = type;
		t.length = length;
		t.text = text;
		return t;
	}

	inline bool is_line_comment(Tokenizer *tok) { return tok->at[0] == '/' && tok->at[1] == '/' && tok->at[2] != '@'; }
	inline bool is_block_comment_opening(Tokenizer *tok) { return tok->at[0] == '/' && tok->at[1] == '*'; }
	inline bool is_block_comment_closing(Tokenizer *tok) { return tok->at[0] == '*' && tok->at[1] == '/'; }
	inline bool is_eos(Tokenizer *tok) { return tok->at[0] == '\0'; }
	inline bool is_line_end(Tokenizer *tok) { return tok->at[0] == '\n' || tok->at[0] == '\r'; }

	inline bool is_eos(char c) { return c == '\0'; }
	inline bool is_line_end(char c) { return c == '\n' || c == '\r'; }
	inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	inline bool is_numeric(char c) { return (c >= '0' && c <= '9'); }
	inline bool is_alpha_numeric(char c) { return is_alpha(c) || is_numeric(c); }
	inline bool is_number(char c) { return is_numeric(c) || c == '.'; } // 0.0 0.0f 0.0L 0x00 0b00
	inline bool is_whitespace(Tokenizer *tok) {
		char c = tok->at[0];
	 	return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || is_line_end(c));
	 }

	 inline void consume_line(Tokenizer *tok) {
	 	while (!is_eos(tok) && !is_line_end(tok))
	 		tok->at++;
	 }

	 inline bool try_consume_block_comment_opening(Tokenizer *tok) {
	 	bool is_opening = is_block_comment_opening(tok);
	 	if (is_opening)
	 		tok->at+=2;
	 	return is_opening;
	}

	inline bool try_consume_block_comment_closing(Tokenizer *tok) {
	 	bool is_closing = is_block_comment_closing(tok);
	 	if (is_closing)
	 		tok->at+=2;
	 	return is_closing;
	}

	void consume_whitespace(Tokenizer *tok) {
		while (true) {
			if (is_whitespace(tok)) {
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
	}

	float parse_number(Tokenizer *tok, int sign, int integer, bool do_fraction) {
		if (do_fraction) {
			int divisor = 1;
			int fraction = 0;
			while (is_numeric(tok->at[0])) {
				char c = tok->at[0];
				fraction = fraction*10 + (c - '0');
				divisor *= 10;
				tok->at++;
			}
			return sign * (integer + (fraction/(float)divisor));
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

	Token next_token(Tokenizer *tok, bool verbatim = true) {
		if (tok->debug_start != '\0') {
			int debug_current_size = tok->at - tok->debug_start;
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
			token.length = tok->at - token.text;
			if (tok->at[0] == '"')
				tok->at++;
		} else if (c == '<') {
			token.type = TokenType_Tag;
			token.text = tok->at;
			while (!is_eos(tok) && tok->at[0] != '>') {
				if (tok->at[0] == '\\' && tok->at[1])
					tok->at++;
				tok->at++;
			}
			token.length = tok->at - token.text;
			if (tok->at[0] == '>')
				tok->at++;
		} else if (c == '/' && tok->at[0] == '/' && tok->at[1] == '@') {
			token.type = TokenType_CommandMarker;
			token.length = 3;
			tok->at += 2;
		} else if (c == ':' && tok->at[0] == ':') {
			token.type = TokenType_Namespace;
			token.length = 2;
			tok->at++;
		} else if (is_alpha(c) || c == '_') {
			while (is_alpha_numeric(tok->at[0]) || tok->at[0] == '_')
				tok->at++;
			token.type = TokenType_Identifier;
			token.length = tok->at - token.text;
		} else {
			if (verbatim) { // If verbatim, then we wont try to interpret any numbers, but just parse them as they read.
				if (is_numeric(c)) {
					while (is_numeric(tok->at[0]))
						tok->at++;
					token.type = TokenType_Number;
					token.length = tok->at - token.text;
				}
			} else {
				float number;
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

	Token next_line(Tokenizer *tok) {
		consume_whitespace(tok);

		char *line = tok->at;

		while (true) {
			char c = tok->at[0];
			if (is_eos(c) || is_line_end(c)) {
				break;
			}
			tok->at++;
		}

		Token token = make_token(line, tok->at - line, TokenType_Line);
		return token;
	}

	inline void fast_forward_until(Tokenizer *tok, char character) {
		while (true) {
			if (tok->at[0] == character)
				break;
			tok->at++;
		}
	}
	inline bool fast_forward_until(char **at, String string) {
		int match_index = 0;
		while (true) {
			if (*at[0] == '\0')
				return false;

			if (*at[0] == string[match_index]) {
				match_index++; (*at)++;
				if (match_index == string.length)
					return true;
			} else if (match_index > 0) {
				match_index = 0;
			} else {
				(*at)++;
			}
		}
		return true;
	}
	inline bool fast_forward_until(Tokenizer *tok, String string) {
		return fast_forward_until(&tok->at, string);
	}

	#define PRINT_TOKEN(token) printf("%.*s\n", token.length, *token.string);
	#define PRINT_TOKEN_NUMBER(token) printf("%f\n", token.number);

	#define ASSERT_NEXT_TOKEN(tokenizer, string) ASSERT(parser::is_equal(parser::next_token(&tokenizer), TOKENIZE(string)), "Unexpected token "#string)
	#define ASSERT_NEXT_TOKEN_TYPE(tokenizer, t) ASSERT(parser::next_token(&tokenizer).type == t, "Unexpected token")

#if 0
#define DEFAULT_INVALID_CHARACTERS " \n(),;"
static size_t default_nr_invalid = sizeof(DEFAULT_INVALID_CHARACTERS);

static bool is_valid(char c, const char *invalid_characters, size_t nr_invalid_characters) {
		for (size_t i = 0; i < nr_invalid_characters; ++i) {
			if (c == invalid_characters[i])
				return false;
		}
		return true;
	}

	char *next(char *buffer, int *index, MemoryArena *arena, const char *invalid_characters, size_t nr_invalid_characters, bool skip_ahead = true, int *parsed_nr_chars = 0) {
		char c;
		int start_index = *index;
		int end_index = *index;
		while (is_valid(buffer[end_index], invalid_characters, nr_invalid_characters)) {
			end_index++;
		}

		int size = (end_index - start_index);
		char *token = allocate_memory(*arena, size + 1);
		for (int i = 0; i < size; i++)
			token[i] = buffer[i + start_index];

		token[size] = '\0';

		if (skip_ahead) {
			while (!is_valid(buffer[end_index], invalid_characters, nr_invalid_characters)) {
				end_index++;
			}
		}

		*index = end_index;
		if (parsed_nr_chars != 0)
			*parsed_nr_chars = (end_index - start_index);

		return token;
	}

	void skip_ahead(char *buffer, int *index, MemoryArena *arena, const char *invalid_characters = DEFAULT_INVALID_CHARACTERS, size_t nr_invalid_characters = default_nr_invalid) {
		while (!is_valid(buffer[*index], invalid_characters, nr_invalid_characters)) {
			(*index)++;
		}
	}

	char *next_line(char *buffer, int *index, MemoryArena *arena, bool skip_ahead = true) {
		return next(buffer, index, arena, "\n", 1, skip_ahead);
	}

	char *next_token(char *buffer, int *index, MemoryArena *arena, bool skip_ahead = true) {
		return next(buffer, index, arena, DEFAULT_INVALID_CHARACTERS, default_nr_invalid, skip_ahead);
	}
#endif
}

#define TOKENIZE(text) (parser::make_token((char *)text, sizeof(text)-1, TokenType_String))

