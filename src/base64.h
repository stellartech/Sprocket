#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED
  
#ifdef __cplusplus
extern "C" {
#endif
  
char*
base64_encode(const unsigned char *inp, int in_len, int *out_len);
  
char*
base64_decode(const unsigned char *inp, int in_len);
  
#ifdef __cplusplus
}
#endif
  
#endif /* BASE64_H_INCLUDED */

