/*
  4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
n*/

// TOP
#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#define EXTRA_KEYWORDS "custom_keywords.h"

#include "4coder_default_include.cpp"

// NOTE(allen|a4.0.22): This no longer serves as very good example code.
// Good example code will be coming soon, but in the mean time you can go
// to 4coder_remapping_commands.cpp for examples of what binding code looks like.

#if !defined(NO_BINDING)




void paste_clipboard_index(Application_Links* app, int clip_index)
{
    View_Summary view = get_active_view(app, AccessOpen);
    
    int32_t len = clipboard_index(app, 0, clip_index, 0, 0);
    char *str = 0;
    
    if (len <= app->memory_size)
        str = (char*)app->memory;
    
    if (str != 0)
    {
        clipboard_index(app, 0, clip_index, str, len);
        
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
		int32_t pos = view.cursor.pos;
        
        buffer_replace_range(app, &buffer, pos, pos, str, len);
        view_set_cursor(app, &view, seek_pos(pos + len), true);
        
        Theme_Color paste = {};
        paste.tag = Stag_Paste;
        get_theme_colors(app, &paste, 1);
        view_post_fade(app, &view, 0.667f, pos, pos + len, paste.color);
        
        auto_tab_range(app);
    }
}

static void
activate_clipboard_lister(Application_Links *app, Partition *scratch, Heap *heap,
                          View_Summary *view, Lister_State *state,
                          String text_field, void *user_data, bool32 activated_by_mouse)
{
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    int clip_index = (int) PtrAsInt(user_data);
    paste_clipboard_index(app, clip_index);
}

CUSTOM_COMMAND_SIG(clipmate_lister)
{
    Partition* arena = &global_part;
    Temp_Memory temp = begin_temp_memory(arena);
    
    View_Summary view = get_active_view(app, AccessOpen);
    view_end_ui_mode(app, &view);
    
    int32_t option_count = clipboard_count(app, 0);
    if (option_count > 10)
        option_count = 10;
    
    Lister_Option* options = push_array(arena, Lister_Option, option_count);
    for (int32_t i = 0; i < option_count; i++)
    {
        int32_t contents_length = clipboard_index(app, 0, i, 0, 0);
        
		char* str_index = push_array(arena, char, 5);
		itoa(i, str_index, 10);
        
        char* clipboard_contents = push_array(arena, char, contents_length);
        clipboard_index(app, 0, i, clipboard_contents, contents_length);
        
        options[i].string = make_string(str_index, (int) strlen(str_index));
        options[i].status = make_string(clipboard_contents, contents_length);
        options[i].user_data = IntAsPtr(i);
    }
    
    begin_integrated_lister__basic_list(app, "Clipboard:", activate_clipboard_lister, 0, 0, options, option_count, 0, &view);
    
    end_temp_memory(temp);
}


CUSTOM_COMMAND_SIG(f2_cut) {
    exec_command(app, seek_beginning_of_line);
    exec_command(app, set_mark);
    exec_command(app, seek_end_of_line);
    exec_command(app, cut);
    exec_command(app, delete_char);
}

CUSTOM_COMMAND_SIG(f3_copy) {
    exec_command(app, seek_beginning_of_line);
    exec_command(app, set_mark);
    exec_command(app, seek_end_of_line);
    exec_command(app, copy);
    exec_command(app, seek_beginning_of_line);
    exec_command(app, move_down);
}

CUSTOM_COMMAND_SIG(f4_paste) {
    exec_command(app, paste);
    write_string(app, make_lit_string( "\n" ));
}

CUSTOM_COMMAND_SIG(match_brace) {
    // It does not work as I expected...
    exec_command(app, select_surrounding_scope);
    exec_command(app, cursor_mark_swap);
}

CUSTOM_COMMAND_SIG(insert_newline_above)
CUSTOM_DOC( "Insert a new line above the current line." )
{
    exec_command(app, move_up);
    exec_command(app, seek_end_of_line);
    write_string(app, make_lit_string( "\n" ));
}

CUSTOM_COMMAND_SIG(insert_newline_below)
CUSTOM_DOC( "Insert a new line below the current line." )
{
    exec_command(app, seek_end_of_line);
    write_string(app, make_lit_string( "\n" ));
}

void custom_keys(Bind_Helper *context) {
    
    begin_map(context, mapid_global);

    // Original Emacs-like movement
    bind(context, 'p', MDFR_CTRL, move_up);
    bind(context, 'n', MDFR_CTRL, move_down);
    bind(context, 'f', MDFR_CTRL, move_right);
    bind(context, 'b', MDFR_CTRL, move_left);

    // Tom Emacs movement
    bind(context, 'p', MDFR_ALT, seek_whitespace_up_end_line);
    bind(context, 'n', MDFR_ALT, seek_whitespace_down_end_line);
    bind(context, 'f', MDFR_ALT, seek_alphanumeric_or_camel_right);
    bind(context, 'b', MDFR_ALT, seek_alphanumeric_or_camel_left);

    // Tom Emacs favorite
    bind(context, 'w', MDFR_CTRL, kill_buffer); // Original Emacs: C-x k
    bind(context, '\t', MDFR_CTRL, change_active_panel); // Original Emacs: C-x o 
    bind(context, '2', MDFR_ALT, open_panel_vsplit); // Original Emacs: C-x 2
    bind(context, '3', MDFR_ALT, open_panel_hsplit); // Original Emacs: C-x 3
    bind(context, '0', MDFR_ALT, close_panel); // Original Emacs: C-x 0

    // 4coder defaults (some commented by me)
    bind(context, ',', MDFR_CTRL, change_active_panel);
    bind(context, '<', MDFR_CTRL, change_active_panel_backwards);
    bind(context, 'N', MDFR_CTRL, interactive_new);
    bind(context, 'o', MDFR_CTRL, interactive_open_or_new);
    bind(context, 'o', MDFR_ALT, open_in_other);
    // bind(context, 'k', MDFR_CTRL, interactive_kill_buffer);
    bind(context, 'i', MDFR_CTRL, interactive_switch_buffer);
    bind(context, 'h', MDFR_CTRL, project_go_to_root_directory);
    bind(context, 'S', MDFR_CTRL, save);
    bind(context, 's', MDFR_CTRL | MDFR_ALT, save_all_dirty_buffers);
    bind(context, '.', MDFR_ALT, change_to_build_panel);
    bind(context, ',', MDFR_ALT, close_build_panel);
    bind(context, 's', MDFR_ALT, goto_next_jump_sticky);
    bind(context, 'S', MDFR_ALT, goto_prev_jump_sticky);
    // bind(context, 'n', MDFR_ALT, goto_next_jump_sticky);
    // bind(context, 'N', MDFR_ALT, goto_prev_jump_sticky);
    bind(context, 'M', MDFR_ALT, goto_first_jump_sticky);
    bind(context, 'm', MDFR_ALT, build_in_build_panel);
    //bind(context, 'b', MDFR_ALT, toggle_filebar);
    bind(context, 'z', MDFR_ALT, execute_any_cli);
    bind(context, 'Z', MDFR_ALT, execute_previous_cli);
    bind(context, 'x', MDFR_ALT, command_lister);
    bind(context, 'X', MDFR_ALT, project_command_lister);
    bind(context, 'I', MDFR_CTRL, list_all_functions_all_buffers_lister);
    //bind(context, 'E', MDFR_ALT, exit_4coder);
    bind(context, key_f1, MDFR_NONE, project_fkey_command);
    bind(context, key_f2, MDFR_NONE, cut);
    bind(context, key_f3, MDFR_NONE, copy);
    bind(context, key_f4, MDFR_NONE, paste);
    bind(context, key_f4, MDFR_ALT, exit_4coder);
    bind(context, key_f5, MDFR_NONE, project_fkey_command);
    bind(context, key_f6, MDFR_NONE, project_fkey_command);
    bind(context, key_f7, MDFR_NONE, project_fkey_command);
    bind(context, key_f8, MDFR_NONE, project_fkey_command);
    bind(context, key_f9, MDFR_NONE, toggle_filebar);
    bind(context, key_f10, MDFR_NONE, project_fkey_command);
    bind(context, key_f11, MDFR_NONE, project_fkey_command);
    bind(context, key_f12, MDFR_NONE, project_fkey_command);
    bind(context, key_f13, MDFR_NONE, project_fkey_command);
    bind(context, key_f14, MDFR_NONE, project_fkey_command);
    bind(context, key_f15, MDFR_NONE, move_up);
    bind(context, key_f16, MDFR_NONE, move_down);
    bind(context, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
    bind(context, key_mouse_wheel, MDFR_CTRL, mouse_wheel_change_face_size);
    end_map(context);
    
    
    begin_map(context, mapid_file);
    bind(context, 'p', MDFR_ALT, seek_whitespace_up_end_line);
    bind(context, 'n', MDFR_ALT, seek_whitespace_down_end_line);
    bind(context, '\t', MDFR_CTRL, change_active_panel);
    
    bind_vanilla_keys(context, write_character);
    bind(context, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
    bind(context, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
    bind(context, key_mouse_left_release, MDFR_NONE, click_set_cursor);
    bind(context, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
    
    bind(context, key_del, MDFR_NONE, delete_char);
    bind(context, key_del, MDFR_SHIFT, delete_char);
    bind(context, key_back, MDFR_NONE, backspace_char);
    bind(context, key_back, MDFR_SHIFT, backspace_char);
    
    bind(context, key_up, MDFR_NONE, move_up);
    bind(context, key_up, MDFR_SHIFT, move_up);
    bind(context, key_down, MDFR_NONE, move_down);
    bind(context, key_down, MDFR_SHIFT, move_down);
    bind(context, key_left, MDFR_NONE, move_left);
    bind(context, key_left, MDFR_SHIFT, move_left);
    bind(context, key_right, MDFR_NONE, move_right);
    bind(context, key_right, MDFR_SHIFT, move_right);
    
    // Emacs movement style
    bind(context, 'a', MDFR_CTRL, seek_beginning_of_line);
    bind(context, 'e', MDFR_CTRL, seek_end_of_line);
    
    bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
    bind(context, key_home, MDFR_SHIFT, seek_beginning_of_line);
    bind(context, key_home, MDFR_CTRL, goto_beginning_of_file);
    bind(context, key_end, MDFR_NONE, seek_end_of_line);
    bind(context, key_end, MDFR_SHIFT, seek_end_of_line);
    bind(context, key_end, MDFR_CTRL, goto_end_of_file);
    
    bind(context, key_page_up, MDFR_NONE, page_up);
    bind(context, key_page_up, MDFR_SHIFT, page_up);
    bind(context, key_page_up, MDFR_CTRL, goto_beginning_of_file);
    bind(context, key_page_up, MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);
    bind(context, key_page_down, MDFR_NONE, page_down);
    bind(context, key_page_down, MDFR_SHIFT, page_down);
    bind(context, key_page_down, MDFR_CTRL, goto_end_of_file);
    bind(context, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
    
    
    bind(context, key_up, MDFR_CTRL, seek_whitespace_up_end_line);
    bind(context, key_up, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_up_end_line);
    bind(context, key_down, MDFR_CTRL, seek_whitespace_down_end_line);
    bind(context, key_down, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_down_end_line);
    bind(context, key_left, MDFR_CTRL, seek_whitespace_left);
    bind(context, key_left, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_left);
    bind(context, key_right, MDFR_CTRL, seek_whitespace_right);
    bind(context, key_right, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_right);
    
    bind(context, key_up, MDFR_ALT, move_line_up);
    bind(context, key_down, MDFR_ALT, move_line_down);
    
    bind(context, key_back, MDFR_CTRL, backspace_word);
    bind(context, key_del, MDFR_CTRL, delete_word);
    bind(context, 'd', MDFR_ALT, delete_word);
    bind(context, key_back, MDFR_ALT, snipe_token_or_word);
    bind(context, key_del, MDFR_ALT, snipe_token_or_word_right);
    
    bind(context, ' ', MDFR_CTRL, set_mark);
    bind(context, key_f5, MDFR_NONE, replace_in_range);
    bind(context, 'c', MDFR_CTRL, copy);
    bind(context, 'd', MDFR_CTRL, delete_range);
    bind(context, 'D', MDFR_CTRL, delete_line);
    bind(context, 'k', MDFR_CTRL, delete_line);
    bind(context, key_f8, MDFR_CTRL, center_view);
    bind(context, 'E', MDFR_CTRL, left_adjust_view);
    
    bind(context, 'f', MDFR_CTRL, search);
    bind(context, 'F', MDFR_CTRL, list_all_locations);
    bind(context, 'F', MDFR_ALT, list_all_substring_locations_case_insensitive);
    bind(context, 'g', MDFR_ALT, goto_line);
    bind(context, 'G', MDFR_CTRL, list_all_locations_of_selection);
    bind(context, 'j', MDFR_CTRL, snippet_lister);
    //bind(context, 'K', MDFR_CTRL, kill_buffer);
    bind(context, 'l', MDFR_CTRL, duplicate_line);
    bind(context, 'm', MDFR_CTRL, cursor_mark_swap);
    bind(context, 'O', MDFR_CTRL, reopen);
    bind(context, 'q', MDFR_CTRL, query_replace);
    bind(context, 'Q', MDFR_CTRL, query_replace_identifier);
    bind(context, 'q', MDFR_ALT, query_replace_selection);
    bind(context, 'r', MDFR_CTRL, reverse_search);
    bind(context, 's', MDFR_CTRL, save);
    bind(context, 't', MDFR_CTRL, search_identifier);
    bind(context, 'T', MDFR_CTRL, list_all_locations_of_identifier);
    bind(context, 'v', MDFR_CTRL, paste_and_indent);
    bind(context, 'V', MDFR_CTRL, paste_next_and_indent);
    bind(context, 'x', MDFR_CTRL, cut);
    bind(context, 'y', MDFR_CTRL, redo);
    bind(context, 'z', MDFR_CTRL, undo);
    bind(context, '1', MDFR_CTRL, view_buffer_other_panel);
    bind(context, '2', MDFR_CTRL, swap_buffers_between_panels);
    bind(context, '\n', MDFR_NONE, newline_or_goto_position_sticky);
    bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
    bind(context, '>', MDFR_CTRL, view_jump_list_with_lister);
    bind(context, ' ', MDFR_SHIFT, write_character);
    end_map(context);
    
    begin_map(context, default_code_map);
    inherit_map(context, mapid_file);
    bind(context, key_left, MDFR_CTRL, seek_alphanumeric_or_camel_left);
    bind(context, key_right, MDFR_CTRL, seek_alphanumeric_or_camel_right);
    
    // Emacs movement style
    bind(context, 'b', MDFR_ALT, seek_alphanumeric_or_camel_left);
    bind(context, 'f', MDFR_ALT, seek_alphanumeric_or_camel_right);
    
    bind(context, '\n', MDFR_NONE, write_and_auto_tab);
    bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, ')', MDFR_NONE, write_and_auto_tab);
    bind(context, ']', MDFR_NONE, write_and_auto_tab);
    bind(context, ';', MDFR_NONE, write_and_auto_tab);
    bind(context, '#', MDFR_NONE, write_and_auto_tab);
    bind(context, '/', MDFR_CTRL, comment_line_toggle);
    bind(context, '\t', MDFR_NONE, word_complete);
    
    // Disabled to use Ctrl+TAB to change active panel
    //bind(context, '\t', MDFR_CTRL, auto_tab_range);
    
    bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
    bind(context, 'r', MDFR_ALT, write_block);
    bind(context, 't', MDFR_ALT, write_todo);
    bind(context, 'y', MDFR_ALT, write_note);
    
    // Disabled... not sure why...
    //bind(context, 'D', MDFR_ALT, list_all_locations_of_type_definition);
    bind(context, 'T', MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
    bind(context, '[', MDFR_CTRL, open_long_braces);
    bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
    bind(context, '}', MDFR_CTRL, open_long_braces_break);
    bind(context, '[', MDFR_ALT, select_surrounding_scope);
    bind(context, ']', MDFR_ALT, select_prev_scope_absolute);
    bind(context, '\'', MDFR_ALT, select_next_scope_absolute);
    bind(context, '/', MDFR_ALT, place_in_scope);
    bind(context, '-', MDFR_ALT, delete_current_scope);
    bind(context, 'j', MDFR_ALT, scope_absorb_down);
    bind(context, 'i', MDFR_ALT, if0_off);
    bind(context, '1', MDFR_ALT, open_file_in_quotes);
    
    // <alt 2> If the current file is a *.cpp or *.h, 
    // attempts to open the corresponding *.h or *.cpp file in the other view.
    //bind(context, '2', MDFR_ALT, open_matching_file_cpp);
    
    bind(context, '0', MDFR_CTRL, write_zero_struct);
    end_map(context);
    
    begin_map(context, default_lister_ui_map);
    
    bind_vanilla_keys(context, lister__write_character);
    bind(context, key_esc, MDFR_NONE, lister__quit);
    bind(context, 'g', MDFR_CTRL, lister__quit);
    bind(context, '\n', MDFR_NONE, lister__activate);
    bind(context, '\t', MDFR_NONE, lister__activate);
    bind(context, key_back, MDFR_NONE, lister__backspace_text_field);
    //bind(context, '.', MDFR_NONE, lister__backspace_text_field);
    
    bind(context, key_up, MDFR_NONE, lister__move_up);
    bind(context, 'p', MDFR_CTRL, lister__move_up);
    //bind(context, 'k', MDFR_ALT, lister__move_up);
    bind(context, key_page_up, MDFR_NONE, lister__move_up);
    
    bind(context, key_down, MDFR_NONE, lister__move_down);
    bind(context, 'n', MDFR_CTRL, lister__move_down);
    //bind(context, 'j', MDFR_ALT, lister__move_down);
    bind(context, key_page_down, MDFR_NONE, lister__move_down);
    
    bind(context, key_mouse_wheel, MDFR_NONE, lister__wheel_scroll);
    bind(context, key_mouse_left, MDFR_NONE, lister__mouse_press);
    bind(context, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
    bind(context, key_mouse_move, MDFR_NONE, lister__repaint);
    bind(context, key_animate, MDFR_NONE, lister__repaint);
    end_map(context);
}


extern "C" int32_t
get_bindings(void *data, int32_t size) {
    Bind_Helper context_ = begin_bind_helper(data, size);
    Bind_Helper *context = &context_;
    
    set_all_default_hooks(context);
#if defined(__APPLE__) && defined(__MACH__)
    mac_default_keys(context);
#else
    custom_keys(context);
#endif
    
    int32_t result = end_bind_helper(context);
    return(result);
}


#endif //NO_BINDING

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

