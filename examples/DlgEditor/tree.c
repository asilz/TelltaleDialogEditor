#include <tree.h>
#include <stdlib.h>
#include <stdio.h>
#include <stream.h>
#include <string.h>
#include <assert.h>

void treeFree(struct TreeNode *root)
{
    if (root->sibling != NULL)
    {
        treeFree(root->sibling);
        free(root->sibling);
    }
    if (root->dataSize > sizeof(root->staticBuffer))
    {
        free(root->dynamicBuffer);
    }
    else if (root->dataSize == 0 && root->child != NULL)
    {
        treeFree(root->child);
        free(root->child);
    }
}

void morrisFree(struct TreeNode *root)
{
    // left is sibling
    // right is child
    root = root->child;

    while (root)
    {
        if (root->sibling == NULL)
        {
            if (root->dataSize > 0)
            {
                if (root->dataSize > sizeof(root->staticBuffer))
                {
                    free(root->dynamicBuffer);
                }
                root->child = NULL;
            }
            struct TreeNode *tmp = root;
            root = root->child;
            free(tmp);
        }
        else
        {
            struct TreeNode *current = root->sibling;
            while (current->child != NULL && current->dataSize == 0 && current->child != root)
            {
                current = current->child;
            }

            if (current->child == NULL || current->dataSize > 0)
            {
                current->child = root;
                root = root->sibling;
            }
            else
            {
                if (current->child != NULL && current->dataSize > sizeof(root->staticBuffer))
                {
                    free(current->dynamicBuffer);
                }
                current->child = NULL;
                current = root->sibling;
                if (root->dynamicBuffer != NULL && root->dataSize > 8)
                {
                    free(root->dynamicBuffer);
                }
                free(root);
                root = current;
            }
        }
    }
}

uint32_t writeTree(FILE *stream, const struct TreeNode *root)
{
    if (root->dataSize > 0)
    {
        if (root->dataSize > sizeof(root->staticBuffer))
        {
            return fwrite(root->dynamicBuffer, 1, root->dataSize, stream);
        }
        else
        {
            return fwrite(root->staticBuffer, 1, root->dataSize, stream);
        }
    }

    uint32_t ret = 0;
    root = root->child;
    while (root != NULL)
    {
        if (root->serializeType)
        {
            ret += fwrite(&(root->description->crc), 1, sizeof(root->description->crc), stream);
        }
        if (root->isBlocked)
        {
            size_t childSize = fwrite(&ret, 1, sizeof(ret), stream);
            childSize += writeTree(stream, root);
            cfseek(stream, -(int64_t)childSize, SEEK_CUR);
            fwrite(&childSize, 1, sizeof(uint32_t), stream);
            cfseek(stream, childSize - sizeof(uint32_t), SEEK_CUR);
            ret += childSize;
        }
        else
        {
            ret += writeTree(stream, root);
        }
        root = root->sibling;
    }
    return ret;
}
