#include "chibicc.h"

char *base_file; // XXX?

int verrorf(const char* fmt, va_list ap)
{
  assert(!"TODO");
}

const char *read_source_file(const char *path)
{
  assert(!"TODO");
}

void* scratch_calloc(size_t number, size_t size)
{
  return calloc(number, size);
}

void* scratch_realloc(void *ptr, size_t size)
{
  return realloc(ptr, size);
}

static void test_hashmap(void)
{
  // NOTE(aks): this was originally placed in hashmap.c and exposed via the
  // chibicc executable
  HashMap *map = calloc(1, sizeof(HashMap));

  for (int i = 0; i < 5000; i++)
    hashmap_put(map, format("key %d", i), (void *)(size_t)i);
  for (int i = 1000; i < 2000; i++)
    hashmap_delete(map, format("key %d", i));
  for (int i = 1500; i < 1600; i++)
    hashmap_put(map, format("key %d", i), (void *)(size_t)i);
  for (int i = 6000; i < 7000; i++)
    hashmap_put(map, format("key %d", i), (void *)(size_t)i);

  for (int i = 0; i < 1000; i++)
    assert((size_t)hashmap_get(map, format("key %d", i)) == i);
  for (int i = 1000; i < 1500; i++)
    assert(hashmap_get(map, "no such key") == NULL);
  for (int i = 1500; i < 1600; i++)
    assert((size_t)hashmap_get(map, format("key %d", i)) == i);
  for (int i = 1600; i < 2000; i++)
    assert(hashmap_get(map, "no such key") == NULL);
  for (int i = 2000; i < 5000; i++)
    assert((size_t)hashmap_get(map, format("key %d", i)) == i);
  for (int i = 5000; i < 6000; i++)
    assert(hashmap_get(map, "no such key") == NULL);
  for (int i = 6000; i < 7000; i++)
    hashmap_put(map, format("key %d", i), (void *)(size_t)i);

  assert(hashmap_get(map, "no such key") == NULL);
}

static void test_normalize_source_string(void)
{
  { // test no-op
    char* s0 = strdup("foo\nbar\n");
    char* s1 = normalize_source_string(s0, strlen(s0)+1);
    assert(strcmp(s1, "foo\nbar\n") == 0);
    assert((s1 == s0) && "expected same ptr as input");
  }

  { // test BOM removal
    char* s0 = strdup("\xef\xbb\xbf foo\nbar\n");
    char* s1 = normalize_source_string(s0, strlen(s0)+1);
    assert(strcmp(s1, " foo\nbar\n") == 0);
    assert((s1 == s0+3) && "expected string to be relocated");
  }

  { // test insertion of missing \n
    char* s0 = strdup("foo\nbar");
    const size_t cap = strlen(s0)+1;
    char* s1 = normalize_source_string(s0, cap);
    assert(strcmp(s1, "foo\nbar\n") == 0);
    // NOTE(aks) tempting to test that pointer has been reloacted but I might
    // want to realloc() it? (which may return the same pointer)
  }

  { // test unicode escape normalization
    char* s0 = strdup(" \"h\\u00e6ll\\u00f8\" \n");
    char* s1 = normalize_source_string(s0, strlen(s0)+1);
    assert(strcmp(s1, " \"hællø\" \n") == 0);
  }

  { // test backslash+newline replacement
    {
      char* s0 = strdup("foo \\\nbar\nbaz\n");
      char* s1 = normalize_source_string(s0, strlen(s0)+1);
      assert(strcmp(s1, "foo bar\n\nbaz\n") == 0);
    }
    {
      char* s0 = strdup("foo \\\nbar \\\nbaz\n");
      char* s1 = normalize_source_string(s0, strlen(s0)+1);
      assert(strcmp(s1, "foo bar baz\n\n\n") == 0);
    }
  }
}

void test_all(void)
{
  test_hashmap();
  test_normalize_source_string();
  init_macros(); // XXX not really a test?
}

#if defined( USE_LIBC )

int main(int argc, char** argv)
{
  test_all();
  printf("unittest OK\n");
  return 0;
}

#elif defined( USE_LIBC_ANYPCT_WASM32 )

__attribute__((visibility("default")))
void run_tests(void)
{
  test_all();
}

#else
#error "missing USE_* define"
#endif
