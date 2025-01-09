#include "chibicc.h"

// Read a UTF-8-encoded Unicode code point from a source file.
// We assume that source files are always in UTF-8.
//
// UTF-8 is a variable-width encoding in which one code point is
// encoded in one to four bytes. One byte UTF-8 code points are
// identical to ASCII. Non-ASCII characters are encoded using more
// than one byte.
static uint32_t decode_utf8(const char **new_pos, const char *p) {
  if ((unsigned char)*p < 128) {
    *new_pos = p + 1;
    return *p;
  }

  const char *start = p;
  int len;
  uint32_t c;

  if ((unsigned char)*p >= 0b11110000) {
    len = 4;
    c = *p & 0b111;
  } else if ((unsigned char)*p >= 0b11100000) {
    len = 3;
    c = *p & 0b1111;
  } else if ((unsigned char)*p >= 0b11000000) {
    len = 2;
    c = *p & 0b11111;
  } else {
    error_at(start, "invalid UTF-8 sequence");
  }

  for (int i = 1; i < len; i++) {
    if ((unsigned char)p[i] >> 6 != 0b10)
      error_at(start, "invalid UTF-8 sequence");
    c = (c << 6) | (p[i] & 0b111111);
  }

  *new_pos = p + len;
  return c;
}

static bool in_range(uint32_t *range, uint32_t c) {
  for (int i = 0; range[i] != -1; i += 2)
    if (range[i] <= c && c <= range[i + 1])
      return true;
  return false;
}

// [https://www.sigbus.info/n1570#D] C11 allows not only ASCII but
// some multibyte characters in certan Unicode ranges to be used in an
// identifier.
//
// This function returns true if a given character is acceptable as
// the first character of an identifier.
//
// For example, ¾ (U+00BE) is a valid identifier because characters in
// 0x00BE-0x00C0 are allowed, while neither ⟘ (U+27D8) nor '　'
// (U+3000, full-width space) are allowed because they are out of range.
static bool is_ident1(uint32_t c) {
  static uint32_t range[] = {
    '_', '_', 'a', 'z', 'A', 'Z', '$', '$',
    0x00A8, 0x00A8, 0x00AA, 0x00AA, 0x00AD, 0x00AD, 0x00AF, 0x00AF,
    0x00B2, 0x00B5, 0x00B7, 0x00BA, 0x00BC, 0x00BE, 0x00C0, 0x00D6,
    0x00D8, 0x00F6, 0x00F8, 0x00FF, 0x0100, 0x02FF, 0x0370, 0x167F,
    0x1681, 0x180D, 0x180F, 0x1DBF, 0x1E00, 0x1FFF, 0x200B, 0x200D,
    0x202A, 0x202E, 0x203F, 0x2040, 0x2054, 0x2054, 0x2060, 0x206F,
    0x2070, 0x20CF, 0x2100, 0x218F, 0x2460, 0x24FF, 0x2776, 0x2793,
    0x2C00, 0x2DFF, 0x2E80, 0x2FFF, 0x3004, 0x3007, 0x3021, 0x302F,
    0x3031, 0x303F, 0x3040, 0xD7FF, 0xF900, 0xFD3D, 0xFD40, 0xFDCF,
    0xFDF0, 0xFE1F, 0xFE30, 0xFE44, 0xFE47, 0xFFFD,
    0x10000, 0x1FFFD, 0x20000, 0x2FFFD, 0x30000, 0x3FFFD, 0x40000, 0x4FFFD,
    0x50000, 0x5FFFD, 0x60000, 0x6FFFD, 0x70000, 0x7FFFD, 0x80000, 0x8FFFD,
    0x90000, 0x9FFFD, 0xA0000, 0xAFFFD, 0xB0000, 0xBFFFD, 0xC0000, 0xCFFFD,
    0xD0000, 0xDFFFD, 0xE0000, 0xEFFFD, -1,
  };

  return in_range(range, c);
}

// Returns true if a given character is acceptable as a non-first
// character of an identifier.
static bool is_ident2(uint32_t c) {
  static uint32_t range[] = {
    '0', '9', '$', '$', 0x0300, 0x036F, 0x1DC0, 0x1DFF, 0x20D0, 0x20FF,
    0xFE20, 0xFE2F, -1,
  };

  return is_ident1(c) || in_range(range, c);
}

// Returns the number of columns needed to display a given
// character in a fixed-width font.
//
// Based on https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
static int char_width(uint32_t c) {
  static uint32_t range1[] = {
    0x0000, 0x001F, 0x007f, 0x00a0, 0x0300, 0x036F, 0x0483, 0x0486,
    0x0488, 0x0489, 0x0591, 0x05BD, 0x05BF, 0x05BF, 0x05C1, 0x05C2,
    0x05C4, 0x05C5, 0x05C7, 0x05C7, 0x0600, 0x0603, 0x0610, 0x0615,
    0x064B, 0x065E, 0x0670, 0x0670, 0x06D6, 0x06E4, 0x06E7, 0x06E8,
    0x06EA, 0x06ED, 0x070F, 0x070F, 0x0711, 0x0711, 0x0730, 0x074A,
    0x07A6, 0x07B0, 0x07EB, 0x07F3, 0x0901, 0x0902, 0x093C, 0x093C,
    0x0941, 0x0948, 0x094D, 0x094D, 0x0951, 0x0954, 0x0962, 0x0963,
    0x0981, 0x0981, 0x09BC, 0x09BC, 0x09C1, 0x09C4, 0x09CD, 0x09CD,
    0x09E2, 0x09E3, 0x0A01, 0x0A02, 0x0A3C, 0x0A3C, 0x0A41, 0x0A42,
    0x0A47, 0x0A48, 0x0A4B, 0x0A4D, 0x0A70, 0x0A71, 0x0A81, 0x0A82,
    0x0ABC, 0x0ABC, 0x0AC1, 0x0AC5, 0x0AC7, 0x0AC8, 0x0ACD, 0x0ACD,
    0x0AE2, 0x0AE3, 0x0B01, 0x0B01, 0x0B3C, 0x0B3C, 0x0B3F, 0x0B3F,
    0x0B41, 0x0B43, 0x0B4D, 0x0B4D, 0x0B56, 0x0B56, 0x0B82, 0x0B82,
    0x0BC0, 0x0BC0, 0x0BCD, 0x0BCD, 0x0C3E, 0x0C40, 0x0C46, 0x0C48,
    0x0C4A, 0x0C4D, 0x0C55, 0x0C56, 0x0CBC, 0x0CBC, 0x0CBF, 0x0CBF,
    0x0CC6, 0x0CC6, 0x0CCC, 0x0CCD, 0x0CE2, 0x0CE3, 0x0D41, 0x0D43,
    0x0D4D, 0x0D4D, 0x0DCA, 0x0DCA, 0x0DD2, 0x0DD4, 0x0DD6, 0x0DD6,
    0x0E31, 0x0E31, 0x0E34, 0x0E3A, 0x0E47, 0x0E4E, 0x0EB1, 0x0EB1,
    0x0EB4, 0x0EB9, 0x0EBB, 0x0EBC, 0x0EC8, 0x0ECD, 0x0F18, 0x0F19,
    0x0F35, 0x0F35, 0x0F37, 0x0F37, 0x0F39, 0x0F39, 0x0F71, 0x0F7E,
    0x0F80, 0x0F84, 0x0F86, 0x0F87, 0x0F90, 0x0F97, 0x0F99, 0x0FBC,
    0x0FC6, 0x0FC6, 0x102D, 0x1030, 0x1032, 0x1032, 0x1036, 0x1037,
    0x1039, 0x1039, 0x1058, 0x1059, 0x1160, 0x11FF, 0x135F, 0x135F,
    0x1712, 0x1714, 0x1732, 0x1734, 0x1752, 0x1753, 0x1772, 0x1773,
    0x17B4, 0x17B5, 0x17B7, 0x17BD, 0x17C6, 0x17C6, 0x17C9, 0x17D3,
    0x17DD, 0x17DD, 0x180B, 0x180D, 0x18A9, 0x18A9, 0x1920, 0x1922,
    0x1927, 0x1928, 0x1932, 0x1932, 0x1939, 0x193B, 0x1A17, 0x1A18,
    0x1B00, 0x1B03, 0x1B34, 0x1B34, 0x1B36, 0x1B3A, 0x1B3C, 0x1B3C,
    0x1B42, 0x1B42, 0x1B6B, 0x1B73, 0x1DC0, 0x1DCA, 0x1DFE, 0x1DFF,
    0x200B, 0x200F, 0x202A, 0x202E, 0x2060, 0x2063, 0x206A, 0x206F,
    0x20D0, 0x20EF, 0x302A, 0x302F, 0x3099, 0x309A, 0xA806, 0xA806,
    0xA80B, 0xA80B, 0xA825, 0xA826, 0xFB1E, 0xFB1E, 0xFE00, 0xFE0F,
    0xFE20, 0xFE23, 0xFEFF, 0xFEFF, 0xFFF9, 0xFFFB, 0x10A01, 0x10A03,
    0x10A05, 0x10A06, 0x10A0C, 0x10A0F, 0x10A38, 0x10A3A, 0x10A3F, 0x10A3F,
    0x1D167, 0x1D169, 0x1D173, 0x1D182, 0x1D185, 0x1D18B, 0x1D1AA, 0x1D1AD,
    0x1D242, 0x1D244, 0xE0001, 0xE0001, 0xE0020, 0xE007F, 0xE0100, 0xE01EF,
    -1,
  };

  if (in_range(range1, c))
    return 0;

  static uint32_t range2[] = {
    0x1100, 0x115F, 0x2329, 0x2329, 0x232A, 0x232A, 0x2E80, 0x303E,
    0x3040, 0xA4CF, 0xAC00, 0xD7A3, 0xF900, 0xFAFF, 0xFE10, 0xFE19,
    0xFE30, 0xFE6F, 0xFF00, 0xFF60, 0xFFE0, 0xFFE6, 0x1F000, 0x1F644,
    0x20000, 0x2FFFD, 0x30000, 0x3FFFD, -1,
  };

  if (in_range(range2, c))
    return 2;
  return 1;
}

// Returns the number of columns needed to display a given
// string in a fixed-width font.
static int display_width(const char *p, int len) {
  const char *start = p;
  int w = 0;
  while (p - start < len) {
    uint32_t c = decode_utf8(&p, p);
    w += char_width(c);
  }
  return w;
}

#include "normalize.h"

// Input file
static File *current_file;

// A list of all input files.
static File **input_files;

// True if the current position is at the beginning of a line
static bool at_bol;

// True if the current position follows a space character
static bool has_space;

// Reports an error and exit.
noreturn void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verrorf(fmt,ap);
  abort();
}

int errorf(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int n = verrorf(fmt, ap);
  va_end(ap);
  return n;
}

// Reports an error message in the following format.
//
// foo.c:10: x = y + 1;
//               ^ <error message here>
static void verror_at(char *filename, const char *input, int line_no,
                      const char *loc, char *fmt, va_list ap) {
  // Find a line containing `loc`.
  const char *line = loc;
  while (input < line && line[-1] != '\n')
    line--;

  const char *end = loc;
  while (*end && *end != '\n')
    end++;

  // Print out the line.
  int indent = errorf("%s:%d: ", filename, line_no);
  errorf("%.*s\n", (int)(end - line), line);

  // Show the error message.
  int pos = display_width(line, loc - line) + indent;

  errorf("%*s", pos, ""); // print pos spaces.
  errorf("^ ");
  verrorf(fmt, ap);
  errorf("\n");
}

void error_at(const char *loc, char *fmt, ...) {
  int line_no = 1;
  for (const char *p = current_file->contents; p < loc; p++)
    if (*p == '\n')
      line_no++;

  va_list ap;
  va_start(ap, fmt);
  verror_at(current_file->name, current_file->contents, line_no, loc, fmt, ap);
  abort();
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->file->name, tok->file->contents, tok->line_no, tok->loc, fmt, ap);
  abort();
}

void warn_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->file->name, tok->file->contents, tok->line_no, tok->loc, fmt, ap);
  va_end(ap);
}

// Consumes the current token if it matches `op`.
bool equal(Token *tok, char *op) {
  return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is `op`.
Token *skip(Token *tok, char *op) {
  if (!equal(tok, op))
    error_tok(tok, "expected '%s'", op);
  return tok->next;
}

bool consume(Token **rest, Token *tok, char *str) {
  if (equal(tok, str)) {
    *rest = tok->next;
    return true;
  }
  *rest = tok;
  return false;
}

// Create a new token.
static Token *new_token(TokenKind kind, const char *start, const char *end) {
  Token *tok = scratch_calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  tok->file = current_file;
  tok->filename = current_file->display_name;
  tok->at_bol = at_bol;
  tok->has_space = has_space;

  at_bol = has_space = false;
  return tok;
}

// Read an identifier and returns the length of it.
// If p does not point to a valid identifier, 0 is returned.
static int read_ident(const char *start) {
  const char *p = start;
  uint32_t c = decode_utf8(&p, p);
  if (!is_ident1(c))
    return 0;

  for (;;) {
    const char *q;
    c = decode_utf8(&q, p);
    if (!is_ident2(c))
      return p - start;
    p = q;
  }
}

// Read a punctuator token from p and returns its length.
static int read_punct(const char *p) {
  static char *kw[] = {
    "<<=", ">>=", "...", "==", "!=", "<=", ">=", "->", "+=",
    "-=", "*=", "/=", "++", "--", "%=", "&=", "|=", "^=", "&&",
    "||", "<<", ">>", "##",
  };

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (startswith(p, kw[i]))
      return strlen(kw[i]);

  return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(Token *tok) {
  static HashMap map;

  if (map.capacity == 0) {
    static char *kw[] = {
      "return", "if", "else", "for", "while", "int", "sizeof", "char",
      "struct", "union", "short", "long", "void", "typedef", "_Bool",
      "enum", "static", "goto", "break", "continue", "switch", "case",
      "default", "extern", "_Alignof", "_Alignas", "do", "signed",
      "unsigned", "const", "volatile", "auto", "register", "restrict",
      "__restrict", "__restrict__", "_Noreturn", "float", "double",
      "typeof", "asm", "_Thread_local", "__thread", "_Atomic",
      "__attribute__",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
      hashmap_put(&map, kw[i], (void *)1);
  }

  return hashmap_get2(&map, tok->loc, tok->len);
}

static int read_escaped_char(const char **new_pos, const char *p) {
  if ('0' <= *p && *p <= '7') {
    // Read an octal number.
    int c = *p++ - '0';
    if ('0' <= *p && *p <= '7') {
      c = (c << 3) + (*p++ - '0');
      if ('0' <= *p && *p <= '7')
        c = (c << 3) + (*p++ - '0');
    }
    *new_pos = p;
    return c;
  }

  if (*p == 'x') {
    // Read a hexadecimal number.
    p++;
    if (!isxdigit(*p))
      error_at(p, "invalid hex escape sequence");

    int c = 0;
    for (; isxdigit(*p); p++)
      c = (c << 4) + from_hex(*p);
    *new_pos = p;
    return c;
  }

  *new_pos = p + 1;

  // Escape sequences are defined using themselves here. E.g.
  // '\n' is implemented using '\n'. This tautological definition
  // works because the compiler that compiles our compiler knows
  // what '\n' actually is. In other words, we "inherit" the ASCII
  // code of '\n' from the compiler that compiles our compiler,
  // so we don't have to teach the actual code here.
  //
  // This fact has huge implications not only for the correctness
  // of the compiler but also for the security of the generated code.
  // For more info, read "Reflections on Trusting Trust" by Ken Thompson.
  // https://github.com/rui314/chibicc/wiki/thompson1984.pdf
  switch (*p) {
  case 'a': return '\a';
  case 'b': return '\b';
  case 't': return '\t';
  case 'n': return '\n';
  case 'v': return '\v';
  case 'f': return '\f';
  case 'r': return '\r';
  // [GNU] \e for the ASCII escape character is a GNU C extension.
  case 'e': return 27;
  default: return *p;
  }
}

// Find a closing double-quote.
static const char *string_literal_end(const char *p) {
  const char *start = p;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0')
      error_at(start, "unclosed string literal");
    if (*p == '\\')
      p++;
  }
  return p;
}

static Token *read_string_literal(const char *start, const char *quote) {
  const char *end = string_literal_end(quote + 1);
  char *buf = scratch_calloc(1, end - quote);
  int len = 0;

  for (const char *p = quote + 1; p < end;) {
    if (*p == '\\')
      buf[len++] = read_escaped_char(&p, p + 1);
    else
      buf[len++] = *p++;
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->ty = array_of(ty_char, len + 1);
  tok->str = buf;
  return tok;
}

// Read a UTF-8-encoded string literal and transcode it in UTF-16.
//
// UTF-16 is yet another variable-width encoding for Unicode. Code
// points smaller than U+10000 are encoded in 2 bytes. Code points
// equal to or larger than that are encoded in 4 bytes. Each 2 bytes
// in the 4 byte sequence is called "surrogate", and a 4 byte sequence
// is called a "surrogate pair".
static Token *read_utf16_string_literal(const char *start, const char *quote) {
  const char *end = string_literal_end(quote + 1);
  uint16_t *buf = scratch_calloc(2, end - start);
  int len = 0;

  for (const char *p = quote + 1; p < end;) {
    if (*p == '\\') {
      buf[len++] = read_escaped_char(&p, p + 1);
      continue;
    }

    uint32_t c = decode_utf8(&p, p);
    if (c < 0x10000) {
      // Encode a code point in 2 bytes.
      buf[len++] = c;
    } else {
      // Encode a code point in 4 bytes.
      c -= 0x10000;
      buf[len++] = 0xd800 + ((c >> 10) & 0x3ff);
      buf[len++] = 0xdc00 + (c & 0x3ff);
    }
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->ty = array_of(ty_ushort, len + 1);
  tok->str = (char *)buf;
  return tok;
}

// Read a UTF-8-encoded string literal and transcode it in UTF-32.
//
// UTF-32 is a fixed-width encoding for Unicode. Each code point is
// encoded in 4 bytes.
static Token *read_utf32_string_literal(const char *start, const char *quote, Type *ty) {
  const char *end = string_literal_end(quote + 1);
  uint32_t *buf = scratch_calloc(4, end - quote);
  int len = 0;

  for (const char *p = quote + 1; p < end;) {
    if (*p == '\\')
      buf[len++] = read_escaped_char(&p, p + 1);
    else
      buf[len++] = decode_utf8(&p, p);
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->ty = array_of(ty, len + 1);
  tok->str = (char *)buf;
  return tok;
}

static Token *read_char_literal(const char *start, const char *quote, Type *ty) {
  const char *p = quote + 1;
  if (*p == '\0')
    error_at(start, "unclosed char literal");

  int c;
  if (*p == '\\')
    c = read_escaped_char(&p, p + 1);
  else
    c = decode_utf8(&p, p);

  char *end = strchr(p, '\'');
  if (!end)
    error_at(p, "unclosed char literal");

  Token *tok = new_token(TK_NUM, start, end + 1);
  tok->val = c;
  tok->ty = ty;
  return tok;
}

static bool convert_pp_int(Token *tok) {
  const char *p = tok->loc;

  // Read a binary, octal, decimal or hexadecimal number.
  int base = 10;
  if (!strncasecmp(p, "0x", 2) && isxdigit(p[2])) {
    p += 2;
    base = 16;
  } else if (!strncasecmp(p, "0b", 2) && (p[2] == '0' || p[2] == '1')) {
    p += 2;
    base = 2;
  } else if (*p == '0') {
    base = 8;
  }

  int64_t val = strtoul(p, NULL, base);

  // Read U, L or LL suffixes.
  bool l = false;
  bool u = false;

  if (startswith(p, "LLU") || startswith(p, "LLu") ||
      startswith(p, "llU") || startswith(p, "llu") ||
      startswith(p, "ULL") || startswith(p, "Ull") ||
      startswith(p, "uLL") || startswith(p, "ull")) {
    p += 3;
    l = u = true;
  } else if (!strncasecmp(p, "lu", 2) || !strncasecmp(p, "ul", 2)) {
    p += 2;
    l = u = true;
  } else if (startswith(p, "LL") || startswith(p, "ll")) {
    p += 2;
    l = true;
  } else if (*p == 'L' || *p == 'l') {
    p++;
    l = true;
  } else if (*p == 'U' || *p == 'u') {
    p++;
    u = true;
  }

  if (p != tok->loc + tok->len)
    return false;

  // Infer a type.
  Type *ty;
  if (base == 10) {
    if (l && u)
      ty = ty_ulong;
    else if (l)
      ty = ty_long;
    else if (u)
      ty = (val >> 32) ? ty_ulong : ty_uint;
    else
      ty = (val >> 31) ? ty_long : ty_int;
  } else {
    if (l && u)
      ty = ty_ulong;
    else if (l)
      ty = (val >> 63) ? ty_ulong : ty_long;
    else if (u)
      ty = (val >> 32) ? ty_ulong : ty_uint;
    else if (val >> 63)
      ty = ty_ulong;
    else if (val >> 32)
      ty = ty_long;
    else if (val >> 31)
      ty = ty_uint;
    else
      ty = ty_int;
  }

  tok->kind = TK_NUM;
  tok->val = val;
  tok->ty = ty;
  return true;
}

// The definition of the numeric literal at the preprocessing stage
// is more relaxed than the definition of that at the later stages.
// In order to handle that, a numeric literal is tokenized as a
// "pp-number" token first and then converted to a regular number
// token after preprocessing.
//
// This function converts a pp-number token to a regular number token.
static void convert_pp_number(Token *tok) {
  // Try to parse as an integer constant.
  if (convert_pp_int(tok))
    return;

  // If it's not an integer, it must be a floating point constant.
  char *end;
  double val = strtod(tok->loc, &end);

  Type *ty;
  if (*end == 'f' || *end == 'F') {
    ty = ty_float;
    end++;
  } else if (*end == 'l' || *end == 'L') {
    ty = ty_ldouble;
    end++;
  } else {
    ty = ty_double;
  }

  if (tok->loc + tok->len != end)
    error_tok(tok, "invalid numeric constant");

  tok->kind = TK_NUM;
  tok->fval = val;
  tok->ty = ty;
}

void convert_pp_tokens(Token *tok) {
  for (Token *t = tok; t->kind != TK_EOF; t = t->next) {
    if (is_keyword(t))
      t->kind = TK_KEYWORD;
    else if (t->kind == TK_PP_NUM)
      convert_pp_number(t);
  }
}

// Initialize line info for all tokens.
static void add_line_numbers(Token *tok) {
  const char *p = current_file->contents;
  int n = 1;

  do {
    if (p == tok->loc) {
      tok->line_no = n;
      tok = tok->next;
    }
    if (*p == '\n')
      n++;
  } while (*p++);
}

Token *tokenize_string_literal(Token *tok, Type *basety) {
  Token *t;
  if (basety->size == 2)
    t = read_utf16_string_literal(tok->loc, tok->loc);
  else
    t = read_utf32_string_literal(tok->loc, tok->loc, basety);
  t->next = tok->next;
  return t;
}

// Tokenize a given string and returns new tokens.
Token *tokenize(File *file) {
  current_file = file;

  const char *p = file->contents;
  Token head = {0};
  Token *cur = &head;

  at_bol = true;
  has_space = false;

  while (*p) {
    // Skip line comments.
    if (startswith(p, "//")) {
      p += 2;
      while (*p != '\n')
        p++;
      has_space = true;
      continue;
    }

    // Skip block comments.
    if (startswith(p, "/*")) {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed block comment");
      p = q + 2;
      has_space = true;
      continue;
    }

    // Skip newline.
    if (*p == '\n') {
      p++;
      at_bol = true;
      has_space = false;
      continue;
    }

    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      has_space = true;
      continue;
    }

    // Numeric literal
    if (isdigit(*p) || (*p == '.' && isdigit(p[1]))) {
      const char *q = p++;
      for (;;) {
        if (p[0] && p[1] && strchr("eEpP", p[0]) && strchr("+-", p[1]))
          p += 2;
        else if (isalnum(*p) || *p == '.')
          p++;
        else
          break;
      }
      cur = cur->next = new_token(TK_PP_NUM, q, p);
      continue;
    }

    // String literal
    if (*p == '"') {
      cur = cur->next = read_string_literal(p, p);
      p += cur->len;
      continue;
    }

    // UTF-8 string literal
    if (startswith(p, "u8\"")) {
      cur = cur->next = read_string_literal(p, p + 2);
      p += cur->len;
      continue;
    }

    // UTF-16 string literal
    if (startswith(p, "u\"")) {
      cur = cur->next = read_utf16_string_literal(p, p + 1);
      p += cur->len;
      continue;
    }

    // Wide string literal
    if (startswith(p, "L\"")) {
      cur = cur->next = read_utf32_string_literal(p, p + 1, ty_int);
      p += cur->len;
      continue;
    }

    // UTF-32 string literal
    if (startswith(p, "U\"")) {
      cur = cur->next = read_utf32_string_literal(p, p + 1, ty_uint);
      p += cur->len;
      continue;
    }

    // Character literal
    if (*p == '\'') {
      cur = cur->next = read_char_literal(p, p, ty_int);
      cur->val = (char)cur->val;
      p += cur->len;
      continue;
    }

    // UTF-16 character literal
    if (startswith(p, "u'")) {
      cur = cur->next = read_char_literal(p, p + 1, ty_ushort);
      cur->val &= 0xffff;
      p += cur->len;
      continue;
    }

    // Wide character literal
    if (startswith(p, "L'")) {
      cur = cur->next = read_char_literal(p, p + 1, ty_int);
      p += cur->len;
      continue;
    }

    // UTF-32 character literal
    if (startswith(p, "U'")) {
      cur = cur->next = read_char_literal(p, p + 1, ty_uint);
      p += cur->len;
      continue;
    }

    // Identifier or keyword
    int ident_len = read_ident(p);
    if (ident_len) {
      cur = cur->next = new_token(TK_IDENT, p, p + ident_len);
      p += cur->len;
      continue;
    }

    // Punctuators
    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
      p += cur->len;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  add_line_numbers(head.next);
  return head.next;
}

File **get_input_files(void) {
  return input_files;
}

File *new_file(char *name, int file_no, const char *contents) {
  File *file = scratch_calloc(1, sizeof(File));
  file->name = name;
  file->display_name = name;
  file->file_no = file_no;
  file->contents = contents;
  return file;
}

static const char* get_source_file(const char* path)
{
  for (int i=0; i < num_gen_includes; i++) {
    if (strcmp(path, gen_includes[i][0]) == 0) return gen_includes[i][1];
  }
  return read_source_file(path);
}

Token *tokenize_file(char *path) {
  const char *p = get_source_file(path);
  if (!p)
    return NULL;

  // Save the filename for assembler .file directive.
  static int file_no;
  File *file = new_file(path, file_no + 1, p);

  // Save the filename for assembler .file directive.
  input_files = scratch_realloc(input_files, sizeof(char *) * (file_no + 2));
  input_files[file_no] = file;
  input_files[file_no + 1] = NULL;
  file_no++;

  return tokenize(file);
}
