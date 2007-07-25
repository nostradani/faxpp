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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <faxpp/xml_parser.h>

#define MSECS_IN_SECS 1000000

unsigned long getTime()
{
  struct timeval timev;
  gettimeofday(&timev, 0);

  return (timev.tv_sec * MSECS_IN_SECS) + timev.tv_usec;
}

void
output_text(Text *text, FILE *stream)
{
  char *buffer = (char*)text->ptr;
  char *buffer_end = buffer + text->len;

  while(buffer < buffer_end) {
    putc(*buffer++, stream);
  }
}

#define SHOW_URIS 1

void
output_event(Event *event, FILE *stream)
{
  int i;
  AttrValue *atval;

  switch(event->type) {
  case START_DOCUMENT_EVENT:
    if(event->version.ptr != 0) {
      fprintf(stream, "<?xml version=\"");
      output_text(&event->version, stream);
      if(event->encoding.ptr != 0) {
        fprintf(stream, "\" encoding=\"");
        output_text(&event->encoding, stream);
      }
      if(event->standalone.ptr != 0) {
        fprintf(stream, "\" standalone=\"");
        output_text(&event->standalone, stream);
      }
      fprintf(stream, "\"?>");
    }
    break;
  case END_DOCUMENT_EVENT:
    break;
  case START_ELEMENT_EVENT:
  case SELF_CLOSING_ELEMENT_EVENT:
    fprintf(stream, "<");
#if SHOW_URIS
    if(event->uri.ptr != 0) {
      fprintf(stream, "{");
      output_text(&event->uri, stream);
      fprintf(stream, "}");
    } else
#endif
    if(event->prefix.ptr != 0) {
      output_text(&event->prefix, stream);
      fprintf(stream, ":");
    }
    output_text(&event->name, stream);

    for(i = 0; i < event->attr_count; ++i) {
      fprintf(stream, " ");
#if SHOW_URIS
      if(event->attrs[i].uri.ptr != 0) {
        fprintf(stream, "{");
        output_text(&event->attrs[i].uri, stream);
        fprintf(stream, "}");
      } else
#endif
      if(event->attrs[i].prefix.ptr != 0) {
        output_text(&event->attrs[i].prefix, stream);
        fprintf(stream, ":");
      }
      output_text(&event->attrs[i].name, stream);
      fprintf(stream, "=\"");

      atval = &event->attrs[i].value;
      while(atval) {
        switch(atval->type) {
        case CHARACTERS_EVENT:
          output_text(&atval->value, stream);
          break;
        case ENTITY_REFERENCE_EVENT:
          fprintf(stream, "&");
          output_text(&atval->value, stream);
          fprintf(stream, ";");
          break;
        case DEC_CHAR_REFERENCE_EVENT:
          fprintf(stream, "&#");
          output_text(&atval->value, stream);
          fprintf(stream, ";");
          break;
        case HEX_CHAR_REFERENCE_EVENT:
          fprintf(stream, "&#x");
          output_text(&atval->value, stream);
          fprintf(stream, ";");
          break;
        }
        atval = atval->next;
      }

      fprintf(stream, "\"");
    }

    if(event->type == SELF_CLOSING_ELEMENT_EVENT)
      fprintf(stream, "/>");
    else
      fprintf(stream, ">");
    break;
  case END_ELEMENT_EVENT:
    fprintf(stream, "</");
#if SHOW_URIS
    if(event->uri.ptr != 0) {
      fprintf(stream, "{");
      output_text(&event->uri, stream);
      fprintf(stream, "}");
    } else
#endif
    if(event->prefix.ptr != 0) {
      output_text(&event->prefix, stream);
      fprintf(stream, ":");
    }
    output_text(&event->name, stream);
    fprintf(stream, ">");
    break;
  case CHARACTERS_EVENT:
    output_text(&event->value, stream);
    break;
  case CDATA_EVENT:
    fprintf(stream, "<![CDATA[");
    output_text(&event->value, stream);
    fprintf(stream, "]]>");
    break;
  case IGNORABLE_WHITESPACE_EVENT:
    output_text(&event->value, stream);
    break;
  case COMMENT_EVENT:
    fprintf(stream, "<!--");
    output_text(&event->value, stream);
    fprintf(stream, "-->");
    break;
  case PI_EVENT:
    fprintf(stream, "<?");
    output_text(&event->name, stream);
    if(event->value.ptr != 0) {
      fprintf(stream, " ");
      output_text(&event->value, stream);
    }
    fprintf(stream, "?>");
    break;
  case ENTITY_REFERENCE_EVENT:
    fprintf(stream, "&");
    output_text(&event->name, stream);
    fprintf(stream, ";");
    break;
  case DEC_CHAR_REFERENCE_EVENT:
    fprintf(stream, "&#");
    output_text(&event->name, stream);
    fprintf(stream, ";");
    break;
  case HEX_CHAR_REFERENCE_EVENT:
    fprintf(stream, "&#x");
    output_text(&event->name, stream);
    fprintf(stream, ";");
    break;
  }
}

int
main(int argc, char **argv)
{
  int fd;
  long length;
  void *xml;
  int i;
  unsigned long startTime;

  if(argc < 2) {
    printf("Too few arguments\n");
    exit(-1);
  }

  ParserEnv env;

  TokenizerError err = init_parser(&env, WELL_FORMED_PARSE_MODE, utf8_encode);
  if(err != NO_ERROR) {
    printf("ERROR: %s\n", err_to_string(err));
    exit(1);
  }

  for(i = 1; i < argc; ++i) {

    startTime = getTime();

    fd = open(argv[i], O_RDONLY);
    if(fd == -1) {
      printf("Open failed: %d\n", errno);
      exit(1);
    }

    length = lseek(fd, 0, SEEK_END);

    xml = mmap(0, length, PROT_READ, MAP_SHARED, fd, 0);
    if(xml == MAP_FAILED) {
      printf("Mmap failed: %d\n", errno);
      exit(1);
    }

    close(fd);

    err = init_parse(&env, xml, length);
    if(err != NO_ERROR) {
      printf("ERROR: %s\n", err_to_string(err));
      exit(1);
    }

/*     env.tenv.encode = utf8_encode; */

    err = next_event(&env);
    while(env.event.type != END_DOCUMENT_EVENT) {
      if(err != NO_ERROR) {
        printf("%03d:%03d ERROR: %s\n", env.err_line, env.err_column, err_to_string(err));
        if(err == PREMATURE_END_OF_BUFFER ||
           err == BAD_ENCODING) break;
      }
/*       if(env.event.type != NO_EVENT) */
/*         output_event(&env.event, stdout); */

      err = next_event(&env);
    }
/*     output_event(&env.event, stdout); */

    munmap(xml, length);

    printf("Time taken: %gms\n", ((double)(getTime() - startTime) / MSECS_IN_SECS * 1000));
  }

  free_parser(&env);

  return 0;
}
