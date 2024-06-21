// ===================================================================================================================
// Widget Example
// Drawing standard ImGui widgets inside the node body
//
// First, some unsorted notes about which widgets do and don't draw well inside nodes.  Run the examples to see all the allowed widgets.
//
// - Child windows with scrolling doesn't work in the node.  The child window appears in a normal node,
//   and scrolls, but its contents are floating around in the wrong location, and they are not scaled.
//   Note that you can put scrolling child windows into "deferred popups" (see next item).
// - Listboxes and combo-boxes only work in nodes with a work-around: deferring the popup calls until after the node drawing is
//   completed. Look to the popup-demo for an example.
// - Headers and trees work inside the nodes only with hacks.  This is because they attempt to span the "avaialbe width"
//   and the nodes can't tell these widgets how wide it is. The work-around is to set up a fake
//   table with a static column width, then draw your header and tree widgets in that column.
// - Clickable tabs don't work in nodes.  Tabs appear, but you cannot actually change the tab, so they're functionally useless.
// - Editable text areas work, but you have to manually manage disabling the editor shorcuts while typing is detected.
//   Look around for the call to ed::EnableShortcuts() for an example.
// - Most of the cool graph widgets can't be used because they are hard-coded in ImGui to spawn tooltips, which don't work.

#include <gui.hpp>
#include <gui.h>
#include <stdio.h>

extern "C"
{
#include <types.h>
#include <crc64.h>
}

int BoolRender(struct TreeNode *node, uint32_t flags)
{
    bool option = (bool)(*(node->data.staticBuffer) - '0');
    ImGui::Text("Checked: %s", option ? "true" : "false");
    ImGui::Checkbox("Option 1", &option);
    *(node->data.staticBuffer) = (uint8_t)(option) + '0';
    // printf("%d\n", option);
    return 0;
}

int FloatRender(struct TreeNode *node, uint32_t flags)
{
    float f0 = *(float *)(node->data.staticBuffer);
    ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");
    *(float *)(node->data.staticBuffer) = f0;
    return 0;
}

int u8Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_U8, (uint8_t *)node->data.staticBuffer);
}

int s8Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_S8, (int8_t *)node->data.staticBuffer);
}

int u16Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_U16, (uint16_t *)node->data.staticBuffer);
}

int s16Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_S16, (int16_t *)node->data.staticBuffer);
}

int u32Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_U32, (uint32_t *)node->data.staticBuffer);
}

int s32Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_S32, (int32_t *)node->data.staticBuffer);
}

int u64Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_U64, (uint64_t *)node->data.staticBuffer);
}

int s64Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_S64, (int64_t *)node->data.staticBuffer);
}

int SymbolRender(struct TreeNode *node, uint32_t flags)
{
    const char *fileName = getFileName(*(uint64_t *)node->data.staticBuffer);
    char fileNameBuffer[256] = {0};
    char hexBuffer[19];
    snprintf(hexBuffer, 19, "0x%016" PRIX64, *(uint64_t *)node->data.staticBuffer);

    if (fileName != NULL)
    {
        memcpy(fileNameBuffer, fileName, strlen(fileName) + 1);
    }
    bool textIsInput = ImGui::InputText("String", fileNameBuffer, 256);

    bool hexIsInput = ImGui::InputText("Hex", hexBuffer, 19);
    if (hexIsInput)
    {
        sscanf(hexBuffer + 2, "%16" PRIX64, (uint64_t *)(node->data.staticBuffer));
    }

    if (textIsInput)
    {
        *(uint64_t *)(node->data.staticBuffer) = CRC64_CaseInsensitive(0, (uint8_t *)fileNameBuffer);
    }

    return 0;
}

int HandleBaseRender(struct TreeNode *node, uint32_t flags)
{
    SymbolRender(node, flags);
    if (node->dataSize == 2 * sizeof(uint64_t))
    {
        ImGui::InputScalar("input int", ImGuiDataType_::ImGuiDataType_S64, node->data.dynamicBuffer + sizeof(uint64_t));
    }

    return 0;
}

int StringRender(struct TreeNode *node, uint32_t flags)
{
    constexpr uint32_t BUFFER_SIZE = 1024;
    char stringBuffer[BUFFER_SIZE];
    if (node->dataSize <= sizeof(node->data))
    {
        memcpy(stringBuffer, (char *)(node->data.staticBuffer + 4), node->dataSize - 4);
    }
    else
    {
        memcpy(stringBuffer, (char *)(node->data.dynamicBuffer + 4), node->dataSize - 4);
    }
    stringBuffer[node->dataSize - 4] = '\0';
    ImGui::InputText("String", stringBuffer, BUFFER_SIZE);
    size_t stringLength = strlen(stringBuffer);
    if (node->dataSize <= sizeof(node->data) && stringLength + 4 > sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        node->data.dynamicBuffer = (uint8_t *)malloc(node->dataSize);
        *(uint32_t *)node->data.dynamicBuffer = stringLength;
        memcpy(node->data.dynamicBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize > sizeof(node->data) && stringLength + 4 <= sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        free(node->data.dynamicBuffer);
        *(uint32_t *)node->data.staticBuffer = stringLength;
        memcpy(node->data.staticBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize > sizeof(node->data) && stringLength + 4 > sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        free(node->data.dynamicBuffer);
        node->data.dynamicBuffer = (uint8_t *)malloc(stringLength + 4);
        *(uint32_t *)node->data.dynamicBuffer = stringLength;
        memcpy(node->data.dynamicBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize <= sizeof(node->data) && stringLength + 4 <= sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        *(uint32_t *)node->data.staticBuffer = stringLength;
        memcpy(node->data.staticBuffer + 4, stringBuffer, stringLength);
    }
    return 0;
    // return ImGui::InputText("String", (char *)(node->data.dynamicBuffer + 4), *(uint32_t *)(node->data.dynamicBuffer));
}

int Vector3Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputFloat3("xyz", (float *)(node->data.dynamicBuffer));
}

int Vector4Render(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputFloat4("xyzw", (float *)(node->data.dynamicBuffer));
}

int ColorRender(struct TreeNode *node, uint32_t flags)
{
    return ImGui::ColorPicker4("rgba", (float *)(node->data.dynamicBuffer));
}

int renderNode(struct TreeNode *node, int uniqueID, uint32_t flags)
{
    // printf("%d\n", node->childCount);
    if (node->description != NULL && node->description->render != NULL)
    {
        return node->description->render(node, flags);
    }
    if (node->childCount == 0)
    {
        ImGui::Text("Render function not implemented, cannot view content");
        return -4;
    }
    for (uint16_t i = 0; i < node->childCount; ++i)
    {

        char text[256];
        const char *name = "(null)";
        const MetaClassDescription *description = node->children[i]->description;
        if (node->children[i]->description != NULL)
        {
            name = node->children[i]->description->name;
        }
        snprintf(text, 256, "%s##%d-%d", name, uniqueID, i);
        if (ImGui::TreeNode(text))
        {
            renderNode(node->children[i], uniqueID, flags);
            ImGui::TreePop();
        }
    }
    return 0;
}

DlgApplication::DlgApplication(const char *name, FILE *inputStream) : Application(name)
{
    dlg.dataSize = 0;
    dlg.serializeType = 0;
    dlg.isBlocked = 0;

    readMetaStreamHeader(inputStream, &(this->header));
    DlgRead(inputStream, &(this->dlg), 0);
    fclose(inputStream);

    CreateLinks();
    printf("created Links\n");
}

void DlgApplication::CreateLinks()
{
    /* Iteration through all children in each folder */
    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[13]->children[0]->data.staticBuffer); ++i)
    {
        for (uint32_t j = 1; j < *(uint64_t *)(dlg.children[13]->children[i]->children[2]->children[0]->data.staticBuffer); ++j)
        {
            TreeNode *child = dlg.children[13]->children[i]->children[2]->children[j];
            nodes.push_back(new Node{child, nullptr, nullptr});
            AddNextLinks(nodes.back());
        }
    }

    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[14]->children[0]->data.staticBuffer); ++i)
    {
        TreeNode *node = dlg.children[14]->children[i];
        if (node->description->crc == 0x13998d6f0bde3491) // DlgNodeChoices
        {
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[1]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                nodes.push_back(new Node{child, nullptr, nullptr});
                AddNextLinks(nodes.back());
            }
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[2]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                nodes.push_back(new Node{child, nullptr, nullptr});
                AddNextLinks(nodes.back());
            }
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[3]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                nodes.push_back(new Node{child, nullptr, nullptr});
                AddNextLinks(nodes.back());
            }
        }
        else if (node->description->crc == 0x789758cb1a8d6628 || node->description->crc == 0x36e367b48ad63274 || node->description->crc == 0x97ba9139ccc1cf26) // DlgNodeConditional || DlgNodeSequence || DlgNodeParallel
        {
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[1]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                nodes.push_back(new Node{child, nullptr, nullptr});
                AddNextLinks(nodes.back());
            }
        }
    }
}

void DlgApplication::AddNextLinks(Node *node)
{
    node->nextPin = new NodePin{node, nullptr, NodePin::PinType::next};
    // this->pins.push_back(currentNodePin);
    if (*DlgGetNextID(node->data) == 0x988d0903f713877b)
    {
        return;
    }
    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[14]->children[0]->data.staticBuffer); ++i)
    {
        struct TreeNode *nextTreeNode = dlg.children[14]->children[i];
        if (*DlgGetNextID(node->data) == *DlgGetID(nextTreeNode))
        {
            Node *nextNode = new Node{nextTreeNode, nullptr, nullptr};
            nextNode->prevPin = new NodePin{nextNode, nullptr, NodePin::PinType::prev};
            LinkInfo *link = new LinkInfo{node->nextPin, nextNode->prevPin};
            node->nextPin->link = link;
            nextNode->prevPin->link = link;
            nodes.push_back(nextNode);
            // this->pins.push_back(nextNodePin);
            // this->links.push_back(LinkInfo{&pins[pins.size() - 2], &pins[pins.size() - 1]});
            AddNextLinks(nextNode);
        }
    }
}

void AddMember(struct TreeNode *tree, struct TreeNode *member)
{

    treePushBack(tree, member);
}

void DlgApplication::OnStart()
{
    ed::Config config;
    config.SettingsFile = "Widgets.json";
    m_Context = ed::CreateEditor(&config);

    // printf("Read dlog %d, %d\n", links.size(), pins.size());
}

void DlgApplication::OnStop()
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes[i] != nullptr)
        {
            if (nodes[i]->nextPin != nullptr)
            {
                if (nodes[i]->nextPin->link != nullptr)
                {
                    LinkInfo *linkPtr = nodes[i]->nextPin->link;
                    linkPtr->srcPin->link = nullptr;
                    linkPtr->destPin->link = nullptr;
                    delete linkPtr;
                }
                delete nodes[i]->nextPin;
                nodes[i]->nextPin = nullptr;
            }

            if (nodes[i]->prevPin != nullptr)
            {
                if (nodes[i]->prevPin->link != nullptr)
                {
                    LinkInfo *linkPtr = nodes[i]->prevPin->link;
                    linkPtr->srcPin->link = nullptr;
                    linkPtr->destPin->link = nullptr;
                    delete linkPtr;
                }
                delete nodes[i]->prevPin;
                nodes[i]->prevPin = nullptr;
            }
            delete nodes[i];
            nodes[i] = nullptr;
        }
    }
    ed::DestroyEditor(m_Context);
}

void DlgApplication::OnFrame(float deltaTime)
{
    static bool firstframe = true; // Used to position the nodes on startup
    auto &io = ImGui::GetIO();

    // FPS Counter Ribbon
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
    ImGui::Separator();

    // Node Editor Widget
    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    float nodeX = 0;
    float nodeY = 0;

    // printf("loop start\n");

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        float width = 450; // bad magic numbers. used to define width of tree widget
        Node *node = nodes[i];
        ed::BeginNode((uint64_t)node);
        const char *name = "(null)";
        if (node->data->description != NULL)
        {
            name = node->data->description->name;
        }
        ImGui::Text(name);
        if (node->prevPin != nullptr)
        {
            ed::BeginPin((uint64_t)node->prevPin, ed::PinKind::Input);
            ImGui::Text("-> prev");
            ed::EndPin();
        }
        if (node->nextPin != nullptr)
        {
            if (node->prevPin != nullptr)
            {
                ImGui::SameLine();
            }
            ImGui::Dummy(ImVec2(width / 2 + width / 4, 0)); //  magic number - Crude & simple way to nudge over the output pin. Consider using layout and springs
            ImGui::SameLine();
            ed::BeginPin((uint64_t)(node->nextPin), ed::PinKind::Output);
            ImGui::Text("next ->");
            ed::EndPin();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(width, 0));
        ImGui::PopStyleVar();

        // Start columns, but use only first one.
        char textBuffer[128];
        snprintf(textBuffer, 128, "##%s%d", name, i);
        ImGui::BeginColumns(textBuffer, 2,
                            ImGuiOldColumnFlags_NoBorder |
                                ImGuiOldColumnFlags_NoResize |
                                ImGuiOldColumnFlags_NoPreserveWidths |
                                ImGuiOldColumnFlags_NoForceWithinWindow);

        // Adjust column width to match requested one.
        ImGui::SetColumnWidth(0, width + ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x);
        // End of tree column startup --------------------------------------------------------------

        // Back to normal ImGui drawing, in our column.

        if (ImGui::CollapsingHeader(textBuffer))
        {
            renderNode(node->data, i, 0);

            /*
            if (ImGui::TreeNode("Open Tree"))
            {
                static bool OP1_Bool = false;
                ImGui::Text("Checked: %s", OP1_Bool ? "true" : "false");
                ImGui::Checkbox("Option 1", &OP1_Bool);
                ImGui::TreePop();
                // printf("%d\n", OP1_Bool);
            }
            */
        }
        // Tree Column Shutdown
        ImGui::EndColumns();

        ed::EndNode(); // End of Tree Node Demo

        if (firstframe)
        {
            if (node->data->children[0]->description->crc == 0xbfb0ce2bd1f38792)
            {
                nodeX = 0;
                nodeY += 400;
            }
            else
            {
                nodeX += 550;
            }
            ed::SetNodePosition((uint64_t)node, ImVec2(nodeX, nodeY));
        }
    }

    // ==================================================================================================
    // Link Drawing Section

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {

        if (nodes[i]->prevPin != nullptr && !(ed::HasAnyLinks((ed::PinId)(uint64_t)(nodes[i]->prevPin))) && nodes[i]->prevPin->link != nullptr)
        {
            ed::Link((ed::LinkId)(uint64_t)nodes[i]->prevPin->link, (ed::PinId)(uint64_t)(nodes[i]->prevPin->link->srcPin), (ed::PinId)(uint64_t)(nodes[i]->prevPin->link->destPin));
        }

        if (nodes[i]->nextPin != nullptr && !(ed::HasAnyLinks((ed::PinId)(uint64_t)(nodes[i]->nextPin))) && nodes[i]->nextPin->link != nullptr)
        {
            ed::Link((ed::LinkId)(uint64_t)nodes[i]->nextPin->link, (ed::PinId)(uint64_t)(nodes[i]->nextPin->link->srcPin), (ed::PinId)(uint64_t)(nodes[i]->nextPin->link->destPin));
        }
    }

    // ==================================================================================================
    // Interaction Handling Section
    // This was coppied from BasicInteration.cpp. See that file for commented code.

    // Handle creation action ---------------------------------------------------------------------------
    if (ed::BeginCreate())
    {
        ed::PinId srcPinId, destPinId;
        if (ed::QueryNewLink(&srcPinId, &destPinId))
        {
            if (srcPinId && destPinId)
            {
                if (ed::AcceptNewItem())
                {
                    NodePin *srcPin = (NodePin *)(uint64_t)(srcPinId);
                    NodePin *destPin = (NodePin *)(uint64_t)(destPinId);
                    if (srcPin->pinType != destPin->pinType && !ed::PinHadAnyLinks(srcPinId) && !ed::PinHadAnyLinks(destPinId))
                    {
                        LinkInfo *link = new LinkInfo{srcPin, destPin};
                        destPin->link = link;
                        srcPin->link = link;
                        ed::Link((uint64_t)(link), (uint64_t)link->srcPin, (uint64_t)link->destPin);
                        if (srcPin->pinType == NodePin::PinType::next)
                        {
                            *DlgGetNextID(srcPin->parentNode->data) = *DlgGetID(destPin->parentNode->data);
                            *DlgGetPrevID(destPin->parentNode->data) = *DlgGetID(srcPin->parentNode->data);
                        }
                        else
                        {
                            *DlgGetNextID(destPin->parentNode->data) = *DlgGetID(srcPin->parentNode->data);
                            *DlgGetPrevID(srcPin->parentNode->data) = *DlgGetID(destPin->parentNode->data);
                        }
                    }
                }
            }
        }
    }
    ed::EndCreate();

    // Handle deletion action ---------------------------------------------------------------------------
    if (ed::BeginDelete())
    {
        ed::LinkId deletedLinkId;

        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            if (ed::AcceptDeletedItem())
            {
                LinkInfo *linkPtr = (LinkInfo *)(uint64_t)deletedLinkId;
                linkPtr->srcPin->link = nullptr;
                linkPtr->destPin->link = nullptr;
                if (linkPtr->srcPin->pinType == NodePin::PinType::next)
                {
                    *DlgGetNextID(linkPtr->srcPin->parentNode->data) = 0x988d0903f713877b;
                    *DlgGetPrevID(linkPtr->destPin->parentNode->data) = 0x988d0903f713877b;
                }
                else
                {
                    *DlgGetNextID(linkPtr->destPin->parentNode->data) = 0x988d0903f713877b;
                    *DlgGetPrevID(linkPtr->srcPin->parentNode->data) = 0x988d0903f713877b;
                }
                delete linkPtr;
            }
        }
    }
    ed::EndDelete();

    ed::End();
    ed::SetCurrentEditor(nullptr);
    firstframe = false;
    // ImGui::ShowMetricsWindow();
    // ImGui::ShowDemoWindow();
}

/* General Application */

GeneralApplication::GeneralApplication(const char *name, const char *extension, FILE *inputStream) : Application(name)
{
    data.dataSize = 0;
    data.serializeType = 0;
    data.isBlocked = 0;

    readMetaStreamHeader(inputStream, &(this->header));
    const struct MetaClassDescription *description = getMetaClassDescriptionBySymbol(CRC64_CaseInsensitive(0, (uint8_t *)extension));
    if (description == NULL || description->read == NULL)
    {
        printf("file with extension %s is not supported\n", extension);
        exit(-1);
    }
    description->read(inputStream, &(this->data), 0);
    fclose(inputStream);
}

void GeneralApplication::OnStart()
{
    // ed::Config config;
    // config.SettingsFile = "General.json";
    // m_Context = ed::CreateEditor(&config);

    // printf("Read dlog %d, %d\n", links.size(), pins.size());
}

void GeneralApplication::OnFrame(float deltaTime)
{
    float width = ImGui::GetWindowWidth() - 20;
    const char *name = "(null)";
    if (data.description != NULL)
    {
        name = data.description->name;
    }
    char textBuffer[128];
    snprintf(textBuffer, 128, "##%s%d", name, 1);
    ImGui::BeginColumns(textBuffer, 2,
                        ImGuiOldColumnFlags_NoBorder |
                            ImGuiOldColumnFlags_NoResize |
                            ImGuiOldColumnFlags_NoPreserveWidths |
                            ImGuiOldColumnFlags_NoForceWithinWindow);

    // Adjust column width to match requested one.
    ImGui::SetColumnWidth(0, width + ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x);
    // End of tree column startup --------------------------------------------------------------

    // Back to normal ImGui drawing, in our column.

    if (ImGui::CollapsingHeader(textBuffer, ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        ImGui::BeginChild("Scrolling");
        renderNode(&data, 1, 0);
        ImGui::EndChild();
    }

    return;
}

void GeneralApplication::OnStop()
{
    ed::DestroyEditor(m_Context);
}