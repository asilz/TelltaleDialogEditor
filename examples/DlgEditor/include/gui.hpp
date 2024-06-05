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

    struct NodePin
    {
        struct TreeNode *parentNode;
        enum PinType
        {
            prev,
            next
        } pinType;
    };

    struct LinkInfo
    {
        struct NodePin *srcNode;
        struct NodePin *destNode;
    };

    void OnStart() override;

    void OnStop() override;

    void OnFrame(float deltaTime) override;

    void CreateLinks();

    DlgApplication(const char *name, FILE *inputStream);

    void AddNextLinks(struct TreeNode *node);

    ed::EditorContext *m_Context = nullptr;

    struct TreeNode dlg;
    MetaStreamHeader header;
    ImVector<LinkInfo> m_Links;
    ImVector<NodePin> m_Nodes;
};
