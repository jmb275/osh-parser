/*
 * Copyright (c) 2022, Justin Bradley
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Updates:
 * 2022-01-23 Fixes to handle no spaces after semicolons (courtesy of Jaden
 * Goter)
 * 2022-01-27 Improved operator handling. Now no weird whitespace issues
 *     Justin: the original parser didn't properly tokenize when there were no
 *     spaces around the operators. This was fine in the tests because we never
 *     tested for that case, but it was not robust for student testing. To fix
 *     this, we now just put spaces around all operators before the tokenizer.
 * 
 */

#include <sstream>
#include <regex>

#include "parser.hpp"

namespace {
    enum class shell_token_type {
        text,
        redirect_cin,
        redirect_cout,
        append_cout,
        pipe,
        logical_and,
        logical_or,
        semicolon
    };

    shell_token_type get_shell_token_type(const std::string& token)
    {
        switch (token.size()) {
        case 1:
            switch (token[0]) {
            case '<':
                return shell_token_type::redirect_cin;
            case '>':
                return shell_token_type::redirect_cout;
            case '|':
                return shell_token_type::pipe;
            case ';':
                return shell_token_type::semicolon;
            }
        case 2:
            if (token == ">>") {
                return shell_token_type::append_cout;
            }
            else if (token == "&&") {
                return shell_token_type::logical_and;
            }
            else if (token == "||") {
                return shell_token_type::logical_or;
            }
        }

        return shell_token_type::text;
    }
}

std::vector<shell_command> parse_command_string(const std::string& str)
{
    std::vector<shell_command> commands(1);

    enum class parser_state {
        need_any_token,
        need_new_command,
        need_in_path,
        need_out_path
    } state = parser_state::need_new_command;

	// put spaces around operators > < >> && || ; |
    std::string new_str;
    for (int i = 0; i < (int)str.length(); i++) {
		// first look for && || and >>
        if ((str[i] == '>' && str[i+1] == '>') ||
			(str[i] == '&' && str[i+1] == '&') ||
			(str[i] == '|' && str[i+1] == '|')){
				new_str += std::string(" ")+str[i]+str[i+1]+" ";
				i++;
				// std::cout << "Found >> && or ||" << std::endl;
		}
		// next look for > < ; |
		else if(str[i] == ';' ||
				str[i] == '>' ||
				str[i] == '<' ||
				str[i] == '|') {
				new_str += std::string(" ")+str[i]+" ";
				// std::cout << "Found ; > < |" << std::endl;
		}
		// no operator, so just add the character
		else {
				new_str += str[i];
		}
    }
	// std::cout << "str: " << str << std::endl;
	// std::cout << "new_str: " << new_str << std::endl;		

	// Now parse and iterate over the tokens
	std::istringstream iss(new_str);
    std::string token;
    while (iss >> token) {
        auto token_type = get_shell_token_type(token);
		// std::cout << "Beginning of while state is: " << (int) state << std::endl;

        switch (state) {				
        case parser_state::need_any_token:
            switch (token_type) {
            case shell_token_type::text:
                commands.back().args.push_back(token);
                break;

            case shell_token_type::redirect_cin:
                if (commands.back().cin_mode == istream_mode::pipe) {
                    throw parsing_error("Ambiguous input redirect.");
                }
                commands.back().cin_mode = istream_mode::file;
                state = parser_state::need_in_path;
                break;

            case shell_token_type::redirect_cout:
                commands.back().cout_mode = ostream_mode::file;
                state = parser_state::need_out_path;
                break;

            case shell_token_type::append_cout:
                commands.back().cout_mode = ostream_mode::append;
                state = parser_state::need_out_path;
                break;

            case shell_token_type::pipe:
                if (commands.back().cout_mode != ostream_mode::term) {
                    throw parsing_error("Ambiguous output redirect.");
                }
                commands.back().cout_mode = ostream_mode::pipe;
                commands.emplace_back();
                commands.back().cin_mode = istream_mode::pipe;
                state = parser_state::need_new_command;
                break;

            case shell_token_type::logical_and:
                commands.back().next_mode = next_command_mode::on_success;
                commands.emplace_back();
                state = parser_state::need_new_command;
                break;

            case shell_token_type::logical_or:
                commands.back().next_mode = next_command_mode::on_fail;
                commands.emplace_back();
                state = parser_state::need_new_command;
                break;

            case shell_token_type::semicolon:
                commands.emplace_back();
                state = parser_state::need_new_command;
                break;
            }
            break;

        case parser_state::need_new_command:
            if (token_type != shell_token_type::text) {
                throw parsing_error("Invalid NULL command");
            }
            commands.back().cmd = token;
            state = parser_state::need_any_token;
            break;

        case parser_state::need_in_path:
            if (token_type != shell_token_type::text) {
                throw parsing_error("Expecting an input path");
            }
            commands.back().cin_file = token;
            state = parser_state::need_any_token;
            break;

        case parser_state::need_out_path:
				// std::cout << "in need_out_path" << std::endl;
				
            if (token_type != shell_token_type::text) {
                throw parsing_error("Expecting an output path");
            }
            commands.back().cout_file = token;
            state = parser_state::need_any_token;
            break;
        }
		// int s = (int) state;
		// std::cout << "End of while state is: " << s << std::endl;
    }

    if (commands.back().cmd == "") {
        commands.pop_back();
    }

    return commands;
}
