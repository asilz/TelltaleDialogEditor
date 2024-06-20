#include <inttypes.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC int SymbolRender(struct TreeNode *node, uint32_t flags);
EXTERNC int FloatRender(struct TreeNode *node, uint32_t flags);
EXTERNC int BoolRender(struct TreeNode *node, uint32_t flags);
EXTERNC int StringRender(struct TreeNode *node, uint32_t flags);
EXTERNC int Vector3Render(struct TreeNode *node, uint32_t flags);
EXTERNC int ColorRender(struct TreeNode *node, uint32_t flags);
EXTERNC int HandleBaseRender(struct TreeNode *node, uint32_t flags);
EXTERNC int Vector4Render(struct TreeNode *node, uint32_t flags);

EXTERNC int u8Render(struct TreeNode *node, uint32_t flags);
EXTERNC int s8Render(struct TreeNode *node, uint32_t flags);
EXTERNC int u16Render(struct TreeNode *node, uint32_t flags);
EXTERNC int s16Render(struct TreeNode *node, uint32_t flags);
EXTERNC int u32Render(struct TreeNode *node, uint32_t flags);
EXTERNC int s32Render(struct TreeNode *node, uint32_t flags);
EXTERNC int u64Render(struct TreeNode *node, uint32_t flags);
EXTERNC int s64Render(struct TreeNode *node, uint32_t flags);