/* JSON_checker.h */

typedef struct JSON_checker_struct {
    int state;
    int depth;
    int top;
    int* stack;
} * JSON_checker;


int __CTIFFIsValidJSON(const char* json);
const char* __CTIFFTarValidExtMeta(const char* json);
