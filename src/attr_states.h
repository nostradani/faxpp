/*
 * Copyright 2007 Doxological Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file needs to have a number of macros defined before it is included

#define NS_CHAR_STATE(name, ch, next_state, fallback_state) \
TokenizerError \
name(TokenizerEnv *env) \
{ \
  read_char(env); \
\
  switch(env->current_char) { \
  case (ch): \
    env->state = (next_state); \
    next_char(env); \
    break; \
  default: \
    env->state = (fallback_state); \
    break; \
  } \
  return NO_ERROR; \
}

NS_CHAR_STATE(PREFIX(ns_name_state1), 'm', PREFIX(ns_name_state2), PREFIX(attr_name_state))
NS_CHAR_STATE(PREFIX(ns_name_state2), 'l', PREFIX(ns_name_state3), PREFIX(attr_name_state))
NS_CHAR_STATE(PREFIX(ns_name_state3), 'n', PREFIX(ns_name_state4), PREFIX(attr_name_state))
NS_CHAR_STATE(PREFIX(ns_name_state4), 's', PREFIX(ns_name_state5), PREFIX(attr_name_state))

#undef NS_CHAR_STATE

TokenizerError
PREFIX(ns_name_state5)(TokenizerEnv *env)
{
  read_char(env);

  switch(env->current_char) {
  WHITESPACE:
    env->state = PREFIX(attr_equals_state);
    token_end_position(env);
    report_token(XMLNS_NAME_TOKEN, env);
    next_char(env);
    break;
  case '=':
    env->state = PREFIX(attr_value_start_state);
    token_end_position(env);
    report_token(XMLNS_NAME_TOKEN, env);
    next_char(env);
    break;
  case ':':
    env->state = PREFIX(attr_name_seen_colon_state);
    token_end_position(env);
    report_token(XMLNS_PREFIX_TOKEN, env);
    next_char(env);
    token_start_position(env);
    break;
  default:
    env->state = PREFIX(attr_name_state);
    next_char(env);
    if((char_flags(env->current_char) & NCNAME_CHAR) == 0)
      return INVALID_CHAR_IN_ATTRIBUTE_NAME;
    break;
  }
  return NO_ERROR;  
}

TokenizerError
PREFIX(attr_name_state)(TokenizerEnv *env)
{
  while(1) {
    END_CHECK;

    READ_CHAR;

    switch(env->current_char) {
    WHITESPACE:
      env->state = PREFIX(attr_equals_state);
      token_end_position(env);
      report_token(ATTRIBUTE_NAME_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    case '=':
      env->state = PREFIX(attr_value_start_state);
      token_end_position(env);
      report_token(ATTRIBUTE_NAME_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    case ':':
      env->state = PREFIX(attr_name_seen_colon_state);
      token_end_position(env);
      report_token(ATTRIBUTE_PREFIX_TOKEN, env);
      next_char(env);
      token_start_position(env);
      return NO_ERROR;
    default:
      DEFAULT_CASE;
      break;
    }

    next_char(env);
    if((char_flags(env->current_char) & NCNAME_CHAR) == 0)
      return INVALID_CHAR_IN_ATTRIBUTE_NAME;
  }

  // Never happens
  return NO_ERROR;  
}

TokenizerError
PREFIX(attr_name_seen_colon_state)(TokenizerEnv *env)
{
  read_char(env);

  switch(env->current_char) {
  LINE_ENDINGS
  default:
    env->state = PREFIX(attr_name_seen_colon_state2);
    next_char(env);
    if((char_flags(env->current_char) & NCNAME_START_CHAR) == 0)
      return INVALID_CHAR_IN_ATTRIBUTE_NAME;
    break;
  }

  return NO_ERROR;  
}

TokenizerError
PREFIX(attr_name_seen_colon_state2)(TokenizerEnv *env)
{
  while(1) {
    END_CHECK;

    READ_CHAR;

    switch(env->current_char) {
    WHITESPACE:
      env->state = PREFIX(attr_equals_state);
      token_end_position(env);
      report_token(ATTRIBUTE_NAME_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    case '=':
      env->state = PREFIX(attr_value_start_state);
      token_end_position(env);
      report_token(ATTRIBUTE_NAME_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    default:
      DEFAULT_CASE;
      break;
    }

    next_char(env);
    if((char_flags(env->current_char) & NCNAME_CHAR) == 0)
      return INVALID_CHAR_IN_ATTRIBUTE_NAME;
  }

  // Never happens
  return NO_ERROR;  
}

TokenizerError
PREFIX(attr_equals_state)(TokenizerEnv *env)
{
  read_char(env);

  switch(env->current_char) {
  WHITESPACE:
    break;
  case '=':
    env->state = PREFIX(attr_value_start_state);
    break;
  default:
    next_char(env);
    return INVALID_CHAR_IN_ATTRIBUTE;
  }
  next_char(env);
  return NO_ERROR;  
}

TokenizerError
PREFIX(attr_value_start_state)(TokenizerEnv *env)
{
  END_CHECK;

  READ_CHAR;

  switch(env->current_char) {
  WHITESPACE:
    next_char(env);
    break;
  case '"':
    env->state = PREFIX(attr_value_quot_state);
    next_char(env);
    token_start_position(env);
    break;
  case '\'':
    env->state = PREFIX(attr_value_apos_state);
    next_char(env);
    token_start_position(env);
    break;
  default:
    DEFAULT_CASE;

    next_char(env);
    return INVALID_CHAR_IN_ATTRIBUTE;
  }
  return NO_ERROR;
}

TokenizerError
PREFIX(attr_value_apos_state)(TokenizerEnv *env)
{
  while(1) {
    if(env->position >= env->buffer_end) {
      if(env->token.value.ptr) {
        token_end_position(env);
        if(env->token.value.len != 0) {
          report_token(ATTRIBUTE_VALUE_TOKEN, env);
          return NO_ERROR;
        }
      }
      token_start_position(env);
      return PREMATURE_END_OF_BUFFER;
    }

    READ_CHAR;

    switch(env->current_char) {
    case '\'':
      env->state = PREFIX(start_element_ws_state);
      token_end_position(env);
      report_token(ATTRIBUTE_VALUE_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    case '&':
      store_state(env);
      env->state = reference_state;
      token_end_position(env);
      report_token(ATTRIBUTE_VALUE_TOKEN, env);
      next_char(env);
      token_start_position(env);
      return NO_ERROR;
    case '<':
      next_char(env);
      return INVALID_CHAR_IN_ATTRIBUTE;
    WHITESPACE:
      env->current_char = ' ';
      break;
    default:
      DEFAULT_CASE;
      break;
    }
    next_char(env);
  }

  // Never happens
  return NO_ERROR;
}

TokenizerError
PREFIX(attr_value_quot_state)(TokenizerEnv *env)
{
  while(1) {
    if(env->position >= env->buffer_end) {
      if(env->token.value.ptr) {
        token_end_position(env);
        if(env->token.value.len != 0) {
          report_token(ATTRIBUTE_VALUE_TOKEN, env);
          return NO_ERROR;
        }
      }
      token_start_position(env);
      return PREMATURE_END_OF_BUFFER;
    }

    READ_CHAR;

    switch(env->current_char) {
    case '"':
      env->state = PREFIX(start_element_ws_state);
      token_end_position(env);
      report_token(ATTRIBUTE_VALUE_TOKEN, env);
      next_char(env);
      return NO_ERROR;
    case '&':
      store_state(env);
      env->state = reference_state;
      token_end_position(env);
      report_token(ATTRIBUTE_VALUE_TOKEN, env);
      next_char(env);
      token_start_position(env);
      return NO_ERROR;
    case '<':
      next_char(env);
      return INVALID_CHAR_IN_ATTRIBUTE;
    WHITESPACE:
      env->current_char = ' ';
      break;
    default:
      DEFAULT_CASE;
      break;
    }
    next_char(env);
  }

  // Never happens
  return NO_ERROR;
}

