#include <inttypes.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC int renderSymbol(struct TreeNode *node, uint32_t flags);
EXTERNC int renderFloat(struct TreeNode *node, uint32_t flags);
EXTERNC int renderInt4(struct TreeNode *node, uint32_t flags);
EXTERNC int renderBool(struct TreeNode *node, uint32_t flags);
EXTERNC int renderString(struct TreeNode *node, uint32_t flags);
EXTERNC int renderVector3(struct TreeNode *node, uint32_t flags);
EXTERNC int renderColor(struct TreeNode *node, uint32_t flags);