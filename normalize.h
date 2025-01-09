// Encode a given character in UTF-8.
static int encode_utf8(char *buf, uint32_t c) {
  if (c <= 0x7F) {
    buf[0] = c;
    return 1;
  }

  if (c <= 0x7FF) {
    buf[0] = 0b11000000 | (c >> 6);
    buf[1] = 0b10000000 | (c & 0b00111111);
    return 2;
  }

  if (c <= 0xFFFF) {
    buf[0] = 0b11100000 | (c >> 12);
    buf[1] = 0b10000000 | ((c >> 6) & 0b00111111);
    buf[2] = 0b10000000 | (c & 0b00111111);
    return 3;
  }

  buf[0] = 0b11110000 | (c >> 18);
  buf[1] = 0b10000000 | ((c >> 12) & 0b00111111);
  buf[2] = 0b10000000 | ((c >> 6) & 0b00111111);
  buf[3] = 0b10000000 | (c & 0b00111111);
  return 4;
}

static bool startswith(const char *p, const char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

static int from_hex(char c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  return c - 'A' + 10;
}

static uint32_t read_universal_char(char *p, int len) {
  uint32_t c = 0;
  for (int i = 0; i < len; i++) {
    if (!isxdigit(p[i]))
      return 0;
    c = (c << 4) | from_hex(p[i]);
  }
  return c;
}

// Normalizes C-source string `p` and returns it.
// `cap` must be the capacity of the `p`-allocation; so at least `strlen(p)+1`.
// It works inline unless the normalized string requires more storage than
// `cap` in which case a new string is allocated.
// The normalizations are:
//  - Remove leading UTF-8 BOM if present
//  - Remove backslashes followed by newline (cleverly re-inserts newlines so
//    that line numbers are not "corrupted")
//  - Converts unicode escapes to UTF-8 chars (e.g. "\u00f8" => "ø")
//  - Inserts newline on last line if missing.
char* normalize_source_string(char* p, size_t cap)
{
  // UTF-8 texts may start with a 3-byte "BOM" marker sequence.
  // If exists, just skip them because they are useless bytes.
  // (It is actually not recommended to add BOM markers to UTF-8
  // texts, but it's not uncommon particularly on Windows.)
  if (!memcmp(p, "\xef\xbb\xbf", 3)) p+=3;

  { // Replace \r or \r\n with \n.
    // NOTE(aks) came from old canonicalize_newline() function
    int i=0, j=0;
    while (p[i]) {
      if (p[i] == '\r' && p[i + 1] == '\n') {
        i += 2;
        p[j++] = '\n';
      } else if (p[i] == '\r') {
        i++;
        p[j++] = '\n';
      } else {
        p[j++] = p[i++];
      }
    }
    p[j] = '\0';
  }

  { // Remove backslashes followed by a newline.
    // We want to keep the number of newline characters so that
    // the logical line number matches the physical one.
    // This counter maintain the number of newlines we have removed.
    // NOTE(aks) came from old remove_backslash_newline() function
    int n = 0;
    int i=0, j=0;
    while (p[i]) {
      if (p[i] == '\\' && p[i + 1] == '\n') {
        i += 2;
        n++;
      } else if (p[i] == '\n') {
        p[j++] = p[i++];
        for (; n > 0; n--)
          p[j++] = '\n';
      } else {
        p[j++] = p[i++];
      }
    }

    for (; n > 0; n--)
      p[j++] = '\n';
    p[j] = '\0';
  }

  char* end = NULL;
  { // Replace \u or \U escape sequences with corresponding UTF-8 bytes.
    // NOTE(aks) this came from old convert_universal_chars() function.
    // XXX(aks) this seemingly also converts escapes /outside/ of strings? :)
    char* q = p;
    char* pp = p;
    while (*pp) {
      if (startswith(pp, "\\u")) {
        uint32_t c = read_universal_char(pp + 2, 4);
        if (c) {
          pp += 6;
          q += encode_utf8(q, c);
        } else {
          *q++ = *pp++;
        }
      } else if (startswith(pp, "\\U")) {
        uint32_t c = read_universal_char(pp + 2, 8);
        if (c) {
          pp += 10;
          q += encode_utf8(q, c);
        } else {
          *q++ = *pp++;
        }
      } else if (pp[0] == '\\') {
        *q++ = *pp++;
        *q++ = *pp++;
      } else {
        *q++ = *pp++;
      }
    }

    *q = '\0';
    end = q;
  }

  assert(end >= p);

  // Make sure that the last line is properly terminated with '\n'.
  if (end > p && end[-1] != '\n') {
    // NOTE(aks) came from old read_file() function
    size_t n = end-p;
    size_t req = n+2;
    if (req > cap) {
      // not enough capacity; allocate new string and copy it
      char* new_p = scratch_calloc(req, 1);
      memcpy(new_p, p, n);
      p = new_p;
    }
    p[n] = '\n';
    p[n+1] = 0;
  }

  return p;
}
