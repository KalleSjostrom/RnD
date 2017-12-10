#include "string_utils.inl"

#define TokenType_Unknown 1
#define TokenType_String 2
#define TokenType_Number 3
#define TokenType_NumberLiteral 4
#define TokenType_Identifier 5
#define TokenType_Namespace 6
#define TokenType_CommandMarker 7
#define TokenType_CommandMarkerBlock 8
#define TokenType_Line 9
#define TokenType_IDStringMarker 10
#define TokenType_ID32StringMarker 11
#define TokenType_ID64StringMarker 12

namespace parser {

	inline char *type_tostring(int token_type, char *type_buffer) {
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
		bool verbatim;
		char *at;
		char *debug_start;
		size_t debug_size;
	};

	// Only for debugging purposes
	struct ParserContext {
		char *filepath;
		char *source;
		Tokenizer *tokenizer;
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
		return are_strings_equal(a.string, b.string);
	}

	inline Tokenizer make_tokenizer(bool verbatim, char *at, size_t debug_size) {
		Tokenizer t = {};
		t.verbatim = verbatim; // If true, will not do any actual conversions between strings and numbers.
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

	inline bool is_eos(char c) { return c == '\0'; }
	inline bool is_line_end(char c) { return c == '\n' || c == '\r'; }
	inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	inline bool is_numeric(char c) { return (c >= '0' && c <= '9'); }
	inline bool is_alpha_numeric(char c) { return is_alpha(c) || is_numeric(c); }
	inline bool is_number(char c) { return is_numeric(c) || c == 'x' || c == 'b' || c == '.'; } // 0.0 0.0f 0.0L 0x00 0b00
	inline bool is_whitespace(char c) { return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || is_line_end(c)); }

	inline bool is_line_comment(Tokenizer *tok) { return (tok->at[0] == '/' && tok->at[1] == '/' && tok->at[2] != '!') || (tok->at[0] == '-' && tok->at[1] == '-'); }
	inline bool is_block_comment_opening(Tokenizer *tok) { return tok->at[0] == '/' && tok->at[1] == '*' && tok->at[2] != '!'; }
	inline bool is_block_comment_closing(Tokenizer *tok) { return tok->at[0] == '*' && tok->at[1] == '/'; }
	inline bool is_eos(Tokenizer *tok) { return tok->at[0] == '\0'; }
	inline bool is_line_end(Tokenizer *tok) { return tok->at[0] == '\n' || tok->at[0] == '\r'; }
	inline bool is_whitespace(Tokenizer *tok) { return is_whitespace(tok->at[0]); }

	inline bool try_get_id_marker(Tokenizer *tok, Token &token) {
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

	Token next_token(Tokenizer *tok) {
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
			token.length = tok->at - token.text;
		} else {
			if (tok->verbatim) {
				if (is_numeric(c)) {
					while (is_numeric(tok->at[0]))
						tok->at++;
					token.type = TokenType_NumberLiteral;
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

	void get_current_position(ParserContext &context, int *row, int *column) {
		char *start = context.source;
		char *stop = context.tokenizer->at;
		unsigned diff = stop - start;

		char *at = start;
		*row = 1;
		*column = 1;
		for (int i = 0; i < diff; ++i) {
			if (*at == '\n') {
				(*row)++;
				*column = 0;
			} else {
				(*column)++;
			}
			at++;
		}
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

		Token token = make_token(line, tok->at - line, TokenType_Line);
		return token;
	}
	Token peek_next_line(Tokenizer *tok) {
		char *at = tok->at;
		Token t = next_line(tok);
		tok->at = at;
		return t;
	}

	bool ends_in(Token &token, char end) {
		for (unsigned i = token.length-1; i != (unsigned)-1; i--) {
			if(token.string[i] != end && !is_whitespace(token.string[i]))
				return false;
		}
		return false;
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

		Token token = make_token(line, tok->at - line, TokenType_String);
		return token;
	}
}

#define TOKENIZE(text) (parser::make_token((char *)text, sizeof(text)-1, TokenType_String))
