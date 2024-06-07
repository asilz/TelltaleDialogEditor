#include <stdint.h>
#include <stddef.h>

uint64_t CRC64_CaseInsensitive(uint64_t crc, uint8_t *buf);
uint64_t CRC64(uint64_t crc, const char *const buf);
uint16_t searchDatabase(char *databasePath, uint64_t crc);
void writeDatabase();
void binWalk(FILE *stream);