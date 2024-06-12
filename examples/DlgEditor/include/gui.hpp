#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <application.h>

extern "C"
{
#include <dlg.h>
#include <meta.h>
}

#ifdef _MSC_VER
#define portable_strcpy strcpy_s
#define portable_sprintf sprintf_s
#else
#define portable_strcpy strcpy
#define portable_sprintf sprintf
#endif

namespace ed = ax::NodeEditor;

struct DlgApplication : public Application
{
    using Application::Application;

    struct NodePin;
    struct LinkInfo;

    struct Node
    {
        struct TreeNode *data;
        NodePin *prevPin;
        NodePin *nextPin;
    };

    struct NodePin
    {
        Node *parentNode;
        LinkInfo *link;
        enum PinType
        {
            prev,
            next
        } pinType;
    };

    struct LinkInfo
    {
        NodePin *srcPin;
        NodePin *destPin;
    };

    void OnStart() override;

    void OnStop() override;

    void OnFrame(float deltaTime) override;

    void CreateLinks();

    DlgApplication(const char *name, FILE *inputStream);

    void AddNextLinks(Node *node);

    ed::EditorContext *m_Context = nullptr;

    struct TreeNode dlg;
    struct TreeNode *copiedNode;
    MetaStreamHeader header;
    ImVector<Node *> nodes;
};
