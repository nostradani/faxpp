# faxpp

Faxpp is a small, fast and conformant XML pull parser written in C with an API that can return strings in any encoding including UTF-8 and UTF-16. Faxpp is written by John Snelson, and is released under the terms of the Apache Licence v2.
Faxpp provides two different APIs:

- The [FAXPP_Parser](http://faxpp.sourceforge.net/structFAXPP__Parser.html) API would normally suit the needs of most users. It provides an event based pull API for parsing XML documents.
- The [FAXPP_Tokenizer](http://faxpp.sourceforge.net/structFAXPP__Tokenizer.html) API is a low level API giving direct access to the XML tokenizer. Output from this is not compliant with the XML specifications.

## Performance

Faxpp's main performance benefit comes from it's ability to reduce and eliminate string copies - instead, the [FAXPP_Text](http://faxpp.sourceforge.net/structFAXPP__Text.html) structure will point directly to the tokenizer's buffer when possible. This is possible when:
- The output string encoding is the same as the XML document's encoding.
- The event / token does not cross a buffer boundary when streaming input.
- The parser is not set to null terminate it's strings.
- Attribute values do not need to be normalized.

Therefore, to maximize the performance from faxpp the following steps can be taken:
- Choose not to null terminate output strings.
- Choose an output string encoding that is the same as most of the input XML documents that will be parsed.
- Stream XML documents using as large a buffer as possible.
- Turn off attribute value normalization by setting [FAXPP_set_normalize_attrs()](http://faxpp.sourceforge.net/parser_8h.html#97d9fe261f063b0bd5d2b933f8ec9ed8) to 0 (this makes the parser non-conformant).

## Downloads

- Faxpp can be downloaded from Sourceforge.
- Anonymous access to the subversion repository is also available.

## Support

- There is a [user mailing list](http://sourceforge.net/mailarchive/forum.php?forum_name=faxpp-user), [faxpp-user](https://lists.sourceforge.net/lists/listinfo/faxpp-user), which is the appropriate place to direct any questions or problems.
- If you have a bug to report, you may wish to enter it into our [bug database](http://sourceforge.net/tracker/?atid=979420&group_id=201903).
