#include "hphp/runtime/base/base-includes.h"
#include <bson.h>
#include "encode.h"
#include "decode.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////////////
// functions
static Array HHVM_FUNCTION(bson_decode, const String& bson) {
  bson_reader_t * reader;
  const bson_t * bsonObj;
  bool reached_eof;

  Array output = Array::Create();

  reader = bson_reader_new_from_data((uint8_t *)bson.c_str(), bson.size());
  bsonObj = bson_reader_read(reader, &reached_eof);

  bsonToVariant(bsonObj, &output);
  bson_reader_destroy(reader);

  return output;
}

static Array HHVM_FUNCTION(bson_decode_multiple, const String& bson) {
  bson_reader_t * reader;
  const bson_t * bsonObj;
  bool reached_eof;

  int i = 0;
  Array output = Array::Create();

  reader = bson_reader_new_from_data((uint8_t *)bson.c_str(), bson.size());
  while ((bsonObj = bson_reader_read(reader, &reached_eof))) {
    Array document = Array::Create();
    bsonToVariant(bsonObj, &document);
    output.add(i++, document);
  }

  bson_reader_destroy(reader);

  return output;
}

static String encode(const Variant& anything) {
  bson_t bson;
  bson_init(&bson);
        
  fillBSONWithArray(anything.toArray(), &bson);

  const char* output = (const char*) bson_get_data(&bson);        
  return String(output, bson.len, CopyString);
}

static String HHVM_FUNCTION(bson_encode, const Variant& anything) {
  return encode(anything);
}

static String HHVM_FUNCTION(bson_encode_multiple, const Array& documents) {
  String output;

  for (ArrayIter iter(documents); iter; ++iter) {
    output = output + f_bson_encode(iter.secondRef());
  }

  return output;
}

//////////////////////////////////////////////////////////////////////////////

class mongoExtension : public Extension {
 public:
  mongoExtension() : Extension("mongo", "1.4.5") {}
  virtual void moduleInit() {
    HHVM_FE(bson_decode);
    HHVM_FE(bson_encode);
    HHVM_FE(bson_decode_multiple);
    HHVM_FE(bson_encode_multiple);
    loadSystemlib();
  }
} s_mongo_extension;

// Uncomment for non-bundled module
HHVM_GET_MODULE(mongo);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
